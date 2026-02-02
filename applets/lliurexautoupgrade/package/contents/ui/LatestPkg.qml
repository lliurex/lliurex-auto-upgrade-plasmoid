import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as Components
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.lliurexautoupgrade 1.0

Rectangle{
	color:"transparent"
	GridLayout{
        id:filesLayout
        rows: 2
        flow: GridLayout.TopToBottom
        rowSpacing:10
        width:parent.width
        RowLayout{
            id:headLatestPkg
            Layout.fillWidth:true
            PC3.ToolButton {
                height:35
                width:35
                icon.name:"arrow-left.svg"
                Layout.rightMargin:filesLayout.width/3-35-headPkgText.text.length
                PC3.ToolTip{
                    id:backTP   
                    text:i18n("Back to main view")
                }
                onClicked:{
                    backTP.hide()
                    lliurexAutoUpgradeWidget.manageNavigation(0)
                } 
            } 
            Components.Label{
                id:headPkgText
                text:i18n("List of installed/updated packages")
                font.italic:true
                font.pointSize:11
                Layout.fillWidth:true
                Layout.alignment:Qt.AlignHCenter
            }
            
        }
        PC3.ScrollView {
            Layout.topMargin:10
            Layout.bottomMargin:10
            Layout.leftMargin:15
            Layout.rightMargin:10
            implicitWidth:parent.width-10
            implicitHeight:250
            ListView{
                id:listView
                focus:true
                model:lliurexAutoUpgradeWidget.lastInstalledPkg
                delegate: Components.Label {
                    text: "- "+modelData
                    font.pointSize:11
                    height:30
                    width: listView.width-10
                    elide:Text.ElideMiddle

                }
                              
            }
        }
    }

}
