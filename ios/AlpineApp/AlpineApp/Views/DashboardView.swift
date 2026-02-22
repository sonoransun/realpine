import SwiftUI

struct DashboardView: View {
    @State private var viewModel: DashboardViewModel

    private let settings: SettingsStore
    private let secureStorage: SecureStorage

    init(settings: SettingsStore, secureStorage: SecureStorage) {
        self.settings = settings
        self.secureStorage = secureStorage
        _viewModel = State(wrappedValue: DashboardViewModel(
            settings: settings,
            secureStorage: secureStorage
        ))
    }

    var body: some View {
        ScrollView {
            VStack(spacing: 16) {
                if viewModel.isLoading && viewModel.serverStatus == nil {
                    loadingSection
                } else if let error = viewModel.error {
                    errorSection(error)
                } else {
                    serverInfoSection
                    statsSection
                    defaultGroupSection
                    navigationSection
                }
            }
            .padding()
        }
        .navigationTitle("Alpine")
        .toolbar {
            ToolbarItem(placement: .topBarLeading) {
                ConnectionIndicator(isConnected: viewModel.isConnected, showLabel: true)
            }
            ToolbarItem(placement: .topBarTrailing) {
                NavigationLink {
                    SettingsView(settings: settings, secureStorage: secureStorage)
                } label: {
                    Image(systemName: "gear")
                }
            }
        }
        .refreshable {
            await viewModel.refresh()
        }
        .task {
            await viewModel.refresh()
        }
    }

    // MARK: - Loading

    private var loadingSection: some View {
        VStack(spacing: 12) {
            ProgressView()
                .scaleEffect(1.2)
            Text("Connecting to server...")
                .font(AlpineTheme.Typography.caption)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 60)
    }

    // MARK: - Error

    private func errorSection(_ error: String) -> some View {
        VStack(spacing: 16) {
            Image(systemName: "exclamationmark.triangle")
                .font(.system(size: 40))
                .foregroundStyle(.orange)
            Text(error)
                .font(AlpineTheme.Typography.body)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            Button("Retry") {
                Task { await viewModel.refresh() }
            }
            .buttonStyle(.borderedProminent)
            .tint(AlpineTheme.alpineGreen)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 40)
    }

    // MARK: - Server Info

    private var serverInfoSection: some View {
        VStack(alignment: .leading, spacing: 8) {
            Label("Server", systemImage: "server.rack")
                .font(AlpineTheme.Typography.headline)
            if let status = viewModel.serverStatus {
                HStack {
                    VStack(alignment: .leading, spacing: 4) {
                        Text("Status")
                            .font(AlpineTheme.Typography.caption)
                            .foregroundStyle(.secondary)
                        Text(status.status)
                            .font(AlpineTheme.Typography.body)
                    }
                    Spacer()
                    VStack(alignment: .trailing, spacing: 4) {
                        Text("Version")
                            .font(AlpineTheme.Typography.caption)
                            .foregroundStyle(.secondary)
                        Text(status.version)
                            .font(AlpineTheme.Typography.body)
                    }
                }
            }
        }
        .padding()
        .background(AlpineTheme.cardBackground)
        .clipShape(RoundedRectangle(cornerRadius: 12))
        .shadow(color: .black.opacity(0.05), radius: 4, y: 2)
    }

    // MARK: - Stats

    private var statsSection: some View {
        HStack(spacing: 12) {
            statCard(title: "Peers", value: viewModel.peerCount, icon: "person.2.fill", color: AlpineTheme.alpineBlue)
            statCard(title: "Groups", value: viewModel.groupCount, icon: "folder.fill", color: AlpineTheme.alpineGreen)
        }
    }

    private func statCard(title: String, value: Int, icon: String, color: Color) -> some View {
        VStack(spacing: 8) {
            Image(systemName: icon)
                .font(.title2)
                .foregroundStyle(color)
            Text("\(value)")
                .font(AlpineTheme.Typography.statValue)
            Text(title)
                .font(AlpineTheme.Typography.caption)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity)
        .padding()
        .background(AlpineTheme.cardBackground)
        .clipShape(RoundedRectangle(cornerRadius: 12))
        .shadow(color: .black.opacity(0.05), radius: 4, y: 2)
    }

    // MARK: - Default Group

    private var defaultGroupSection: some View {
        Group {
            if let group = viewModel.defaultGroup {
                VStack(alignment: .leading, spacing: 8) {
                    Label("Default Group", systemImage: "star.fill")
                        .font(AlpineTheme.Typography.headline)
                        .foregroundStyle(AlpineTheme.alpineGreen)
                    Text(group.groupName)
                        .font(AlpineTheme.Typography.body)
                    HStack {
                        Label("\(group.numPeers) peers", systemImage: "person.2")
                        Spacer()
                        Label("\(group.totalQueries) queries", systemImage: "magnifyingglass")
                    }
                    .font(AlpineTheme.Typography.caption)
                    .foregroundStyle(.secondary)
                }
                .padding()
                .background(AlpineTheme.cardBackground)
                .clipShape(RoundedRectangle(cornerRadius: 12))
                .shadow(color: .black.opacity(0.05), radius: 4, y: 2)
            }
        }
    }

    // MARK: - Navigation

    private var navigationSection: some View {
        VStack(spacing: 10) {
            NavigationLink {
                SearchView(settings: settings, secureStorage: secureStorage)
            } label: {
                navigationRow(title: "Search", icon: "magnifyingglass", color: AlpineTheme.alpineBlue)
            }
            NavigationLink {
                PeersView(settings: settings, secureStorage: secureStorage)
            } label: {
                navigationRow(title: "Peers", icon: "person.2", color: AlpineTheme.alpineGreen)
            }
            NavigationLink {
                GroupsView(settings: settings, secureStorage: secureStorage)
            } label: {
                navigationRow(title: "Groups", icon: "folder", color: .orange)
            }
        }
    }

    private func navigationRow(title: String, icon: String, color: Color) -> some View {
        HStack {
            Image(systemName: icon)
                .font(.title3)
                .foregroundStyle(color)
                .frame(width: 32)
            Text(title)
                .font(AlpineTheme.Typography.headline)
                .foregroundStyle(.primary)
            Spacer()
            Image(systemName: "chevron.right")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .padding()
        .background(AlpineTheme.cardBackground)
        .clipShape(RoundedRectangle(cornerRadius: 12))
        .shadow(color: .black.opacity(0.05), radius: 4, y: 2)
    }
}
