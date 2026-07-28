#include "LliurexAutoUpgradeWidget.h"
#include "LliurexAutoUpgradeWidgetUtils.h"

#include <KLocalizedString>
#include <KFormat>
#include <KNotification>
#include <QDebug>
#include <QTimer>
#include <QtCore/QStringList>

#include <QDBusConnection>
#include <QDBusMessage>


LliurexAutoUpgradeWidget::LliurexAutoUpgradeWidget(QObject *parent)
    : QObject(parent)
    , m_utils(new LliurexAutoUpgradeWidgetUtils(this))

   
{
   
    notificationTitle=i18n("LliureX-Auto-Upgrade");
    notificationBody=i18n("Ready to check status");
    notificationHead=i18n("Last execution:");
    notificationFoot=i18n("Wait for next check");
    notificationUpgradeFoot=i18n("Wait for next reboot");
    turnOffWarning=i18n("Do not turn off or restart the computer");
    lastUpgradeDownloaded=i18n("Last component downloaded:");
    lastUpgradeInstalled=i18n("Last component installed:");

    connect(m_utils,&LliurexAutoUpgradeWidgetUtils::startWidgetFinished,this,&LliurexAutoUpgradeWidget::handleStartFinished);
    connect(m_utils,&LliurexAutoUpgradeWidgetUtils::subscriptionFinished,this,&LliurexAutoUpgradeWidget::enableWidget);
    connect(m_utils,&LliurexAutoUpgradeWidgetUtils::unitStateChanged,this,&LliurexAutoUpgradeWidget::manageState);
    
    QTimer::singleShot(0,this,[this](){
        m_utils->startWidget();
    });

}  

void LliurexAutoUpgradeWidget::handleStartFinished(bool showWidget,bool startOk){

    if (showWidget){
        if (startOk){
            setSubToolTip(notificationBody);
            changeTryIconState(1);
            m_utils->createSubscription();
        }else{
            disableApplet();
        }
    }else{
        changeTryIconState(2);
    }

}

void LliurexAutoUpgradeWidget::enableWidget(bool success,QString error){

    if (success){
        changeTryIconState(0);
        qDebug() << "[LLIUREX-AUTO-UPGRADE]: Successfully subscribed to systemd manager signals.";
    }else{
        qDebug() << "[LLIUREX-AUTO-UPGRADE]: Failed to subscribe to systemd D-Bus signals:" << error;
        disableApplet();
    }
}

void LliurexAutoUpgradeWidget::manageState(int actionCode,QString lastExecutionTime,QString waitTime,QString upgradeItem){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Receiveing state: "<<actionCode;
    closeAllNotifications();
    setCurrentStackIndex(0);
    setShowDetailsBtn(false);

    if (actionCode==1){
        notificationBody=i18n("Ready to check status");
        setIconName("lliurex-auto-upgrade-warning");
        setIconNamePh("lliurex-auto-upgrade-warning");
        setSubToolTip(notificationBody);
    }else if(actionCode==2){
        notificationBody=i18n("Checking status");
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        setSubToolTip(notificationBody);
    }else if (actionCode==3){
        notificationBody=i18n("Installing packages.");
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        setSubToolTip(notificationBody+"\n"+turnOffWarning);
        sendNotification();
     }else if (actionCode==4){
        notificationBody=i18n("Updates installed")+"\n"+notificationFoot;
        setIconName("lliurex-auto-upgrade-ok");
        setIconNamePh("lliurex-auto-upgrade-ok");
        QString headText=notificationHead+" "+lastExecutionTime;
        setSubToolTip(headText+"\n"+notificationBody);
        m_notification = new KNotification(QStringLiteral("RemoteAction"),KNotification::CloseOnTimeout,this);
        m_notification->setComponentName(QStringLiteral("lliurexautoupgrade"));
        m_notification->setTitle(notificationBody);
        m_notification->setText("");
        m_notification->setIconName("lliurex-auto-upgrade-ok");
        m_notification->sendEvent();
    }else if(actionCode==5){
        notificationBody=i18n("No changes")+"\n"+notificationFoot;
        setIconName("lliurex-auto-upgrade-ok");
        setIconNamePh("lliurex-auto-upgrade-ok");
        QString headText=notificationHead+" "+lastExecutionTime;
        setSubToolTip(headText+"\n"+notificationBody);
    }else if(actionCode==6){
        notificationBody=i18n("Error, process not completed")+"\n"+notificationFoot;
        setIconName("lliurex-auto-upgrade-error");
        setIconNamePh("lliurex-auto-upgrade-error");
        QString headText=notificationHead+" "+lastExecutionTime;
        setSubToolTip(headText+"\n"+notificationBody);
    }else if (actionCode==7){
        notificationBody=i18n("Ready to start unattended upgrade in %1 seconds",waitTime);
        setIconName("lliurex-auto-upgrade-warning");
        setIconNamePh("lliurex-auto-upgrade-warning");
        setSubToolTip(notificationBody);
    }else if (actionCode==8){
        notificationBody=i18n("Gathering unattended upgrade package list");
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        setSubToolTip(notificationBody);
    }else if (actionCode==9){
        notificationBody=i18n("Unattended upgrade is downloading packages for the %1 component",upgradeItem);
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        setSubToolTip(notificationBody);
    }else if (actionCode==10){
        lastUpradeItem=upgradeItem;
        notificationBody=i18n("Unattended upgrade have been downloaded packages for the %1 component",upgradeItem);
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        setSubToolTip(notificationBody);
    }else if (actionCode==11){
        lastUpradeItem="";
        notificationBody=i18n("Unattended upgrade has downloaded every component");
        setIconName("lliurex-auto-upgrade-ok");
        setIconNamePh("lliurex-auto-upgrade-ok");
        QString headText=notificationHead+" "+lastExecutionTime;
        setSubToolTip(headText+"\n"+notificationBody);
    }else if (actionCode==12){
        notificationBody=i18n("Unattended upgrade download limit reached")+"\n"+lastUpgradeDownloaded+" "+lastUpradeItem+"\n"+notificationUpgradeFoot;
        setIconName("lliurex-auto-upgrade-ok");
        setIconNamePh("lliurex-auto-upgrade-ok");
        QString headText=notificationHead+" "+lastExecutionTime;
        setSubToolTip(headText+"\n"+notificationBody); 
    }else if (actionCode==13){
        closeAllNotifications();
        notificationBody=i18n("Unattended upgrade is installing packages for the %1 component.",upgradeItem);
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        QString message=notificationBody+" "+turnOffWarning;
        setSubToolTip(message);
        m_upgradeNotification = new KNotification(QStringLiteral("RemoteAction"),KNotification::Persistent,this);
        m_upgradeNotification->setComponentName(QStringLiteral("lliurexautoupgrade"));
        m_upgradeNotification->setTitle(message);
        m_upgradeNotification->setText("");
        m_upgradeNotification->setIconName("lliurex-auto-upgrade");
        m_upgradeNotification->sendEvent();
    }else if (actionCode==14){
        closeAllNotifications();
        lastUpradeItem=upgradeItem;
        notificationBody=i18n("Unattended upgrade has installed packages for the %1 component.",upgradeItem);
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        setSubToolTip(notificationBody+"\n"+turnOffWarning);
    }else if (actionCode==15){
        closeAllNotifications();
        lastUpradeItem="";
        notificationBody=i18n("The system has been updated");
        setIconName("lliurex-auto-upgrade-ok");
        setIconNamePh("lliurex-auto-upgrade-ok");
        QString headText=notificationHead+" "+lastExecutionTime;
        setSubToolTip(headText+"\n"+notificationBody); 
        m_notification = new KNotification(QStringLiteral("RemoteAction"),KNotification::CloseOnTimeout,this);
        m_notification->setComponentName(QStringLiteral("lliurexautoupgrade"));
        m_notification->setTitle(notificationBody);
        m_notification->setText("");
        m_notification->setIconName("lliurex-auto-upgrade-ok");
        m_notification->sendEvent();
    }else if (actionCode==16){
        closeAllNotifications();
        notificationBody=i18n("Unattended upgrade install limit reached")+"\n"+lastUpgradeInstalled+" "+lastUpradeItem+"\n"+notificationUpgradeFoot;
        setIconName("lliurex-auto-upgrade-ok");
        setIconNamePh("lliurex-auto-upgrade-ok");
        QString headText=notificationHead+" "+lastExecutionTime;
        setSubToolTip(headText+"\n"+notificationBody); 
        m_notification = new KNotification(QStringLiteral("RemoteAction"),KNotification::CloseOnTimeout,this);
        m_notification->setComponentName(QStringLiteral("lliurexautoupgrade"));
        m_notification->setTitle(notificationBody);
        m_notification->setText("");
        m_notification->setIconName("lliurex-auto-upgrade-ok");
        m_notification->sendEvent();
    }else if(actionCode==17){
        closeAllNotifications();
        lastUpradeItem="";
        notificationBody=i18n("Error, unattended upgrade failed. Canceling process");
        setIconName("lliurex-auto-upgrade-error");
        setIconNamePh("lliurex-auto-upgrade-error");
        QString headText=notificationHead+" "+lastExecutionTime;
        setSubToolTip(headText+"\n"+notificationBody);
    }

    if (actionCode==4 || actionCode==5){
        QStringList installedPkg=m_utils->lastInstalledPkg;
        setLastInstalledPkg(installedPkg);
        if (!installedPkg.isEmpty()){
            setShowDetailsBtn(true);
        }else{
            setShowDetailsBtn(false);
        }
    }
        
}


void LliurexAutoUpgradeWidget::disableApplet(){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Unable to connect with lliurex-auto-upgrade";
    notificationBody=i18n("Unable to get information from LliureX-Auto-Upgrade");
   
    setIconName("lliurex-auto-upgrade-error");
    setIconNamePh("lliurex-auto-upgrade-error");
    setSubToolTip(notificationBody);
    changeTryIconState(0);

}

void LliurexAutoUpgradeWidget::sendNotification(){
    
   if (QDBusConnection::sessionBus().isConnected()) {
        QDBusMessage msg= QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                       "/org/freedesktop/Notifications",
                                       "org.freedesktop.Notifications",
                                       "Notify" 
                                       );

        uint replacesId=0;
        QStringList actions;
        QVariantMap hints;
        QString message=notificationBody+" "+turnOffWarning;
        hints.insert("desktop-entry","lliurex-auto-upgrade-plasmoid");
        msg << "LliureX-Auto-Upgrade" << replacesId << "lliurex-auto-upgrade" << message << "" << actions << hints << 0; 
        QDBusMessage reply=QDBusConnection::sessionBus().call(msg);
        if (reply.type()== QDBusMessage::ReplyMessage){
            lastNotificationId=reply.arguments().at(0).toUInt();
        }else{
            lastNotificationId=0;
        }
    }
}

void LliurexAutoUpgradeWidget::closeAllNotifications(){

    uint referenceId=0;

    if (m_notification){
        m_notification->close();
        m_notification->deleteLater();
        m_notification=nullptr;
    }

    if (m_upgradeNotification){
        m_upgradeNotification->close();
        m_upgradeNotification->deleteLater();
        m_upgradeNotification=nullptr;
    }

    if (referenceId<lastNotificationId){
        if (QDBusConnection::sessionBus().isConnected()) {
            QDBusMessage msg=QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                       "/org/freedesktop/Notifications",
                                       "org.freedesktop.Notifications",
                                       "CloseNotification" 
                                       );
            msg << lastNotificationId;
            QDBusConnection::sessionBus().send(msg);
        }
    }
}

LliurexAutoUpgradeWidget::TrayStatus LliurexAutoUpgradeWidget::status() const
{
    return m_status;
}

void LliurexAutoUpgradeWidget::changeTryIconState(int state){

    if (state==0){
    	setStatus(ActiveStatus);
        setToolTip(notificationTitle);
    }else if (state==1){
        setStatus(PassiveStatus);
    }else if (state==2){
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        setSubToolTip(i18n("LliureX-Auto-Upgrade is not enabled in this computer"));
        setStatus(HiddenStatus);
    }

}

void LliurexAutoUpgradeWidget::manageNavigation(int stackIndex)
{

    setCurrentStackIndex(stackIndex);
}

void LliurexAutoUpgradeWidget::setStatus(LliurexAutoUpgradeWidget::TrayStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

QString LliurexAutoUpgradeWidget::iconName() const
{
    return m_iconName;
}

void LliurexAutoUpgradeWidget::setIconName(const QString &name)
{
    if (m_iconName != name) {
        m_iconName = name;
        emit iconNameChanged();
    }
}

QString LliurexAutoUpgradeWidget::iconNamePh() const
{
    return m_iconNamePh;
}

void LliurexAutoUpgradeWidget::setIconNamePh(const QString &name)
{
    if (m_iconNamePh != name) {
        m_iconNamePh = name;
        emit iconNamePhChanged();
    }
}

QString LliurexAutoUpgradeWidget::toolTip() const
{
    return m_toolTip;
}

void LliurexAutoUpgradeWidget::setToolTip(const QString &toolTip)
{
    if (m_toolTip != toolTip) {
        m_toolTip = toolTip;
        emit toolTipChanged();
    }
}

QString LliurexAutoUpgradeWidget::subToolTip() const
{
    return m_subToolTip;
}

void LliurexAutoUpgradeWidget::setSubToolTip(const QString &subToolTip)
{
    if (m_subToolTip != subToolTip) {
        m_subToolTip = subToolTip;
        emit subToolTipChanged();
    }
}

bool LliurexAutoUpgradeWidget::showDetailsBtn()
{

    return m_showDetailsBtn;
}

void LliurexAutoUpgradeWidget::setShowDetailsBtn(bool showDetailsBtn)
{

    if (m_showDetailsBtn!=showDetailsBtn){
        m_showDetailsBtn=showDetailsBtn;
        emit showDetailsBtnChanged();
    }
}

int LliurexAutoUpgradeWidget::currentStackIndex()
{

    return m_currentStackIndex;
}

void LliurexAutoUpgradeWidget::setCurrentStackIndex(int currentStackIndex)
{

    if (m_currentStackIndex!=currentStackIndex){
        m_currentStackIndex=currentStackIndex;
        emit currentStackIndexChanged();
    }
}

QStringList LliurexAutoUpgradeWidget::lastInstalledPkg()
{

    return m_lastInstalledPkg;
}

void LliurexAutoUpgradeWidget::setLastInstalledPkg(QStringList installedPkg)
{

    if (m_lastInstalledPkg!=installedPkg){
        m_lastInstalledPkg=installedPkg;
        emit lastInstalledPkgChanged();
    }
}
