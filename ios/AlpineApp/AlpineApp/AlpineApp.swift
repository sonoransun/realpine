import SwiftUI

@main
struct AlpineApp: App {
    @State private var settingsStore = SettingsStore()
    private let secureStorage = SecureStorage()
    @State private var authManager: AuthManager?

    var body: some Scene {
        WindowGroup {
            if let authManager {
                AuthGateView {
                    ContentView()
                }
                .environment(settingsStore)
                .environment(secureStorage)
                .environment(authManager)
            } else {
                ProgressView()
                    .onAppear {
                        authManager = AuthManager(
                            secureStorage: secureStorage,
                            settingsStore: settingsStore
                        )
                    }
            }
        }
    }
}
