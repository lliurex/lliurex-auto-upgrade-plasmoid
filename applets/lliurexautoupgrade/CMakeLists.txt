
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.lliurexautoupgrade)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.lliurexautoupgrade")

set(lliurexautoupgradewidget_SRCS
    plugin/plugin.cpp
    plugin/LliurexAutoUpgradeWidget.cpp
    plugin/LliurexAutoUpgradeWidgetUtils.cpp
   
)

add_library(lliurexautoupgradeplugin SHARED ${lliurexautoupgradewidget_SRCS})

target_link_libraries(lliurexautoupgradeplugin
                      Qt5::Quick
                      Qt5::Core
                      Qt5::Widgets
                      KF5::CoreAddons
                      KF5::I18n
                      KF5::Notifications
                      KF5::KIOWidgets
  
                      )

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/lliurexautoupgrade)
install(TARGETS lliurexautoupgradeplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/lliurexautoupgrade)
install(FILES "icons/lliurex-auto-upgrade.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/lliurex-auto-upgrade-ok.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/lliurex-auto-upgrade-error.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "knotifications5/lliurexautoupgrade.notifyrc"
       DESTINATION ${DATAROOTDIR}/usr/share/knotifications5/)
