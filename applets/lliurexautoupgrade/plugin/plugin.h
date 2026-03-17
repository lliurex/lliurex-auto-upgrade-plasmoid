#ifndef LLIUREX_AUTO_UPGRADE_PLUGIN_H
#define LLIUREX_AUTO_UPGRADE_PLUGIN_H

#include <QQmlEngineExtensionPlugin>

class LliurexAutoUpgradePlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)

public:
        using QQmlEngineExtensionPlugin::QQmlEngineExtensionPlugin;
};

#endif // LLIUREX_AUTO_UPGRADE_PLUGIN_H


