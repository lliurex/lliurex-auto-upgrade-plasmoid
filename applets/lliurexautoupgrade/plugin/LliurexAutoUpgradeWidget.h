#ifndef PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_H
#define PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_H

#include <QObject>
#include <QProcess>
#include <QPointer>
#include <KNotification>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QFileSystemWatcher>
#include <KIO/CommandLauncherJob>

#include "LliurexAutoUpgradeWidgetUtils.h"


class QTimer;
class KNotification;
class AsyncDbus;


class LliurexAutoUpgradeWidget : public QObject
{
    Q_OBJECT


    Q_PROPERTY(TrayStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString toolTip READ toolTip NOTIFY toolTipChanged)
    Q_PROPERTY(QString subToolTip READ subToolTip NOTIFY subToolTipChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(QString iconNamePh READ iconNamePh NOTIFY iconNamePhChanged)
    Q_ENUMS(TrayStatus)

public:
    /**
     * System tray icon states.
     */
    enum TrayStatus {
        ActiveStatus=0,
        PassiveStatus,
        HiddenStatus
    };

    LliurexAutoUpgradeWidget(QObject *parent = nullptr);

    TrayStatus status() const;
    void changeTryIconState (int state);
    void setStatus(TrayStatus status);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    QString subToolTip() const;
    void setSubToolTip(const QString &subToolTip);

    QString iconName() const;
    void setIconName(const QString &name);

    QString iconNamePh() const;
    void setIconNamePh(const QString &name);

signals:
   
    void toolTipChanged();
    void subToolTipChanged();
    void iconNameChanged();
    void iconNamePhChanged();
    void statusChanged();
 
private:

    TrayStatus m_status = PassiveStatus;
    QString m_iconName = QStringLiteral("lliurex-auto-upgrade");
    QString m_iconNamePh = QStringLiteral("lliurex-auto-upgrade");
    QString m_toolTip;
    QString m_subToolTip;
    QString notificationTitle;
    QString notificationBody;
    QString notificationServerBody;
    LliurexAutoUpgradeWidgetUtils *m_utils;
    QPointer<KNotification> m_notification;
    void plasmoidMode();
    void disableApplet();

private slots:
    
    void manageState(const QString &newState);


};


#endif // PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_H
