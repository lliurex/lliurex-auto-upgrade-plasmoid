#ifndef PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_H
#define PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_H

#include <QObject>
#include <QPointer>
#include <KNotification>
#include <QDBusInterface>

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
    Q_PROPERTY(bool showDetailsBtn READ showDetailsBtn NOTIFY showDetailsBtnChanged)
    Q_PROPERTY(int currentStackIndex READ currentStackIndex NOTIFY currentStackIndexChanged)
    Q_PROPERTY(QStringList lastInstalledPkg READ lastInstalledPkg NOTIFY lastInstalledPkgChanged)
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

    bool showDetailsBtn();
    void setShowDetailsBtn(bool);

    int currentStackIndex();
    void setCurrentStackIndex(int);

    QStringList lastInstalledPkg();
    void setLastInstalledPkg(QStringList lastInstalledPkgChanged);

public slots:

    void manageNavigation(int stackIndex);
    
signals:
   
    void toolTipChanged();
    void subToolTipChanged();
    void iconNameChanged();
    void iconNamePhChanged();
    void statusChanged();
    void showDetailsBtnChanged();
    void currentStackIndexChanged();
    void lastInstalledPkgChanged();
 
private:

    TrayStatus m_status = PassiveStatus;
    QString m_iconName = QStringLiteral("lliurex-auto-upgrade-warning");
    QString m_iconNamePh = QStringLiteral("lliurex-auto-upgrade-warning");
    QString m_toolTip;
    QString m_subToolTip;
    bool m_showDetailsBtn=false;
    int m_currentStackIndex=0;
    QStringList m_lastInstalledPkg;
    QString notificationTitle;
    QString notificationBody;
    QString notificationServerBody;
    uint lastNotificationId;
    QDBusInterface *notifyInterface; 
    LliurexAutoUpgradeWidgetUtils *m_utils;
    QPointer<KNotification> m_notification;
    void plasmoidMode();
    void disableApplet();
    void closeAllNotifications();
    void sendNotification();

private slots:
    
    void manageState(int actionCode,QStringList installedPkg);


};


#endif // PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_H
