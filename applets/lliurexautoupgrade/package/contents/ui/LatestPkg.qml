import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PC3

Item {
    id: root
    implicitWidth: 400
    implicitHeight: 320

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.bottomMargin:25
        spacing: 15

        RowLayout {
            id: headLatestPkg
            Layout.fillWidth: true
            spacing: 10

            PC3.ToolButton {
                id: backButton
                icon.name: "arrow-left"
                Layout.preferredWidth: 35
                Layout.preferredHeight: 35

                PC3.ToolTip {
                    id: backTP
                    text: i18n("Back to main view")
                }

                onClicked: {
                    backTP.hide()
                    lliurexAutoUpgradeWidget.manageNavigation(0)
                }
            }

            PC3.Label {
                id: headPkgText
                text: i18n("List of installed/updated packages")
                font.italic: true
                font.pointSize: 11
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        PC3.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 5
            Layout.rightMargin: 5

            ListView {
                id: listView
                focus: true
                clip: true
                model: lliurexAutoUpgradeWidget.lastInstalledPkg

                delegate: PC3.ItemDelegate {
                    width: listView.width
                    height: 35

                    contentItem: PC3.Label {
                        text: "• " + modelData
                        font.pointSize: 11
                        elide: Text.ElideMiddle
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
