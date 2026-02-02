import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PC3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.lliurexautoupgrade 1.0
// Item - the most basic plasmoid component, an empty container.

Item {
   
    PC3.Page {
        anchors.centerIn:parent
        implicitWidth: Kirigami.Units.gridUnit * 12
        implicitHeight: Kirigami.Units.gridUnit * 6
        
        PlasmaExtras.PlaceholderMessage {
            id:phMsg
            anchors.centerIn: parent
            width: parent.width*2 - (Kirigami.Units.gridUnit * 4)
            iconName: lliurexAutoUpgradeWidget.iconNamePh
            text:lliurexAutoUpgradeWidget.subToolTip
        }

        PC3.Button{
            id:showDetailsBtn
            height:35
            anchors.top:phMsg.bottom
            anchors.horizontalCenter:phMsg.horizontalCenter
            display:AbstractButton.TextBesideIcon
            visible:lliurexAutoUpgradeWidget.showDetailsBtn
            icon.name:"appointment-new"
            text:i18n("View installed/updated packages")
            onClicked:{
                lliurexAutoUpgradeWidget.manageNavigation(1)
            }

        }

    }

}  
