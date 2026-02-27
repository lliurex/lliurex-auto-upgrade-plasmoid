#include "plugin.h"
#include "LliurexAutoUpgradeWidget.h"

#include <QtQml>

void LliurexAutoUpgradePlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.private.lliurexautoupgrade"));
    qmlRegisterType<LliurexAutoUpgradeWidget>(uri, 1, 0, "LliurexAutoUpgradeWidget");
}
