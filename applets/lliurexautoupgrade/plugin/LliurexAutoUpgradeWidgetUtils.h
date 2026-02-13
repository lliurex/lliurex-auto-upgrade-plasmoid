#ifndef PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H
#define PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H

#include <QObject>
#include <QFile>
#include <QDir>

using namespace std;


class LliurexAutoUpgradeWidgetUtils : public QObject
{
    Q_OBJECT


public:
   

   LliurexAutoUpgradeWidgetUtils(QObject *parent = nullptr);

   QString m_unitName="lliurex-auto-upgrade.service";
   QStringList lastInstalledPkg;
   void cleanCache();
   bool showWidget();
   bool testListener();
   bool startListener();
   void getPkgsInstalledInSession();

signals:

    void unitStateChanged(int actionCode,QString lastExecutionTime);

private:    
     
    QString user;
    QString lastUpdate;
    int actionCode=1;
    QString disableAutoUpgrade="/etc/lliurex-auto-upgrade/disabled";
    QString pkgInstalledLog="/run/lliurex-auto-upgrade/installed_packages.log";

    QString getInstalledVersion();
    void getLastInstalledPkg(QString instaledPkg);
    QString getLastExecutionTime();

private slots:

    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

     
};
#endif // PLASMA_LLIUREX_AUTO_UPGRADE_WIDGET_UTILS_H
