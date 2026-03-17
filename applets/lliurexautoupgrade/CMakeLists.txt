
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.lliurexautoupgrade)


#######################################################################################
# Notes Library
set(PLUGIN_TARGET "lliurexautoupgradeplugin")
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.lliurexautoupgrade")
ecm_add_qml_module(${PLUGIN_TARGET} URI org.kde.plasma.private.lliurexautoupgrade VERSION 2.0)


target_sources(${PLUGIN_TARGET} PRIVATE
    plugin/plugin.cpp
    plugin/LliurexAutoUpgradeWidget.cpp
    plugin/LliurexAutoUpgradeWidgetUtils.cpp
    plugin/plugin.h
    plugin/LliurexAutoUpgradeWidget.h
    plugin/LliurexAutoUpgradeWidgetUtils.h
   
)

target_link_libraries(${PLUGIN_TARGET} PRIVATE
                      Qt6::Quick
                      Qt6::Core
                      Qt6::Widgets
                      KF6::CoreAddons
                      KF6::I18n
                      KF6::Notifications
                      KF6::KIOWidgets
                      )

target_include_directories(${PLUGIN_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugin)                      
ecm_finalize_qml_module(${PLUGIN_TARGET})

install(TARGETS ${PLUGIN_TARGET} DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/lliurexautoupgrade)
install(FILES "icons/lliurex-auto-upgrade.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/lliurex-auto-upgrade-ok.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/lliurex-auto-upgrade-error.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/lliurex-auto-upgrade-warning.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "knotifications6/lliurexautoupgrade.notifyrc"
        DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
