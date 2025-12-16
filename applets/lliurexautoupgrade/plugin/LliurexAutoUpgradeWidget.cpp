#include "LliurexAutoUpgradeWidget.h"
#include "LliurexAutoUpgradeWidgetUtils.h"

#include <KLocalizedString>
#include <KFormat>
#include <KNotification>
#include <KRun>
#include <QTimer>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QThread>
#include <QtCore/QStringList>
#include <QJsonObject>
#include <QDebug>
#include <KIO/CommandLauncherJob>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>


LliurexAutoUpgradeWidget::LliurexAutoUpgradeWidget(QObject *parent)
    : QObject(parent)
    , m_utils(new LliurexAutoUpgradeWidgetUtils(this))

   
{
    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Iniciando";
    m_utils->cleanCache();
    notificationTitle=i18n("LliureX-Auto-Upgrade");
    setSubToolTip(notificationTitle);
    plasmoidMode();

}  

void LliurexAutoUpgradeWidget::plasmoidMode(){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Iniciando";

    if (!m_utils->showWidget()){
       changeTryIconState(2);
    }else{
        qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Comprobando dBus";
        if (m_utils->testListener()){
            if (m_utils->startListener()){
                connect(m_utils,&LliurexAutoUpgradeWidgetUtils::unitStateChanged,this,&LliurexAutoUpgradeWidget::manageState);
                changeTryIconState(0);
            }else{
                disableApplet();
            }
        }else{
            disableApplet();
        }
      

    }

}

void LliurexAutoUpgradeWidget::manageState(const QString &state){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Receiveing state"<<state;
}


void LliurexAutoUpgradeWidget::disableApplet(){

    qDebug()<<"[LLIUREX-AUTO-UPGRADE]: Desactivando";
    notificationBody=i18n("LliureX-Auto-Upgrade not available in this computer");
   
    setIconName("lliurex-auto-upgrade");
    setIconNamePh("lliurex-auto-upgrade");
    setSubToolTip(notificationBody);
    changeTryIconState(1);

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
        setStatus(HiddenStatus);
    }

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


