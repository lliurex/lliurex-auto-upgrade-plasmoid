
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.lliurexautoupgrade)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.lliurexautoupgrade")
ecm_add_qml_module(lliurexautoupgradeplugin URI org.kde.plasma.private.lliurexautoupgrade)

target_sources(lliurexautoupgradeplugin PRIVATE
    plugin/plugin.cpp
    plugin/LliurexAutoUpgradeWidget.cpp
    plugin/LliurexAutoUpgradeWidgetUtils.cpp
   
)

target_link_libraries(lliurexautoupgradeplugin PRIVATE
                      Qt6::Quick
                      Qt6::Core
                      Qt6::Widgets
                      KF6::CoreAddons
                      KF6::I18n
                      KF6::Notifications
                      KF6::KIOWidgets
                      )
ecm_finalize_qml_module(lliurexautoupgradeplugin)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/lliurexautoupgrade)
install(TARGETS lliurexautoupgradeplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/lliurexautoupgrade)
install(FILES "icons/lliurex-auto-upgrade.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/lliurex-auto-upgrade-run.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/lliurex-auto-upgrade-error.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "knotifications6/lliurexautoupgrade.notifyrc"
       DESTINATION ${DATAROOTDIR}/usr/share/knotifications5/)
