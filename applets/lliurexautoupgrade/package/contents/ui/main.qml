import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PC3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.lliurexautoupgrade 1.0
// Item - the most basic plasmoid component, an empty container.
PlasmoidItem {

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

    switchWidth: Kirigami.Units.gridUnit * 5
    switchHeight: Kirigami.Units.gridUnit * 5

    Plasmoid.icon:lliurexAutoUpgradeWidget.iconName
    toolTipMainText: lliurexAutoUpgradeWidget.toolTip
    toolTipSubText: lliurexAutoUpgradeWidget.subToolTip

     
    fullRepresentation: Item {
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
