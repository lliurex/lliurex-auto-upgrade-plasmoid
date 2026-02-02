import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.lliurexautoupgrade 1.0
// Item - the most basic plasmoid component, an empty container.
Item {

    id:lliurexAutoUpgradeApplet
    
    LliurexAutoUpgradeWidget{
        id:lliurexAutoUpgradeWidget

    }


    Plasmoid.status: {
        /* Warn! Enum types are accesed through ClassName not ObjectName */
        switch (lliurexAutoUpgradeWidget.status){
            case LliurexAutoUpgradeWidget.ActiveStatus:
                return PlasmaCore.Types.ActiveStatus
            case LliurexAutoUpgradeWidget.PassiveStatus:
                return PlasmaCore.Types.PassiveStatus
            case LliurexAutoUpgradeWidget.HiddenStatus:
                return PlasmaCore.Types.HiddenStatus
         
        }
        return  PlasmaCore.Types.ActiveStatus
        
    }

    Plasmoid.switchWidth: units.gridUnit * 5
    Plasmoid.switchHeight: units.gridUnit * 5

    Plasmoid.icon:lliurexAutoUpgradeWidget.iconName
    Plasmoid.toolTipMainText: lliurexAutoUpgradeWidget.toolTip
    Plasmoid.toolTipSubText: lliurexAutoUpgradeWidget.subToolTip

    Component.onCompleted: {
       plasmoid.removeAction("configure");
    }

   
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.fullRepresentation: Item {
        id:root
        Layout.fillWidth:true
        QQC2.StackView{
            id:stackLayout
            property int currentIndex:lliurexAutoUpgradeWidget.currentStackIndex
            width:parent.width
            height:parent.height
            initialItem:infoPanel
            onCurrentIndexChanged:{
                switch(currentIndex){
                    case 0:
                        stackLayout.replace(infoPanel)
                        break;
                    
                    case 1:
                        stackLayout.replace(pkgInstalled)
                        break;
                    
                }
            }

            Component{
                id:infoPanel
                InfoView{
                    id:infoView
                }
            }

            Component{
                id:pkgInstalled
                LatestPkg{
                    id:latestPkg
                }
            }
            
        }

    }
} 
