import SwiftUI

struct ContentView: View {
    @Environment(SettingsStore.self) private var settings
    @Environment(SecureStorage.self) private var secureStorage

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
    ContentView()
        .environment(SettingsStore())
        .environment(SecureStorage())
}
