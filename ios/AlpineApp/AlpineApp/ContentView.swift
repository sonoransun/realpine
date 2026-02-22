import SwiftUI

struct ContentView: View {
    @Environment(SettingsStore.self) private var settings
    @Environment(SecureStorage.self) private var secureStorage
    @Environment(AuthManager.self) private var authManager

    var body: some View {
        TabView {
            Tab("Dashboard", systemImage: "gauge") {
                NavigationStack {
                    DashboardView(settings: settings, secureStorage: secureStorage)
                }
            }

            Tab("Search", systemImage: "magnifyingglass") {
                NavigationStack {
                    SearchView(settings: settings, secureStorage: secureStorage)
                }
            }

            Tab("Peers", systemImage: "network") {
                NavigationStack {
                    PeersView(settings: settings, secureStorage: secureStorage)
                }
            }

            Tab("Groups", systemImage: "folder") {
                NavigationStack {
                    GroupsView(settings: settings, secureStorage: secureStorage)
                }
            }
        }
        .tint(AlpineTheme.alpineGreen)
    }
}

#Preview {
    let settings = SettingsStore()
    let storage = SecureStorage()
    ContentView()
        .environment(settings)
        .environment(storage)
        .environment(AuthManager(secureStorage: storage, settingsStore: settings))
}
