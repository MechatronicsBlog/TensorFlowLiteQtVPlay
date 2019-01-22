import VPlayApps 1.0
import QtQuick 2.11
import QtQuick.Controls 1.4

Page {
    title: qsTr("Settings")
    id: root

    // Properties
    property double minConfidence
    property string model
    property bool   showTime
    property int    nThreads
    property bool   acceleration

    PopupMessage {
        id: message
        timeout: 5000
    }

    AppFlickable {
        id: flick
        anchors.fill: parent
        contentWidth: column.width;
        contentHeight: column.height

        Column{
            id:    column
            width: root.width

            Item{ height: dp(20); width: 1 }

            AppText{
                anchors.margins: dp(20)
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: AppText.AlignHCenter
                width: parent.width
                wrapMode: AppText.WordWrap
                elide: AppText.ElideRight
                text: qsTr("Minimum confidence")
            }

            Item{ height: dp(10); width: 1 }

            AppSlider{
                id: slider
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2*dp(20)
                from:  0
                to:    1
                value: minConfidence
                live:  true
                onValueChanged: minConfidence = value
            }

            AppText {
                anchors.margins: dp(20)
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: AppText.AlignHCenter
                width: parent.width
                wrapMode: AppText.WordWrap
                elide: AppText.ElideRight
                color: Theme.tintColor
                text: Math.round(slider.position * 100) + " %"
            }

            Item{ height: dp(30); width: 1 }

            Row {
                width: parent.width
                spacing: dp(2)

                Item{
                    height: 1
                    width: (parent.width - parent.spacing - tThread.width - iThread.width)*0.5
                }

                AppText{
                    id: tThread
                    anchors.leftMargin: dp(30)
                    anchors.verticalCenter: parent.verticalCenter
                    verticalAlignment: AppText.AlignVCenter
                    wrapMode: AppText.WordWrap
                    elide: AppText.ElideRight
                    text: qsTr("Number of threads")
                }

                IconButton{
                    id: iThread
                    icon: IconType.infocircle
                    color: Theme.tintColor
                    onClicked: message.show(sThreads.to + " " + (sThreads.to>1 ? qsTr("cores") : qsTr("core")) + " " + qsTr("detected"))
                }
            }

            //Item{ height: dp(10); width: 1 }

            AppSlider{
                id: sThreads
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2*dp(15)
                from:  1
                to:    auxUtils.numberThreads()
                enabled: to>1
                live:  true
                snapMode: AppSlider.SnapAlways
                stepSize: 1
                value: nThreads
                onValueChanged: nThreads = value
            }

            AppText {
                anchors.leftMargin:  dp(5)
                anchors.rightMargin: dp(5)
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: AppText.AlignHCenter
                width: parent.width
                wrapMode: AppText.WordWrap
                elide: AppText.ElideRight
                color: Theme.tintColor
                text: sThreads.value + " " + (sThreads.value>1 ? qsTr("threads") : qsTr("thread"))
            }

            Item{
                height: dp(30)
                width:  1
            }

            Row{
                width: parent.width - 2*dp(20)
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: width - tShowInfTime.width - sShowInfTime.width

                AppText {
                    id: tShowInfTime
                    text: qsTr("Show inference time")
                    anchors.verticalCenter: parent.verticalCenter
                    verticalAlignment: AppText.AlignVCenter
                }

                AppSwitch{
                    anchors.verticalCenter: parent.verticalCenter
                    id: sShowInfTime
                    checked: showTime
                    onToggled: showTime = checked
                }
            }

            Item{ height: dp(20); width: 1 }

            Row{
                width: parent.width - 2*dp(20)
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: width - rAcceleration.width - sAcceleration.width

                Row {
                    id: rAcceleration
                    spacing: dp(2)
                    width: tAcceleration.width + iAcceleration.width + spacing

                    AppText {
                        id: tAcceleration
                        text: qsTr("Android Neural Networks")
                        anchors.verticalCenter: parent.verticalCenter
                        verticalAlignment: AppText.AlignVCenter
                        wrapMode: AppText.NoWrap
                        elide: AppText.ElideRight
                    }

                    IconButton{
                        id: iAcceleration
                        icon: IconType.infocircle
                        color: sAcceleration.enabled ? Theme.tintColor : "red"
                        onClicked: message.show(qsTr("Only for Android 8.1 or higher"),message.defTimeout,sAcceleration.enabled?message.defColor:"red")
                    }
                }

                AppSwitch{
                    anchors.verticalCenter: parent.verticalCenter
                    id: sAcceleration
                    enabled: Qt.platform.os === "android"
                    checked: enabled ? acceleration : false
                    onToggled: acceleration = checked
                }
            }

            Item{ height: dp(40); width: 1 }

            AppText{
                anchors.margins: dp(20)
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: AppText.AlignHCenter
                width: parent.width
                wrapMode: AppText.WordWrap
                elide: AppText.ElideRight
                text: qsTr("Tensorflow Lite model")
            }

            Item{ height: dp(30); width: 1 }

            ExclusiveGroup { id: modelGroup }

            AppCheckBox{
                id: chkClassification
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2*dp(20)
                text: qsTr("Image classification")
                exclusiveGroup: modelGroup
                checked: model === "ImageClassification"
                onCheckedChanged: if (checked) model = "ImageClassification"; else chkDetection.checked = true
            }

            Item{ height: dp(20); width: 1 }

            AppCheckBox{
                id: chkDetection
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2*dp(20)
                text: qsTr("Object detection")
                exclusiveGroup: modelGroup
                checked: model === "ObjectDetection"
                onCheckedChanged: if (checked) model = "ObjectDetection"; else chkClassification.checked = true
            }
        }
    }
}
