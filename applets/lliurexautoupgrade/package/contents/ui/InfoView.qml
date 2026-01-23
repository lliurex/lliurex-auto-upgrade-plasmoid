import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.lliurexautoupgrade 1.0
// Item - the most basic plasmoid component, an empty container.

Item {
   
    PC3.Page {
        anchors.centerIn:parent
        implicitWidth: PlasmaCore.Units.gridUnit * 12
        implicitHeight: PlasmaCore.Units.gridUnit * 6
        
        PlasmaExtras.PlaceholderMessage {
            id:phMsg
            anchors.centerIn: parent
            width: parent.width*2 - (PlasmaCore.Units.gridUnit * 4)
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
