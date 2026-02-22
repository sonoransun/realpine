import SwiftUI

struct SearchView: View {
    @State private var viewModel: SearchViewModel
    @State private var navigateToResults = false
    @State private var activeQueryId: Int64?
    @State private var showError = false
    @Environment(SearchHistoryStore.self) private var searchHistory

    private let settings: SettingsStore
    private let secureStorage: SecureStorage

    init(settings: SettingsStore, secureStorage: SecureStorage) {
        self.settings = settings
        self.secureStorage = secureStorage
        // SearchHistoryStore will be assigned from environment in onAppear
        _viewModel = State(wrappedValue: SearchViewModel(
            settings: settings,
            secureStorage: secureStorage,
            searchHistory: SearchHistoryStore()
        ))
    }

    var body: some View {
        VStack(spacing: 20) {
            TransportModeIndicator(mode: viewModel.transportMode)
                .frame(maxWidth: .infinity, alignment: .trailing)
                .padding(.horizontal)

            modePicker

            if viewModel.searchMode != .sparql {
                searchField
            }

            recentSearchesSection

            sparqlEditor
            entityFilterChips
            similaritySlider
            multiModalOptions
            queryPreview

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
        .onAppear {
            viewModel.searchHistory = searchHistory
        }
    }

    // MARK: - Search Mode Picker

    private var modePicker: some View {
        Picker("Search Mode", selection: $viewModel.searchMode) {
            Text("Keyword").tag(SearchMode.keyword)
            Text("Semantic").tag(SearchMode.semantic)
            Text("Entity").tag(SearchMode.entity)
            Text("SPARQL").tag(SearchMode.sparql)
            Text("Multi-Modal").tag(SearchMode.multiModal)
        }
        .pickerStyle(.segmented)
        .padding(.horizontal)
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
                .accessibilityLabel("Search query")
                .accessibilityHint("Enter your search terms")
        }
        .padding()
        .background(AlpineTheme.secondaryBackground)
        .clipShape(RoundedRectangle(cornerRadius: 12))
        .padding(.horizontal)
    }

    // MARK: - Recent Searches

    @ViewBuilder
    private var recentSearchesSection: some View {
        let queryEmpty = viewModel.queryString.trimmingCharacters(in: .whitespaces).isEmpty
        if queryEmpty && !searchHistory.entries.isEmpty && viewModel.searchMode != .sparql {
            VStack(alignment: .leading, spacing: 8) {
                HStack {
                    Text("Recent Searches")
                        .font(AlpineTheme.Typography.caption)
                        .foregroundStyle(.secondary)
                    Spacer()
                    Button("Clear History") {
                        searchHistory.clear()
                    }
                    .font(.caption)
                    .foregroundStyle(AlpineTheme.destructive)
                    .accessibilityLabel("Clear search history")
                    .accessibilityHint("Removes all recent search entries")
                }

                ForEach(searchHistory.entries.prefix(10)) { entry in
                    Button {
                        viewModel.queryString = entry.query
                        if let mode = SearchMode(rawValue: entry.mode) {
                            viewModel.searchMode = mode
                        }
                    } label: {
                        HStack {
                            Image(systemName: "clock.arrow.circlepath")
                                .font(.caption)
                                .foregroundStyle(.secondary)
                            VStack(alignment: .leading, spacing: 2) {
                                Text(entry.query)
                                    .font(AlpineTheme.Typography.body)
                                    .foregroundStyle(AlpineTheme.textPrimary)
                                    .lineLimit(1)
                                Text(entry.mode.capitalized)
                                    .font(.caption2)
                                    .foregroundStyle(.secondary)
                            }
                            Spacer()
                            if let count = entry.resultCount {
                                Text("\(count)")
                                    .font(.caption2)
                                    .foregroundStyle(.secondary)
                            }
                        }
                        .padding(.vertical, 4)
                    }
                    .accessibilityLabel("Recent search: \(entry.query)")
                    .accessibilityHint("Tap to search for \(entry.query) using \(entry.mode) mode")
                }
            }
            .padding(.horizontal)
        }
    }

    // MARK: - SPARQL Editor

    @ViewBuilder
    private var sparqlEditor: some View {
        if viewModel.searchMode == .sparql {
            VStack(alignment: .leading, spacing: 4) {
                Text("Triple Patterns")
                    .font(AlpineTheme.Typography.caption)
                    .foregroundStyle(.secondary)
                TextEditor(text: $viewModel.sparqlQuery)
                    .font(.system(.body, design: .monospaced))
                    .frame(minHeight: 80, maxHeight: 160)
                    .scrollContentBackground(.hidden)
                    .padding(8)
                    .background(AlpineTheme.secondaryBackground)
                    .clipShape(RoundedRectangle(cornerRadius: 8))
                Text("Example: ?file alpine:hasKeyword report")
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
            }
            .padding(.horizontal)
        }
    }

    // MARK: - Entity Filter Chips

    @ViewBuilder
    private var entityFilterChips: some View {
        if viewModel.searchMode == .entity {
            VStack(alignment: .leading, spacing: 8) {
                Text("Filter by Entity Type")
                    .font(AlpineTheme.Typography.caption)
                    .foregroundStyle(.secondary)
                FlowLayout(spacing: 8) {
                    ForEach(EntityType.allCases, id: \.self) { type in
                        let isSelected = viewModel.entityFilters.contains(type)
                        Button {
                            if isSelected {
                                viewModel.entityFilters.remove(type)
                            } else {
                                viewModel.entityFilters.insert(type)
                            }
                        } label: {
                            Text(entityTypeLabel(type))
                                .font(AlpineTheme.Typography.caption)
                                .padding(.horizontal, 12)
                                .padding(.vertical, 6)
                                .background(isSelected ? entityTypeColor(type) : Color.gray.opacity(0.2))
                                .foregroundStyle(isSelected ? .white : .primary)
                                .clipShape(Capsule())
                        }
                    }
                }
            }
            .padding(.horizontal)
        }
    }

    // MARK: - Similarity Slider

    @ViewBuilder
    private var similaritySlider: some View {
        if viewModel.searchMode == .semantic {
            VStack(alignment: .leading, spacing: 4) {
                HStack {
                    Text("Similarity Threshold")
                        .font(AlpineTheme.Typography.caption)
                    Spacer()
                    Text("\(Int(viewModel.similarityThreshold * 100))%")
                        .font(AlpineTheme.Typography.caption)
                        .foregroundStyle(.secondary)
                }
                Slider(value: $viewModel.similarityThreshold, in: 0.1...1.0, step: 0.05)
                    .tint(AlpineTheme.alpineGreen)
            }
            .padding(.horizontal)
        }
    }

    // MARK: - Multi-Modal Options

    @ViewBuilder
    private var multiModalOptions: some View {
        if viewModel.searchMode == .multiModal {
            VStack(alignment: .leading, spacing: 12) {
                // Similarity threshold (reuse the slider)
                VStack(alignment: .leading, spacing: 4) {
                    HStack {
                        Text("Similarity Threshold")
                            .font(AlpineTheme.Typography.caption)
                        Spacer()
                        Text("\(Int(viewModel.similarityThreshold * 100))%")
                            .font(AlpineTheme.Typography.caption)
                            .foregroundStyle(.secondary)
                    }
                    Slider(value: $viewModel.similarityThreshold, in: 0.1...1.0, step: 0.05)
                        .tint(AlpineTheme.alpineGreen)
                }

                // Content category chips
                VStack(alignment: .leading, spacing: 8) {
                    Text("Content Categories")
                        .font(AlpineTheme.Typography.caption)
                        .foregroundStyle(.secondary)
                    FlowLayout(spacing: 8) {
                        ForEach(ContentCategory.allCases, id: \.self) { category in
                            let isSelected = viewModel.contentCategoryFilters.contains(category)
                            Button {
                                if isSelected {
                                    viewModel.contentCategoryFilters.remove(category)
                                } else {
                                    viewModel.contentCategoryFilters.insert(category)
                                }
                            } label: {
                                HStack(spacing: 4) {
                                    Image(systemName: categoryIcon(category))
                                        .font(.caption2)
                                    Text(category.rawValue.capitalized)
                                }
                                .font(AlpineTheme.Typography.caption)
                                .padding(.horizontal, 12)
                                .padding(.vertical, 6)
                                .background(isSelected ? categoryColor(category) : Color.gray.opacity(0.2))
                                .foregroundStyle(isSelected ? .white : .primary)
                                .clipShape(Capsule())
                            }
                        }
                    }
                }

                // Language chips
                VStack(alignment: .leading, spacing: 8) {
                    Text("Language Filter")
                        .font(AlpineTheme.Typography.caption)
                        .foregroundStyle(.secondary)
                    FlowLayout(spacing: 8) {
                        ForEach(["en", "es", "fr", "de", "ja", "zh"], id: \.self) { code in
                            let isSelected = viewModel.languageFilters.contains(code)
                            let displayName = Locale.current.localizedString(forLanguageCode: code) ?? code
                            Button {
                                if isSelected {
                                    viewModel.languageFilters.remove(code)
                                } else {
                                    viewModel.languageFilters.insert(code)
                                }
                            } label: {
                                Text(displayName)
                                    .font(AlpineTheme.Typography.caption)
                                    .padding(.horizontal, 12)
                                    .padding(.vertical, 6)
                                    .background(isSelected ? AlpineTheme.alpineBlue : Color.gray.opacity(0.2))
                                    .foregroundStyle(isSelected ? .white : .primary)
                                    .clipShape(Capsule())
                            }
                        }
                    }
                }

                // Signal weights
                DisclosureGroup("Signal Weights") {
                    VStack(spacing: 8) {
                        weightSlider(label: "Text", value: $viewModel.signalWeights.text)
                        weightSlider(label: "Entity", value: $viewModel.signalWeights.entity)
                        weightSlider(label: "Category", value: $viewModel.signalWeights.category)
                        weightSlider(label: "Language", value: $viewModel.signalWeights.language)
                    }
                    .padding(.top, 4)
                }
                .font(AlpineTheme.Typography.caption)
            }
            .padding(.horizontal)
        }
    }

    private func weightSlider(label: String, value: Binding<Double>) -> some View {
        HStack {
            Text(label)
                .font(AlpineTheme.Typography.caption)
                .frame(width: 70, alignment: .leading)
            Slider(value: value, in: 0.0...2.0, step: 0.1)
                .tint(AlpineTheme.alpineBlue)
            Text(String(format: "%.1f", value.wrappedValue))
                .font(.caption2)
                .foregroundStyle(.secondary)
                .frame(width: 30)
        }
    }

    // MARK: - Query Preview

    private var queryPreview: some View {
        Text(viewModel.queryPreview)
            .font(.caption2)
            .foregroundStyle(.secondary)
            .frame(maxWidth: .infinity, alignment: .leading)
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

    private var isSearchDisabled: Bool {
        if viewModel.isLoading { return true }
        switch viewModel.searchMode {
        case .sparql:
            return viewModel.sparqlQuery.trimmingCharacters(in: .whitespaces).isEmpty
        default:
            return viewModel.queryString.trimmingCharacters(in: .whitespaces).isEmpty
        }
    }

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
            .background(isSearchDisabled ? Color.gray : AlpineTheme.alpineGreen)
            .foregroundStyle(.white)
            .clipShape(RoundedRectangle(cornerRadius: 12))
        }
        .disabled(isSearchDisabled)
        .padding(.horizontal)
        .accessibilityLabel(viewModel.isLoading ? "Searching" : "Search")
        .accessibilityHint("Submits the search query")
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

    // MARK: - Entity Helpers

    private func entityTypeLabel(_ type: EntityType) -> String {
        switch type {
        case .personalName: return "Person"
        case .placeName: return "Place"
        case .organizationName: return "Organization"
        case .date: return "Date"
        case .fileType: return "File Type"
        }
    }

    private func entityTypeColor(_ type: EntityType) -> Color {
        switch type {
        case .personalName: return .blue
        case .placeName: return .green
        case .organizationName: return .purple
        case .date: return .orange
        case .fileType: return .teal
        }
    }

    // MARK: - Category Helpers

    private func categoryIcon(_ category: ContentCategory) -> String {
        switch category {
        case .document: return "doc.text"
        case .image: return "photo"
        case .audio: return "waveform"
        case .video: return "film"
        case .archive: return "archivebox"
        case .code: return "chevron.left.forwardslash.chevron.right"
        case .data: return "tablecells"
        case .other: return "questionmark.folder"
        }
    }

    private func categoryColor(_ category: ContentCategory) -> Color {
        switch category {
        case .document: return .blue
        case .image: return .green
        case .audio: return .purple
        case .video: return .red
        case .archive: return .orange
        case .code: return .teal
        case .data: return .indigo
        case .other: return .gray
        }
    }
}
