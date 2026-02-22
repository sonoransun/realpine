import SwiftUI

@main
struct AlpineApp: App {
    @State private var settingsStore = SettingsStore()
    private let secureStorage = SecureStorage()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(settingsStore)
                .environment(secureStorage)
        }
    }
}
