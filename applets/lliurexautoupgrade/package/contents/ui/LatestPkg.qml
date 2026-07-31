import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.components as Components

Item {
    id: root
    implicitWidth: 400
    implicitHeight: 320

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.bottomMargin:20
        spacing: 15

        RowLayout {
            id: headLatestPkg
            Layout.fillWidth: true
            spacing: 10

            Components.ToolButton {
                id: backButton
                icon.name: "arrow-left"
                Layout.preferredWidth: 35
                Layout.preferredHeight: 35

                Components.ToolTip {
                    id: backTP
                    text: i18n("Back to main view")
                }

                onClicked: {
                    backTP.hide()
                    lliurexAutoUpgradeWidget.manageNavigation(0)
                }
            }

            Components.Label {
                id: headPkgText
                text: i18n("List of installed/updated packages")
                font.italic: true
                font.pointSize: 11
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Components.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 5
            Layout.rightMargin: 5

            ListView {
                id: listView
                focus: true
                clip: true
                model: lliurexAutoUpgradeWidget.lastInstalledPkg

                delegate: Components.ItemDelegate {
                    width: listView.width
                    height: 35

                    contentItem: Components.Label {
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
