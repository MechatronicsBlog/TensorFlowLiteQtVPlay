import VPlayApps 1.0
import VPlay 2.0
import QtQuick 2.0

App {
    // You get free licenseKeys from https://v-play.net/licenseKey
    // With a licenseKey you can:
    //  * Publish your games & apps for the app stores
    //  * Remove the V-Play Splash Screen or set a custom one (available with the Pro Licenses)
    //  * Add plugins to monetize, analyze & improve your apps (available with the Pro Licenses)
    //licenseKey: "<generate one from https://v-play.net/licenseKey>"

    id: app

    // Storage keys
    readonly property string kMinConfidence: "MinConfidence"
    readonly property string kModel:         "Model"
    readonly property string kShowTime:      "ShowTime"
    readonly property string kNThreads:      "NThreads"
    readonly property string kAcceleration:  "Acceleration"

    // Default values
    readonly property double defMinConfidence: 0.5
    readonly property string defModel: "ImageClassification"
    readonly property bool   defShowTime: false
    readonly property int    defNThreads: 1
    readonly property bool   defAcceleration: false

    // Properties
    property double minConfidence
    property string model
    property bool   showTime
    property int    nThreads
    property bool   acceleration

    // Local storage component
    Storage {
        id: storage

        Component.onCompleted: {
            minConfidence = getValue(kMinConfidence) !== undefined ? getValue(kMinConfidence) : defMinConfidence
            model         = getValue(kModel)         !== undefined ? getValue(kModel)         : defModel
            showTime      = getValue(kShowTime)      !== undefined ? getValue(kShowTime)      : defShowTime
            nThreads      = getValue(kNThreads)      !== undefined ? getValue(kNThreads)      : defNThreads
            acceleration  = getValue(kAcceleration)  !== undefined ? getValue(kAcceleration)  : defAcceleration
        }
    }

    Navigation {

        NavigationItem{
            title: qsTr("Live")
            icon: IconType.rss

            NavigationStack{
                VideoPage{
                    id: videoPage
                    minConfidence: app.minConfidence
                    model: app.model
                    showTime: app.showTime
                    nThreads: app.nThreads
                    acceleration: app.acceleration
                }
            }
        }

        NavigationItem{
            title: qsTr("Settings")
            icon: IconType.sliders

            NavigationStack{
                AppSettingsPage{
                    id: appSettingsPage
                    minConfidence: app.minConfidence
                    model: app.model
                    showTime: app.showTime
                    nThreads: app.nThreads
                    acceleration: app.acceleration

                    onMinConfidenceChanged: {
                        app.minConfidence = appSettingsPage.minConfidence
                        storage.setValue(kMinConfidence,app.minConfidence)
                    }

                    onModelChanged: {
                        app.model = appSettingsPage.model
                        storage.setValue(kModel,app.model)
                    }

                    onShowTimeChanged: {
                        app.showTime = appSettingsPage.showTime
                        storage.setValue(kShowTime,app.showTime)
                    }

                    onNThreadsChanged: {
                        app.nThreads = appSettingsPage.nThreads
                        storage.setValue(kNThreads,app.nThreads)
                    }

                    onAccelerationChanged: {
                        app.acceleration = appSettingsPage.acceleration
                        storage.setValue(kAcceleration,app.acceleration)
                    }
                }
            }
        }
    }
}
