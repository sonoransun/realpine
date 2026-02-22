import SwiftUI

@main
struct AlpineApp: App {
    @State private var settingsStore = SettingsStore()
    private let secureStorage = SecureStorage()
    @State private var authManager: AuthManager?
    @State private var networkMonitor = NetworkMonitor()
    @State private var searchHistoryStore = SearchHistoryStore()

    var body: some Scene {
        WindowGroup {
            if let authManager {
                AuthGateView {
                    ContentView()
                }
                .environment(settingsStore)
                .environment(secureStorage)
                .environment(authManager)
                .environment(networkMonitor)
                .environment(searchHistoryStore)
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
