#ifndef LLIUREX_AUTO_UPGRADE_PLUGIN_H
#define LLIUREX_AUTO_UPGRADE_PLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class LliurexAutoUpgradePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
/*    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")*/
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) Q_DECL_OVERRIDE;
};

#endif // LLIUREX_AUTO_UPGRADE_PLUGIN_H
