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

#include <tuple>
#include <sys/types.h>


LliurexAutoUpgradeWidgetUtils::LliurexAutoUpgradeWidgetUtils(QObject *parent)
    : QObject(parent)
       
{
    user=qgetenv("USER");
}

void LliurexAutoUpgradeWidgetUtils::cleanCache(){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Clean cache";
    QFile CURRENT_VERSION_TOKEN;
    QDir cacheDir("/home/"+user+"/.cache/plasmashell/qmlcache");
    QString currentVersion="";
    bool clear=false;

    CURRENT_VERSION_TOKEN.setFileName("/home/"+user+"/.config/lliurex-auto-upgrade-widget.conf");
    QString installedVersion=getInstalledVersion();

    if (!CURRENT_VERSION_TOKEN.exists()){
        if (CURRENT_VERSION_TOKEN.open(QIODevice::WriteOnly)){
            QTextStream data(&CURRENT_VERSION_TOKEN);
            data<<installedVersion;
            CURRENT_VERSION_TOKEN.close();
            clear=true;
        }
    }else{
        if (CURRENT_VERSION_TOKEN.open(QIODevice::ReadOnly)){
            QTextStream content(&CURRENT_VERSION_TOKEN);
            currentVersion=content.readLine();
            CURRENT_VERSION_TOKEN.close();
        }

        if (currentVersion!=installedVersion){
            if (CURRENT_VERSION_TOKEN.open(QIODevice::WriteOnly)){
                QTextStream data(&CURRENT_VERSION_TOKEN);
                data<<installedVersion;
                CURRENT_VERSION_TOKEN.close();
                clear=true;
            }
        }
    } 
    if (clear){
        if (cacheDir.exists()){
            cacheDir.removeRecursively();
        }
    }   

}

QString LliurexAutoUpgradeWidgetUtils::getInstalledVersion(){

    QFile INSTALLED_VERSION_TOKEN;
    QString installedVersion="";
    
    INSTALLED_VERSION_TOKEN.setFileName("/var/lib/lliurex-auto-upgrade-plasmoid/version");

    if (INSTALLED_VERSION_TOKEN.exists()){
        if (INSTALLED_VERSION_TOKEN.open(QIODevice::ReadOnly)){
            QTextStream content(&INSTALLED_VERSION_TOKEN);
            installedVersion=content.readLine();
            INSTALLED_VERSION_TOKEN.close();
        }
    }
    return installedVersion;

}  

bool LliurexAutoUpgradeWidgetUtils::showWidget(){

    QFile disabelToken;
    disabelToken.setFileName(disableAutoUpgrade);

    if (disabelToken.exists()){
        return false;
    }else{
        return true;
    }
}  

bool LliurexAutoUpgradeWidgetUtils::testListener(){

 if (!QDBusConnection::systemBus().isConnected()) {
    qDebug() << "[LLIUREX-AUTO-UPGRADE]: Cannot connect to the system D-Bus!";
    return false;
 }else{
    return true;
 }

}
 
bool LliurexAutoUpgradeWidgetUtils::startListener(){

    QDBusInterface managerInterface("org.freedesktop.systemd1",
                                        "/org/freedesktop/systemd1",
                                        "org.freedesktop.systemd1.Manager",
                                        QDBusConnection::systemBus());

    

    QDBusReply<QDBusObjectPath> reply=managerInterface.call("GetUnit",m_unitName);
    QString m_unitPath=reply.value().path();

    QDBusConnection::systemBus().connect(
            "org.freedesktop.systemd1",       // sender service
            m_unitPath,                         // object path
            "org.freedesktop.DBus.Properties",// interface
            "PropertiesChanged",              // signal name
            this,                             // receiver QObject
            SLOT(onPropertiesChanged(const QString&, const QVariantMap&, const QStringList&)));

    QDBusMessage subscribeCall = managerInterface.call("Subscribe");
        if (subscribeCall.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "[LLIUREX-AUTO-UPGRADE]: Failed to subscribe to systemd D-Bus signals:" << subscribeCall.errorMessage();
            return false;
        } else {
            qDebug() << "[LLIUREX-AUTO-UPGRADE]: Successfully subscribed to systemd manager signals.";
            return true;
        }
}

void LliurexAutoUpgradeWidgetUtils::onPropertiesChanged(const QString &interfaceName, const QVariantMap& changedProperties, const QStringList &invalidatedProperties)
{
        Q_UNUSED(interfaceName);
        Q_UNUSED(invalidatedProperties);

        if (changedProperties.contains("StatusText")) {
            QString newState = changedProperties["StatusText"].toString();
            if (newState!=lastUpdate){
                lastUpdate=newState;
                qDebug() << "[LLIUREX-AUTO-UPGRADE]: Unit" << m_unitName << " StatusText changed to:" << newState;
                
                if (newState.contains("First run") || newState.contains("dpkg to finish")){
                    actionCode=1;
                }else if (newState.contains("remote file")){
                    actionCode=2;
                }else if (newState.contains("before installing")){
                    actionCode=3;
                }else if (newState.contains("Installling packages")){
                    actionCode=3;
                    QString tmpPkg=newState.split(": ")[1];
                    getLastInstalledPkg(tmpPkg);

                }else if (newState.contains("Installing finished")){
                    actionCode=4;
                }else if (newState.contains("Nothing to execute")){
                    actionCode=5;
                }
  
                emit unitStateChanged(actionCode,lastInstalledPkg);
            }
        }
      
}

void LliurexAutoUpgradeWidgetUtils::getLastInstalledPkg(QString installedPkg)
{

    QStringList tmpPkg=installedPkg.split(" ");

    for (const QString &pkg : tmpPkg){

        if (!lastInstalledPkg.contains(pkg)){
            lastInstalledPkg.prepend(pkg);
        }

    }

}
