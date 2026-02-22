import SwiftUI

struct GroupsView: View {
    @State private var viewModel: GroupsViewModel
    @State private var showCreateSheet = false
    @State private var showMessage = false
    @State private var newGroupName = ""
    @State private var newGroupDescription = ""

    private let settings: SettingsStore
    private let secureStorage: SecureStorage

    init(settings: SettingsStore, secureStorage: SecureStorage) {
        self.settings = settings
        self.secureStorage = secureStorage
        _viewModel = State(wrappedValue: GroupsViewModel(
            settings: settings,
            secureStorage: secureStorage
        ))
    }

    var body: some View {
        Group {
            if viewModel.isLoading && viewModel.groups.isEmpty {
                loadingView
            } else if viewModel.groups.isEmpty {
                emptyView
            } else {
                groupsList
            }
        }
        .navigationTitle("Groups")
        .navigationBarTitleDisplayMode(.inline)
        .toolbar {
            ToolbarItem(placement: .topBarTrailing) {
                Button {
                    showCreateSheet = true
                } label: {
                    Image(systemName: "plus")
                }
            }
        }
        .sheet(isPresented: $showCreateSheet) {
            createGroupSheet
        }
        .alert("Groups", isPresented: $showMessage) {
            Button("OK") { viewModel.clearMessage() }
        } message: {
            Text(viewModel.message ?? "")
        }
        .onChange(of: viewModel.message) { _, newValue in
            showMessage = newValue != nil
        }
        .task {
            await viewModel.loadGroups()
        }
    }

    // MARK: - Loading

    private var loadingView: some View {
        VStack(spacing: 12) {
            ProgressView()
                .scaleEffect(1.2)
            Text("Loading groups...")
                .font(AlpineTheme.Typography.caption)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: - Empty

    private var emptyView: some View {
        VStack(spacing: 12) {
            Image(systemName: "folder.badge.questionmark")
                .font(.system(size: 36))
                .foregroundStyle(.secondary)
            Text("No groups found")
                .font(AlpineTheme.Typography.headline)
            Text("Create a group to organize peers")
                .font(AlpineTheme.Typography.caption)
                .foregroundStyle(.secondary)
            Button("Create Group") {
                showCreateSheet = true
            }
            .buttonStyle(.borderedProminent)
            .tint(AlpineTheme.alpineGreen)
            .padding(.top, 8)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: - Groups List

    private var groupsList: some View {
        List {
            ForEach(viewModel.groups) { group in
                groupRow(group)
                    .swipeActions(edge: .trailing, allowsFullSwipe: true) {
                        Button(role: .destructive) {
                            Task { await viewModel.deleteGroup(groupId: group.groupId) }
                        } label: {
                            Label("Delete", systemImage: "trash")
                        }
                    }
            }
        }
        .listStyle(.insetGrouped)
        .refreshable {
            await viewModel.loadGroups()
        }
    }

    private func groupRow(_ group: GroupInfo) -> some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Image(systemName: "folder.fill")
                    .foregroundStyle(AlpineTheme.alpineGreen)
                Text(group.groupName)
                    .font(AlpineTheme.Typography.headline)
            }
            if !group.description.isEmpty {
                Text(group.description)
                    .font(AlpineTheme.Typography.body)
                    .foregroundStyle(.secondary)
                    .lineLimit(2)
            }
            HStack(spacing: 16) {
                Label("\(group.numPeers) peers", systemImage: "person.2")
                Label("\(group.totalQueries) queries", systemImage: "magnifyingglass")
                Label("\(group.totalResponses) responses", systemImage: "arrow.down.circle")
            }
            .font(AlpineTheme.Typography.caption)
            .foregroundStyle(.secondary)
        }
        .padding(.vertical, 4)
    }

    // MARK: - Create Group Sheet

    private var createGroupSheet: some View {
        NavigationStack {
            Form {
                Section("New Group") {
                    TextField("Group Name", text: $newGroupName)
                        .autocorrectionDisabled()
                    TextField("Description", text: $newGroupDescription, axis: .vertical)
                        .lineLimit(3...6)
                }
                Section {
                    Button {
                        guard !newGroupName.isEmpty else { return }
                        Task {
                            await viewModel.createGroup(
                                name: newGroupName,
                                description: newGroupDescription
                            )
                            newGroupName = ""
                            newGroupDescription = ""
                            showCreateSheet = false
                        }
                    } label: {
                        Text("Create Group")
                            .frame(maxWidth: .infinity)
                            .font(AlpineTheme.Typography.headline)
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(AlpineTheme.alpineGreen)
                    .disabled(newGroupName.isEmpty)
                    .listRowBackground(Color.clear)
                    .listRowInsets(EdgeInsets())
                }
            }
            .navigationTitle("Create Group")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { showCreateSheet = false }
                }
            }
        }
        .presentationDetents([.medium])
    }
}
