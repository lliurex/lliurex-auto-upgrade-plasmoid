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
   

   LliurexAutoUpgradeWidgetUtils(QObject *parent = nullptr);

   QString m_unitName="lliurex-auto-upgrade.service";
   QStringList lastInstalledPkg;
  
   void startWidget();
   bool startListener();
   void createSubscription();

signals:

    void startWidgetFinished(bool showWidget,bool startOk);
    void unitStateChanged(int actionCode,QString lastExecutionTime);
    void subscriptionFinished(bool success, QString error );

private:    
     
    QString lastUpdate;
    int actionCode=1;
    bool checkFailed=false;
    QString disableAutoUpgrade="/etc/lliurex-auto-upgrade/disabled";
    QString pkgInstalledLog="/run/lliurex-auto-upgrade/installed_packages.log";
    QDBusInterface *managerInterface;

    bool showWidget();
    void getPkgsInstalledInSession();
    bool createInterface();
    void getLastInstalledPkg(QString instaledPkg);
    QString getLastExecutionTime();

private slots:

    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

     
};
#endif // PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H
