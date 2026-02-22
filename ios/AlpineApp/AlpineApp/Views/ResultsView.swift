import SwiftUI

struct ResultsView: View {
    @State private var viewModel: ResultsViewModel

    init(queryId: Int64, settings: SettingsStore, secureStorage: SecureStorage) {
        _viewModel = State(wrappedValue: ResultsViewModel(
            queryId: queryId,
            settings: settings,
            secureStorage: secureStorage
        ))
    }

    var body: some View {
        VStack(spacing: 0) {
            QueryStatusBar(status: viewModel.queryStatus)
                .padding()

            if viewModel.isLoading && viewModel.results.isEmpty {
                loadingView
            } else if let error = viewModel.error {
                errorView(error)
            } else if viewModel.results.isEmpty {
                emptyView
            } else {
                resultsList
            }
        }
        .navigationTitle("Results")
        .navigationBarTitleDisplayMode(.inline)
        .toolbar {
            ToolbarItem(placement: .topBarTrailing) {
                if viewModel.queryStatus?.inProgress == true {
                    Button("Cancel") {
                        viewModel.cancelQuery()
                    }
                    .foregroundStyle(.red)
                }
            }
        }
        .onDisappear {
            viewModel.stopPolling()
        }
    }

    // MARK: - Loading

    private var loadingView: some View {
        VStack(spacing: 16) {
            ProgressView()
                .scaleEffect(1.2)
            Text("Querying peers...")
                .font(AlpineTheme.Typography.caption)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: - Error

    private func errorView(_ error: String) -> some View {
        VStack(spacing: 12) {
            Image(systemName: "exclamationmark.triangle")
                .font(.system(size: 36))
                .foregroundStyle(.orange)
            Text(error)
                .font(AlpineTheme.Typography.body)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
        }
        .padding()
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: - Empty

    private var emptyView: some View {
        VStack(spacing: 12) {
            Image(systemName: "magnifyingglass")
                .font(.system(size: 36))
                .foregroundStyle(.secondary)
            Text("No results found")
                .font(AlpineTheme.Typography.headline)
            if let status = viewModel.queryStatus {
                Text("Queried \(status.peersQueried) of \(status.totalPeers) peers")
                    .font(AlpineTheme.Typography.caption)
                    .foregroundStyle(.secondary)
            }
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: - Results List

    private var resultsList: some View {
        List {
            ForEach(viewModel.results) { peerResult in
                Section {
                    ForEach(peerResult.resources) { resource in
                        ResourceCard(resource: resource)
                    }
                } header: {
                    HStack {
                        Image(systemName: "desktopcomputer")
                            .foregroundStyle(AlpineTheme.alpineBlue)
                        Text("Peer \(peerResult.peerId)")
                            .font(AlpineTheme.Typography.caption)
                        Spacer()
                        Text("\(peerResult.resources.count) resources")
                            .font(AlpineTheme.Typography.caption)
                            .foregroundStyle(.secondary)
                    }
                }
            }
        }
        .listStyle(.insetGrouped)
    }
}
