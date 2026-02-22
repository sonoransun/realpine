import SwiftUI

struct SearchView: View {
    @State private var viewModel: SearchViewModel
    @State private var navigateToResults = false
    @State private var activeQueryId: Int64?
    @State private var showError = false

    private let settings: SettingsStore
    private let secureStorage: SecureStorage

    init(settings: SettingsStore, secureStorage: SecureStorage) {
        self.settings = settings
        self.secureStorage = secureStorage
        _viewModel = State(wrappedValue: SearchViewModel(
            settings: settings,
            secureStorage: secureStorage
        ))
    }

    var body: some View {
        VStack(spacing: 20) {
            TransportModeIndicator(mode: viewModel.transportMode)
                .frame(maxWidth: .infinity, alignment: .trailing)
                .padding(.horizontal)

            searchField

            advancedOptions

            searchButton

            Spacer()
        }
        .padding(.top)
        .navigationTitle("Search")
        .navigationBarTitleDisplayMode(.inline)
        .navigationDestination(isPresented: $navigateToResults) {
            if let queryId = activeQueryId {
                ResultsView(
                    queryId: queryId,
                    settings: settings,
                    secureStorage: secureStorage
                )
            }
        }
        .alert("Search Error", isPresented: $showError) {
            Button("OK") { viewModel.clearError() }
        } message: {
            Text(viewModel.error ?? "An unknown error occurred")
        }
        .onChange(of: viewModel.error) { _, newValue in
            showError = newValue != nil
        }
    }

    // MARK: - Search Field

    private var searchField: some View {
        HStack {
            Image(systemName: "magnifyingglass")
                .foregroundStyle(.secondary)
            TextField("Search query...", text: $viewModel.queryString)
                .textFieldStyle(.plain)
                .autocorrectionDisabled()
                .submitLabel(.search)
                .onSubmit {
                    performSearch()
                }
        }
        .padding()
        .background(AlpineTheme.secondaryBackground)
        .clipShape(RoundedRectangle(cornerRadius: 12))
        .padding(.horizontal)
    }

    // MARK: - Advanced Options

    private var advancedOptions: some View {
        DisclosureGroup("Advanced Options") {
            VStack(spacing: 12) {
                HStack {
                    Text("Group Name")
                        .font(AlpineTheme.Typography.caption)
                        .frame(width: 100, alignment: .leading)
                    TextField("Default", text: $viewModel.groupName)
                        .textFieldStyle(.roundedBorder)
                        .autocorrectionDisabled()
                        .textInputAutocapitalization(.never)
                }
                HStack {
                    Text("Auto-halt Limit")
                        .font(AlpineTheme.Typography.caption)
                        .frame(width: 100, alignment: .leading)
                    TextField("100", text: $viewModel.autoHaltLimit)
                        .textFieldStyle(.roundedBorder)
                        .keyboardType(.numberPad)
                }
                HStack {
                    Text("Peer Desc Max")
                        .font(AlpineTheme.Typography.caption)
                        .frame(width: 100, alignment: .leading)
                    TextField("50", text: $viewModel.peerDescMax)
                        .textFieldStyle(.roundedBorder)
                        .keyboardType(.numberPad)
                }
            }
            .padding(.top, 8)
        }
        .padding(.horizontal)
    }

    // MARK: - Search Button

    private var searchButton: some View {
        Button {
            performSearch()
        } label: {
            HStack {
                if viewModel.isLoading {
                    ProgressView()
                        .tint(.white)
                        .padding(.trailing, 4)
                }
                Text(viewModel.isLoading ? "Searching..." : "Search")
                    .font(AlpineTheme.Typography.headline)
            }
            .frame(maxWidth: .infinity)
            .padding()
            .background(
                viewModel.queryString.trimmingCharacters(in: .whitespaces).isEmpty || viewModel.isLoading
                    ? Color.gray
                    : AlpineTheme.alpineGreen
            )
            .foregroundStyle(.white)
            .clipShape(RoundedRectangle(cornerRadius: 12))
        }
        .disabled(viewModel.queryString.trimmingCharacters(in: .whitespaces).isEmpty || viewModel.isLoading)
        .padding(.horizontal)
    }

    // MARK: - Actions

    private func performSearch() {
        Task {
            if let queryId = await viewModel.search() {
                activeQueryId = queryId
                navigateToResults = true
            }
        }
    }
}
