#include "LliurexAutoUpgradeWidget.h"
#include "LliurexAutoUpgradeWidgetUtils.h"

#include <KLocalizedString>
#include <KFormat>
#include <KNotification>
#include <QDebug>
#include <QtCore/QStringList>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

LliurexAutoUpgradeWidget::LliurexAutoUpgradeWidget(QObject *parent)
    : QObject(parent)
    , m_utils(new LliurexAutoUpgradeWidgetUtils(this))

   
{
    m_utils->cleanCache();
    notificationTitle=i18n("LliureX-Auto-Upgrade");
    notificationBody=i18n("Waiting to check status");
    notifyInterface=new QDBusInterface("org.freedesktop.Notifications",
                                   "/org/freedesktop/Notifications",
                                   "org.freedesktop.Notifications",
                                   QDBusConnection::sessionBus()); 
    setSubToolTip(notificationBody);
    plasmoidMode();

}  

void LliurexAutoUpgradeWidget::plasmoidMode(){

    if (!m_utils->showWidget()){
       changeTryIconState(2);
    }else{
        if (m_utils->testListener()){
            if (m_utils->startListener()){
                changeTryIconState(0);
                connect(m_utils,&LliurexAutoUpgradeWidgetUtils::unitStateChanged,this,&LliurexAutoUpgradeWidget::manageState);
            }else{
                disableApplet();
            }
        }else{
            disableApplet();
        }
      

    }

}

void LliurexAutoUpgradeWidget::manageState(int actionCode,QStringList installedPkg){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Receiveing state: "<<actionCode<<" Instaled pkg: "<<installedPkg;
    closeAllNotifications();
    setCurrentStackIndex(0);
    setShowDetailsBtn(false);

    if (actionCode==1){
        notificationBody=i18n("Waiting to check status");
        setIconName("lliurex-auto-upgrade-warning");
        setIconNamePh("lliurex-auto-upgrade-warning");
        setSubToolTip(notificationBody);
    }else if(actionCode==2){
        notificationBody=i18n("Checking status");
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        setSubToolTip(notificationBody);
    }else if (actionCode==3){
        notificationBody=i18n("Installing packages. Do not turn off or restart the computer");
        setIconName("lliurex-auto-upgrade");
        setIconNamePh("lliurex-auto-upgrade");
        setSubToolTip(notificationBody);
        sendNotification();
     }else if (actionCode==4){
        notificationBody=i18n("Installing finished. Everything is up to date");
        setIconName("lliurex-auto-upgrade-ok");
        setIconNamePh("lliurex-auto-upgrade-ok");
        setSubToolTip(notificationBody);
        m_notification = new KNotification(QStringLiteral("RemoteAction"),KNotification::CloseOnTimeout,this);
        m_notification->setComponentName(QStringLiteral("lliurexautoupgrade"));
        m_notification->setTitle(notificationBody);
        m_notification->setText("");
        m_notification->setIconName("lliurex-auto-upgrade-ok");
        m_notification->sendEvent();
    }else if(actionCode==5){
        notificationBody=i18n("Everything is up to date");
        setIconName("lliurex-auto-upgrade-ok");
        setIconNamePh("lliurex-auto-upgrade-ok");
        setSubToolTip(notificationBody);
    }

    if (actionCode==4 || actionCode==5){
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
    
    if (notifyInterface->isValid()){
        uint replacesId=0;
        QStringList actions;
        QVariantMap hints;
        hints.insert("desktop-entry","lliurex-auto-upgrade-plasmoid");
        QDBusReply<uint> reply=notifyInterface->call("Notify","LliureX-Auto-Upgrade",replacesId,"lliurex-auto-upgrade",notificationBody,"",actions,hints,0);
        if (reply.isValid()){
            lastNotificationId=reply.value();
        }
    }
}

void LliurexAutoUpgradeWidget::closeAllNotifications(){

    uint referenceId=0;

    if (m_notification){
        m_notification->close();
    }

    if (referenceId<lastNotificationId){
        if (notifyInterface->isValid()){
            notifyInterface->call("CloseNotification",lastNotificationId);
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
