#ifndef PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H
#define PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QDBusInterface>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariantMap>


class LliurexAutoUpgradeWidgetUtils : public QObject
{
    Q_OBJECT


public:

    static LliurexAutoUpgradeWidgetUtils& instance(){
        static LliurexAutoUpgradeWidgetUtils _instance;
        return _instance;
    }

    LliurexAutoUpgradeWidgetUtils(const LliurexAutoUpgradeWidgetUtils&)=delete;
    LliurexAutoUpgradeWidgetUtils& operator=(const LliurexAutoUpgradeWidgetUtils&) =delete;

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
        FullDownloadedWait=11,
        FullDownloaded = 12,
        DownloadLimit = 13,
        UpdatingComponent = 14,
        ComponentUpdated = 15,
        SystemUpdated = 16,
        UpdateLimit = 17,
        UpdatedError = 18
    };

   Q_ENUM(UpgradeAction);   
   
   QString m_unitName="lliurex-auto-upgrade.service";
  
   
   bool startListener();

   QStringList getPkgsInstalledInSession() const;

   void startWidget();
   void createSubscription();


signals:

    void startWidgetFinished(bool showWidget,bool startOk);
    void unitStateChanged(UpgradeAction actionCode, const QString& lastExecutionTime, const QString& waitTime, const QString& upgradeItem, const QString& lliurexVersion);
    void subscriptionFinished(bool success, QString error );

private:

    explicit LliurexAutoUpgradeWidgetUtils();

    UpgradeAction actionCode=UpgradeAction::ReadyToCheck;
    QDBusInterface *managerInterface;

    bool checkFailed=false;
    bool updatedFailed=false;
    
    bool isSubscribed=false;
    bool isInitializing=false;
    bool isSubscribing=false;
    bool interfaceCreated=false;

    QString lastUpdate;
    QString disableAutoUpgrade="/etc/lliurex-auto-upgrade/disabled";
    QString pkgInstalledLog="/run/lliurex-auto-upgrade/installed_packages.log";
    QStringList upgradeItems;

    QString lastExecution="";
    QString upgradeItem="";
    QString waitTime="";
    QString lliurexVersion="";

    QSet<QString>lastInstalledPkg;

    bool createInterface();

    QString getLastExecutionTime();
    QString getUpgradeItem(const QString &message);
    QString getWaitTimeForUpgrade(const QString &message);
    
    void getLliurexVersion();
    void getLastInstalledPkg(QString instaledPkg);
    
private slots:

    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
 
};

Q_DECLARE_METATYPE(LliurexAutoUpgradeWidgetUtils::UpgradeAction)
#endif // PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H
