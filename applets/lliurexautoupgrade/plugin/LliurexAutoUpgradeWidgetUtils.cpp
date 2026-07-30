#include "LliurexAutoUpgradeWidgetUtils.h"

#include <QFile>
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
#include <QtConcurrent>
#include <QPointer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <tuple>
#include <sys/types.h>
#include <algorithm>


LliurexAutoUpgradeWidgetUtils::LliurexAutoUpgradeWidgetUtils(QObject *parent)
    : QObject(parent)
    , actionCode(UpgradeAction::ReadyToCheck)
       
{

}

void LliurexAutoUpgradeWidgetUtils::startWidget(){

    QPointer<LliurexAutoUpgradeWidgetUtils>safeThis(this);

    QtConcurrent::run([safeThis]() {

        if (!safeThis){
            return;
        }

        bool showWidget=false;
        bool startOk=false;

        try{
            showWidget=safeThis->showWidget();
            if (showWidget){
                safeThis->getPkgsInstalledInSession();
                startOk=safeThis->createInterface();
            }
        }catch (std::exception& e){
            qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Error initializing widget: " <<e.what();
        } 

        if (safeThis){
            emit safeThis->startWidgetFinished(showWidget,startOk);
        }

    });
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

    if (!QDBusConnection::systemBus().isConnected()) {
        qDebug() << "[LLIUREX-AUTO-UPGRADE]: Cannot connect to the system D-Bus!";
        return false;
    }else{
        managerInterface=new QDBusInterface("org.freedesktop.systemd1",
                                        "/org/freedesktop/systemd1",
                                        "org.freedesktop.systemd1.Manager",
                                        QDBusConnection::systemBus());
       
        if (managerInterface->isValid()){
            return true;
        }else{
            return false;
        }
    }

}

void LliurexAutoUpgradeWidgetUtils::createSubscription(){

    if (!managerInterface || !managerInterface->isValid()){
        emit subscriptionFinished(false,"DBus interface not valid");
        return;
    }
    QDBusPendingCall subscriptionCall = managerInterface->asyncCall("Subscribe");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(subscriptionCall,this);

    connect(watcher, &QDBusPendingCallWatcher::finished,this,[this](QDBusPendingCallWatcher *self){
        
        if (!self) return;

        QDBusPendingReply<void>subReply=*self;
        self->deleteLater();

        if (subReply.isError()){
            emit subscriptionFinished(false,subReply.error().message());
            return;
        }

        QDBusPendingCall unitCall = managerInterface->asyncCall("GetUnit", m_unitName);
        QDBusPendingCallWatcher *unitWatcher = new QDBusPendingCallWatcher(unitCall, this);
        
        connect(unitWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *uSelf) {
            
            if (!uSelf) return;

            QDBusPendingReply<QDBusObjectPath> unitReply = *uSelf;
            uSelf->deleteLater();

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
                    emit subscriptionFinished(true, "");
                }else{
                    emit subscriptionFinished(false, "DBusConnection fails");
                }
            }else{
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
                QString lastExecution="";
                QString upgradeItem="";
                QString waitTime="";
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
                    actionCode=UpgradeAction::FullDownloaded;
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
                }else if (newState.contains("upgrade install limit reached")){
                    updatedFailed=false;
                    actionCode=UpgradeAction::UpdateLimit;
                }else if (newState.contains("upgrade failed")){
                    updatedFailed=true;
                    actionCode=UpgradeAction::UpdatedError;
                }

                lastExecution=getLastExecutionTime();

                emit unitStateChanged(actionCode,lastExecution,waitTime,upgradeItem);
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

    QRegularExpressionMatch match=regex.match(&message);

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
