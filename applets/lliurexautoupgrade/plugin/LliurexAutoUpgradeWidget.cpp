#include "LliurexAutoUpgradeWidget.h"
#include "LliurexAutoUpgradeWidgetUtils.h"

#include <KLocalizedString>
#include <KFormat>
#include <KNotification>
#include <QDebug>
#include <QStringBuilder>
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
    notificationFoot=i18n("Waiting for the next check");
    updateFoot=i18n("The update process will continue in the next restart");
    updateLimitFoot=i18n("Will continue in the next restart");
    turnOffWarning=i18n("Do not turn off or restart the computer");
    lastUpgradeDownloaded=i18n("Last component downloaded:");
    lastUpgradeInstalled=i18n("Last component updated:");

    connect(m_utils,&LliurexAutoUpgradeWidgetUtils::startWidgetFinished,this,&LliurexAutoUpgradeWidget::handleStartFinished);
    connect(m_utils,&LliurexAutoUpgradeWidgetUtils::subscriptionFinished,this,&LliurexAutoUpgradeWidget::enableWidget);
    connect(m_utils,&LliurexAutoUpgradeWidgetUtils::unitStateChanged,this,&LliurexAutoUpgradeWidget::manageState);
 
    QDBusConnection::sessionBus().connect(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "NotificationClosed",
        this,
        SLOT(onNotificationClosed(uint,uint))
    );
   
    QTimer::singleShot(0,this,[this](){
        m_utils->startWidget();
    });

}

LliurexAutoUpgradeWidget::~LliurexAutoUpgradeWidget(){

    closeNotificationForced();

}  

void LliurexAutoUpgradeWidget::handleStartFinished(bool showWidget,bool startOk){

    if (showWidget){
        if (startOk){
            setSubToolTip(notificationBody);
            setMessagePh(notificationBody);
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

void LliurexAutoUpgradeWidget::manageState(LliurexAutoUpgradeWidgetUtils::UpgradeAction actionCode,const QString& lastExecutionTime,const QString& waitTime,const QString& upgradeItem,const QString& lliurexVersion){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Receiveing state: "<<static_cast<int>(actionCode);
    closeAllNotifications();
    setCurrentStackIndex(0);
    setShowDetailsBtn(false);

    QString icon = "lliurex-auto-upgrade-running";
    bool useHeadText = false;
    bool appendWarning = false;
    bool useFootText = false;
    bool useUpdateFootText = false;

    bool showNotification = false;
    bool persistentNotification=false;
    QString eventId;

    qDebug()<<"ITEM: "<<upgradeItem;

    switch (actionCode) {
        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::ReadyToCheck:
            notificationBody = i18n("Ready to check status");
            icon = "lliurex-auto-upgrade-warning";
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::CheckingStatus:
            notificationBody = i18n("Checking status");
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::InstallingPackages:
            notificationBody = i18n("Installing packages");
            appendWarning = true;
            showNotification = true;
            persistentNotification=true;
            eventId=QStringLiteral("InstallAction");
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::PackagesInstalled:
            notificationBody = i18n("Updates installed");
            icon = "lliurex-auto-upgrade-ok";
            useFootText = true;
            useHeadText = true;
            showNotification=true;
            eventId=QStringLiteral("InstallAction");
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::NoChanges:
            notificationBody = i18n("No changes");
            icon = "lliurex-auto-upgrade-ok";
            useFootText = true;
            useHeadText = true;
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::ProcessError:
            notificationBody = i18n("Error, process not completed");
            icon = "lliurex-auto-upgrade-error";
            useFootText = true;
            useHeadText = true;
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::StartingAutoUpgrade:
            notificationBody=i18n("Ready to start unattended upgrade in %1 seconds",waitTime);
            icon = "lliurex-auto-upgrade-warning";
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::GatheringPackages:
            notificationBody=i18n("Gathering the package list for the update");
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::DownloadingComponent:
            notificationBody=i18n("Downloading updates for %1",upgradeItem);
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::ComponentDownloaded:
            notificationBody=i18n("Updates for %1 have been downloaded",upgradeItem);
            lastUpgradeItem=upgradeItem;
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::FullDownloadedWait:
            notificationBody=i18n("All updates downloaded") % "\n" % updateFoot;
            icon="lliurex-auto-upgrade-ok";
            useHeadText=true;
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::FullDownloaded:
            notificationBody=i18n("All updates downloaded");
            lastUpgradeItem="";
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::DownloadLimit:
            if (!lastUpgradeItem.isEmpty()){
                notificationBody=lastUpgradeDownloaded % " " %lastUpgradeItem % "\n" % i18n("The download of updates has reached its limit.");
                icon = "lliurex-auto-upgrade-ok";
                useHeadText=true;
                useUpdateFootText=true;
            }else{
                notificationBody=i18n("The download of updates has reached its limit.");
            }
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::UpdatingComponent:
            notificationBody=i18n("Installing updates for %1",upgradeItem);
            appendWarning = true;
            showNotification=true;
            persistentNotification=true;
            eventId=QStringLiteral("UpdateAction");
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::ComponentUpdated:
            notificationBody=i18n("Updates for %1 have been installed",upgradeItem);
            icon = "lliurex-auto-upgrade-ok";
            lastUpgradeItem=upgradeItem;
            useHeadText=true;
            useUpdateFootText=true;
            showNotification=true;
            showUpdatedNotification=true;
            eventId=QStringLiteral("InstallAction");
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::SystemUpdated:
            notificationBody=i18n("The system is up to date:") % " " % lliurexVersion;
            icon = "lliurex-auto-upgrade-ok";
            useHeadText=true;
            useFootText=true;
            lastUpgradeItem="";
            if (showUpdatedNotification){
                showNotification=true;
                showUpdatedNotification=false;
            }
            eventId=QStringLiteral("UpdateAction");
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::UpdateLimit:
            if (!lastUpgradeItem.isEmpty()){
                notificationBody=lastUpgradeInstalled % " " % lastUpgradeItem % "\n" % i18n("The update process has reached its limit.");
                icon = "lliurex-auto-upgrade-ok";
                useHeadText=true;
                useUpdateFootText=true;
            }else{
                notificationBody=i18n("The update process has reached its limit.");
            }
            break;

        case LliurexAutoUpgradeWidgetUtils::UpgradeAction::UpdatedError:
            notificationBody=i18n("Error, the update has failed. Canceling process");
            icon = "lliurex-auto-upgrade-error";
            useHeadText=true;
            break;

        default:
            qDebug() << "[LLIUREX-AUTO-UPGRADE]: Unknown action code received:" << static_cast<int>(actionCode);
            return;
    }

    if (showNotification){
        QString titleText= notificationBody;
        if (appendWarning){
            titleText=titleText % ". " % turnOffWarning;
        }
        if (persistentNotification){
            sendNotification();
        }else{
            m_notification = new KNotification(eventId, KNotification::CloseOnTimeout, this);
            m_notification->setComponentName(QStringLiteral("lliurexautoupgrade"));
            m_notification->setTitle(titleText);
            m_notification->setText("");
            m_notification->setIconName(icon);
            m_notification->sendEvent();
        }
    }
   
    if (useFootText){
        notificationBody=notificationBody % "\n" % notificationFoot;
    }else if (useUpdateFootText){
        notificationBody=notificationBody % " " % updateLimitFoot;
    }

    setIconName(icon);
    setIconNamePh(icon);

    QString messagePh=notificationBody;
    messagePh.replace("\n","\n\n");
    
    if (useHeadText) {
        QString headText = notificationHead % " "  % lastExecutionTime;
        setSubToolTip(headText % "\n" % notificationBody);
        setMessagePh(headText % "\n" % messagePh);
    } else if (appendWarning) {
        setSubToolTip(notificationBody % ". " % turnOffWarning);
        setMessagePh(messagePh % ". " % turnOffWarning);
    } else {
        setSubToolTip(notificationBody);
        setMessagePh(messagePh);
    }

    if (actionCode == LliurexAutoUpgradeWidgetUtils::UpgradeAction::PackagesInstalled || actionCode == LliurexAutoUpgradeWidgetUtils::UpgradeAction::NoChanges) {
        QStringList installedPkg = m_utils->lastInstalledPkg;
        setLastInstalledPkg(installedPkg);
        setShowDetailsBtn(!installedPkg.isEmpty());
    }
         
}


void LliurexAutoUpgradeWidget::disableApplet(){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Unable to connect with lliurex-auto-upgrade";
    notificationBody=i18n("Unable to get information from LliureX-Auto-Upgrade");
   
    setIconName("lliurex-auto-upgrade-error");
    setIconNamePh("lliurex-auto-upgrade-error");
    setSubToolTip(notificationBody);
    setMessagePh(notificationBody);
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
        
        QDBusConnection::sessionBus().callWithCallback(msg,this,
            SLOT(onNotificationSent(QDBusMessage)),
            SLOT(onNotificationError(QDBusError))
        );
    }

}

void LliurexAutoUpgradeWidget::onNotificationSent(const QDBusMessage &reply){

    if (reply.type()== QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
        lastNotificationId=reply.arguments().at(0).toUInt();
    }else{
        lastNotificationId=0;
    }
}

void LliurexAutoUpgradeWidget::onNotificationError(const QDBusError &error){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Unable to send the notification" << error.message();
    lastNotificationId=0;

}

void LliurexAutoUpgradeWidget::onNotificationClosed(uint id, uint reason){

    if (id==lastNotificationId && reason==2){
        lastNotificationId=0;
    }
}

void LliurexAutoUpgradeWidget::closeAllNotifications(){

    if (m_notification){
        m_notification->close();
        m_notification->deleteLater();
        m_notification=nullptr;
    }

    closeNotificationForced();

}

void LliurexAutoUpgradeWidget::closeNotificationForced(){

    if (lastNotificationId !=0){
        if (QDBusConnection::sessionBus().isConnected()) {
            QDBusMessage closeMsg=QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                       "/org/freedesktop/Notifications",
                                       "org.freedesktop.Notifications",
                                       "CloseNotification" 
                                       );
            closeMsg << lastNotificationId;
            QDBusConnection::sessionBus().send(closeMsg);
            lastNotificationId=0;
        }
    }
}

LliurexAutoUpgradeWidget::TrayStatus LliurexAutoUpgradeWidget::status() const
{
    return m_status;
}

void LliurexAutoUpgradeWidget::changeTryIconState(int state){

    switch(state){
        case 0:
            setStatus(ActiveStatus);
            setToolTip(notificationTitle);
            break;
        case 1:
            setStatus(PassiveStatus);
            break;
        case 2: {
            setIconName("lliurex-auto-upgrade");
            setIconNamePh("lliurex-auto-upgrade");
            QString message=i18n("LliureX-Auto-Upgrade is not enabled in this computer");
            setSubToolTip(message);
            setMessagePh(message);
            setStatus(HiddenStatus);
            break;
        }
        default:
            break;
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

QString LliurexAutoUpgradeWidget::messagePh() const
{
    return m_messagePh;
}

void LliurexAutoUpgradeWidget::setMessagePh(const QString &message)
{
    if (m_messagePh != message) {
        m_messagePh = message;
        emit messagePhChanged();
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
