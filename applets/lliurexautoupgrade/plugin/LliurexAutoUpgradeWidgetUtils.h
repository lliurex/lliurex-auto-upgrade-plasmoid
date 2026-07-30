#ifndef PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H
#define PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QDBusInterface>

using namespace std;


class LliurexAutoUpgradeWidgetUtils : public QObject
{
    Q_OBJECT


public:

    enum class UpgradeAction {
        ReadyToCheck = 1,
        CheckingStatus = 2,
        InstallingPackages = 3,
        PackagesInstalled = 4,
        NoChanges = 5,
        ProcessError = 6,
        StartingAutoUpgrade = 7,
        GatheringPackages = 8,
        DownloadingComponent = 9,
        ComponentDownloaded = 10,
        FullDownloaded = 11,
        DownloadLimit = 12,
        UpdatingComponent = 13,
        ComponentUpdated = 14,
        SystemUpdated = 15,
        UpdateLimit = 16,
        UpdatedError = 17
    };    
   

   LliurexAutoUpgradeWidgetUtils(QObject *parent = nullptr);

   QString m_unitName="lliurex-auto-upgrade.service";
   QStringList lastInstalledPkg;
  
   void startWidget();
   bool startListener();
   void createSubscription();

signals:

    void startWidgetFinished(bool showWidget,bool startOk);
    void unitStateChanged(UpgradeAction actionCode,QString lastExecutionTime, QString waitTime, QString upgradeItem);
    void subscriptionFinished(bool success, QString error );

private:

    QString lastUpdate;
    UpgradeAction actionCode;
    bool checkFailed=false;
    bool updatedFailed=false;
    QString disableAutoUpgrade="/etc/lliurex-auto-upgrade/disabled";
    QString pkgInstalledLog="/run/lliurex-auto-upgrade/installed_packages.log";
    QStringList upgradeItems={"lliurex","security","ubuntu","kernel"};
    QDBusInterface *managerInterface;

    bool showWidget();
    void getPkgsInstalledInSession();
    bool createInterface();
    void getLastInstalledPkg(QString instaledPkg);
    QString getLastExecutionTime();
    QString getUpgradeItem(QString &message);
    QString getWaitTimeForUpgrade(QString &message);

private slots:

    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

     
};

Q_DECLARE_METATYPE(LliurexAutoUpgradeWidgetUtils::UpgradeAction)
#endif // PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H
