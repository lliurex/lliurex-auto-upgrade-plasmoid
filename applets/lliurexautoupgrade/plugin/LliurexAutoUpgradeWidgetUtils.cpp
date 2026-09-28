#include "LliurexAutoUpgradeWidgetUtils.h"

#include <QCoreApplication>
#include <QtConcurrent>
#include <QFile>
#include <QFuture>
#include <QFutureWatcher>
#include <QDebug>
#include <QList>
#include <KLocalizedString>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDate>
#include <QTime>
#include <QThreadPool>
#include <QPointer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QProcess>

#include <tuple>
#include <sys/types.h>
#include <algorithm>


LliurexAutoUpgradeWidgetUtils::LliurexAutoUpgradeWidgetUtils(QObject *parent)
    : QObject(nullptr)
    , managerInterface(nullptr)
    , actionCode(UpgradeAction::ReadyToCheck)
    , isSubscribed(false)
    , isInitializing(false)
    , isSubscribing(false)
    , interfaceCreated(false)
       
{

}

void LliurexAutoUpgradeWidgetUtils::startWidget(){

    if (interfaceCreated && managerInterface && managerInterface->isValid()){
        emit startWidgetFinished(true,true);
        return;
    }

    if (isInitializing){
	qDebug() << "[LLIUREX-AUTO-UPGRADE]: Already exists an initialization process";
        return;
    }

    isInitializing=true;

    QFuture<bool>future=QtConcurrent::run(QThreadPool::globalInstance(),[this]() {

        return this->showWidget();
    });

    auto watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
        bool showWidgetResult = watcher->result();
        bool startOk = false;

        if (showWidgetResult) {
            try {
                this->getPkgsInstalledInSession();
                startOk = this->createInterface();
            } catch (const std::exception& e) {
                qDebug() << "[LLIUREX-AUTO-UPGRADE]: Error initializing widget: " << e.what();
            }
        }

        this->interfaceCreated = startOk; 
        this->isInitializing = false;

        emit this->startWidgetFinished(showWidgetResult, startOk);

        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

bool LliurexAutoUpgradeWidgetUtils::showWidget(){

    QFile disableToken;
    disableToken.setFileName(disableAutoUpgrade);

    if (disableToken.exists()){
        return false;
    }else{
        return true;
    }
}  

bool LliurexAutoUpgradeWidgetUtils::createInterface(){

    if (managerInterface && managerInterface->isValid()) {
        return true;
    }

    if (!QDBusConnection::systemBus().isConnected()) {
        qDebug() << "[LLIUREX-AUTO-UPGRADE]: Cannot connect to the system D-Bus!";
        return false;
    }

    if (managerInterface){
        managerInterface->deleteLater();
    }
    
    managerInterface=new QDBusInterface("org.freedesktop.systemd1",
                                        "/org/freedesktop/systemd1",
                                        "org.freedesktop.systemd1.Manager",
                                        QDBusConnection::systemBus(),qApp);
       
    return managerInterface->isValid();   

}

void LliurexAutoUpgradeWidgetUtils::createSubscription(){

    if (!managerInterface || !managerInterface->isValid()){
        emit subscriptionFinished(false,"DBus interface not valid");
        return;
    }

    if (isSubscribed){
        emit subscriptionFinished(true,"");
        emit unitStateChanged(actionCode,lastExecution,waitTime,upgradeItem,lliurexVersion);
        return;
    }

    if (isSubscribing){
        qDebug() << "[LLIUREX-AUTO-UPGRADE]: Already exists a subscribe process";
        return;
    }

    isSubscribing=true;

    QDBusPendingCall subscriptionCall = managerInterface->asyncCall("Subscribe");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(subscriptionCall,this);

    connect(watcher, &QDBusPendingCallWatcher::finished,this,[this](QDBusPendingCallWatcher *self){
        
        if (!self) return;

        QDBusPendingReply<void>subReply=*self;
        self->deleteLater();

        if (subReply.isError()){
            isSubscribing=false;
            emit subscriptionFinished(false,subReply.error().message());
            return;
        }

        QDBusPendingCall unitCall = managerInterface->asyncCall("GetUnit", m_unitName);
        QDBusPendingCallWatcher *unitWatcher = new QDBusPendingCallWatcher(unitCall, this);
        
        connect(unitWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *uSelf) {
            
            if (!uSelf) return;

            QDBusPendingReply<QDBusObjectPath> unitReply = *uSelf;
            uSelf->deleteLater();

            isSubscribing=false;

            if (!unitReply.isError()) {
                QString path = unitReply.value().path();

                bool connected=QDBusConnection::systemBus().connect(
                    "org.freedesktop.systemd1",
                    path,
                    "org.freedesktop.DBus.Properties",
                    "PropertiesChanged",
                    this,
                    SLOT(onPropertiesChanged(const QString&, const QVariantMap&, const QStringList&)));

                if (connected){
                    isSubscribed=true;
                    QDBusInterface unitInterface("org.freedesktop.systemd1",
                                                 path,
                                                 "org.freedesktop.DBus.Properties",
                                                 QDBusConnection::systemBus());

                    if (unitInterface.isValid()) {
                        QDBusMessage reply = unitInterface.call("Get",
                                                                "org.freedesktop.systemd1.Service",
                                                                "StatusText");

                        if (!reply.errorMessage().isEmpty()) {
                            qDebug() << "[LLIUREX-AUTO-UPGRADE]: Error gathering init status" << reply.errorMessage();
                        } else {
                            QVariant firstArgument = reply.arguments().at(0).value<QDBusVariant>().variant();
                            QString initialStatusText = firstArgument.toString();

                            if (!initialStatusText.isEmpty()) {
                                qDebug() << "[LLIUREX-AUTO-UPGRADE]: Init state gathered:" << initialStatusText;

                                QVariantMap simulatedProperties;
                                simulatedProperties.insert("StatusText", initialStatusText);

                                this->onPropertiesChanged("org.freedesktop.systemd1.Unit", simulatedProperties, QStringList());
                            }
                        }
                    }

                    emit subscriptionFinished(true, "");
                }else{
                    isSubscribed=false;
                    emit subscriptionFinished(false, "DBusConnection fails");
                }
            }else{
                isSubscribed=false;
                emit subscriptionFinished(false, unitReply.error().message());
            }
        });

    });

}

void LliurexAutoUpgradeWidgetUtils::onPropertiesChanged(const QString &interfaceName, const QVariantMap& changedProperties, const QStringList &invalidatedProperties)
{
        Q_UNUSED(interfaceName);
        Q_UNUSED(invalidatedProperties);

        if (changedProperties.contains("StatusText")) {
            QString newState = changedProperties["StatusText"].toString();
            if (newState!=lastUpdate){
                lastUpdate=newState;
                lastExecution="";
                upgradeItem="";
                waitTime="";
                lliurexVersion="";
                qDebug() << "[LLIUREX-AUTO-UPGRADE]: Unit" << m_unitName << " StatusText changed to:" << newState;
                if (newState.contains("First run")) {
                    if (!checkFailed){
                        actionCode=UpgradeAction::ReadyToCheck;
                    }else{
                        actionCode=UpgradeAction::ProcessError;
                    }
                }else if (newState.contains("dpkg to finish")){
                    actionCode=UpgradeAction::ReadyToCheck;
                }else if (newState.contains("remote file")){
                    actionCode=UpgradeAction::CheckingStatus;
                }else if (newState.contains("before installing")){
                    actionCode=UpgradeAction::InstallingPackages;
                }else if (newState.contains("Installing packages")){
                    actionCode=UpgradeAction::InstallingPackages;
                    QStringList tokens=newState.split(": ");
                    if (tokens.size() > 1 ){
                        getLastInstalledPkg(tokens[1]);
                    }
                }else if (newState.contains("Installing finished")){
                    checkFailed=false;
                    actionCode=UpgradeAction::PackagesInstalled;
                }else if (newState.contains("Nothing to execute")){
                    checkFailed=false;
                    actionCode=UpgradeAction::NoChanges;
                }else if (newState.contains("Failed to")){
                    checkFailed=true;
                    actionCode=UpgradeAction::ProcessError;
                }else if (newState.contains("Starting unattended upgrades")){
                    if (!updatedFailed){
                        actionCode=UpgradeAction::StartingAutoUpgrade;
                        waitTime=getWaitTimeForUpgrade(newState);
                    }else{
                        actionCode=UpgradeAction::UpdatedError;
                    }
                }else if (newState.contains("Gathering unattended upgrade")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::GatheringPackages;
                }else if (newState.contains("upgrade is downloading")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::DownloadingComponent;
                    upgradeItem=getUpgradeItem(newState);
                }else if (newState.contains("have been downloaded")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::ComponentDownloaded;
                    upgradeItem=getUpgradeItem(newState);
                }else if (newState.contains("Waiting until next reboot to install")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::FullDownloadedWait;
                }else if (newState.contains("downloaded every component")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::FullDownloadedWait;
                }else if (newState.contains("upgrade download limit reached")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::DownloadLimit;
                }else if (newState.contains("upgrade is installing")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::UpdatingComponent;
                    upgradeItem=getUpgradeItem(newState);
                }else if (newState.contains("have been installed")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::ComponentUpdated;
                    upgradeItem=getUpgradeItem(newState);
                }else if (newState.contains("installed every component.")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::SystemUpdated;
                    lliurexVersion=getLliurexVersion();
                }else if (newState.contains("upgrade install limit reached")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::UpdateLimit;
                }else if (newState.contains("upgrade failed")){
                    updatedFailed=true;
                    actionCode=UpgradeAction::UpdatedError;
                }else if (newState.contains("System is up to date.")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::SystemUpdated;
                    lliurexVersion=getLliurexVersion();
                }

                lastExecution=getLastExecutionTime();

                emit unitStateChanged(actionCode,lastExecution,waitTime,upgradeItem,lliurexVersion);
            }
        }
      
}

void LliurexAutoUpgradeWidgetUtils::getLastInstalledPkg(QString installedPkg)
{

    QStringList tmpPkg=installedPkg.split(" ");

    for (const QString &pkg : tmpPkg){
        if (!pkg.isEmpty()){
            if (!lastInstalledPkg.contains(pkg)){
                lastInstalledPkg.prepend(pkg);
            }
        }

    }

}

QString LliurexAutoUpgradeWidgetUtils::getLastExecutionTime(){

    QDate currentDate=QDate::currentDate();
    QString lastDay=currentDate.toString(Qt::ISODate);
    QTime currentTime=QTime::currentTime();
    QString lastTime=currentTime.toString(Qt::ISODate);

    QString lastTimeStamp=lastDay+" - "+lastTime;

    return lastTimeStamp;


}

QString LliurexAutoUpgradeWidgetUtils::getUpgradeItem(QString &message){

    auto it = std::find_if(upgradeItems.begin(),upgradeItems.end(),[&message](const QString &upgradeItem){
        return message.contains(upgradeItem,Qt::CaseInsensitive);   
    });

    if (it != upgradeItems.end()){
        QString item = *it;
        if (!item.isEmpty()){
            return item.at(0).toUpper() + item.mid(1);
        }
        return item;
    }

    return QString();

}

QString LliurexAutoUpgradeWidgetUtils::getWaitTimeForUpgrade(QString &message){

    static const QRegularExpression regex(R"(\b(\d+)\s+seconds\b)");

    QRegularExpressionMatch match=regex.match(message);

    if (match.hasMatch()){
        return match.captured(1);
    }

    return QString();
}

void LliurexAutoUpgradeWidgetUtils::getPkgsInstalledInSession(){

    QFile pkgsLog(pkgInstalledLog);

    if (pkgsLog.exists()){
        if (pkgsLog.open(QIODevice::ReadOnly)){
            QTextStream content(&pkgsLog);
            while (!content.atEnd()){
                QString tmpLine=content.readLine().remove('\n');
                if (!tmpLine.isEmpty()){
                    QStringList tmpPkg=tmpLine.split(" ");
                    for (const QString &pkg : tmpPkg){
                        if (!pkg.isEmpty()){
                            if (!lastInstalledPkg.contains(pkg)){
                                lastInstalledPkg.prepend(pkg);
                            }
                        }
                    }
                }
            }
            
            pkgsLog.close();
        }
    }
}

QString LliurexAutoUpgradeWidgetUtils::getLliurexVersion(){

    QProcess process;
    process.start("lliurex-version",QStringList());
    if (process.waitForFinished(3000)){
        QString output=QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        QStringList parts=output.split(",");
        if (!parts.isEmpty()){
            return parts.last().trimmed();
        }
    }

    return QString();
}


