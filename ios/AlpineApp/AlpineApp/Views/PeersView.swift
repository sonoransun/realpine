import SwiftUI

struct PeersView: View {
    @State private var viewModel: PeersViewModel
    @State private var showAddSheet = false
    @State private var showMessage = false
    @State private var newPeerIP = ""
    @State private var newPeerPort = ""

    private let settings: SettingsStore
    private let secureStorage: SecureStorage

    init(settings: SettingsStore, secureStorage: SecureStorage) {
        self.settings = settings
        self.secureStorage = secureStorage
        _viewModel = State(wrappedValue: PeersViewModel(
            settings: settings,
            secureStorage: secureStorage
        ))
    }

    var body: some View {
        Group {
            if viewModel.isLoading && viewModel.peers.isEmpty {
                loadingView
            } else if viewModel.peers.isEmpty {
                emptyView
            } else {
                peersList
            }
        }
        .navigationTitle("Peers")
        .navigationBarTitleDisplayMode(.inline)
        .toolbar {
            ToolbarItem(placement: .topBarTrailing) {
                Button {
                    showAddSheet = true
                } label: {
                    Image(systemName: "plus")
                }
            }
        }
        .sheet(isPresented: $showAddSheet) {
            addPeerSheet
        }
        .alert("Peers", isPresented: $showMessage) {
            Button("OK") { viewModel.clearMessage() }
        } message: {
            Text(viewModel.message ?? "")
        }
        .onChange(of: viewModel.message) { _, newValue in
            showMessage = newValue != nil
        }
        .task {
            await viewModel.loadPeers()
        }
    }

    // MARK: - Loading

    private var loadingView: some View {
        VStack(spacing: 12) {
            ProgressView()
                .scaleEffect(1.2)
            Text("Loading peers...")
                .font(AlpineTheme.Typography.caption)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: - Empty

    private var emptyView: some View {
        VStack(spacing: 12) {
            Image(systemName: "person.2.slash")
                .font(.system(size: 36))
                .foregroundStyle(.secondary)
            Text("No peers found")
                .font(AlpineTheme.Typography.headline)
            Text("Add a peer to get started")
                .font(AlpineTheme.Typography.caption)
                .foregroundStyle(.secondary)
            Button("Add Peer") {
                showAddSheet = true
            }
            .buttonStyle(.borderedProminent)
            .tint(AlpineTheme.alpineGreen)
            .padding(.top, 8)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: - Peers List

    private var peersList: some View {
        List {
            ForEach(viewModel.peers) { peer in
                peerRow(peer)
                    .swipeActions(edge: .leading) {
                        Button {
                            Task { await viewModel.pingPeer(peerId: peer.peerId) }
                        } label: {
                            Label("Ping", systemImage: "antenna.radiowaves.left.and.right")
                        }
                        .tint(AlpineTheme.alpineBlue)
                    }
                    .swipeActions(edge: .trailing) {
                        if peer.lastRecvTime > 0 {
                            Button {
                                Task { await viewModel.deactivatePeer(peerId: peer.peerId) }
                            } label: {
                                Label("Deactivate", systemImage: "pause.circle")
                            }
                            .tint(.orange)
                        } else {
                            Button {
                                Task { await viewModel.activatePeer(peerId: peer.peerId) }
                            } label: {
                                Label("Activate", systemImage: "play.circle")
                            }
                            .tint(.green)
                        }
                    }
            }
        }
        .listStyle(.insetGrouped)
        .refreshable {
            await viewModel.loadPeers()
        }
    }

    private func peerRow(_ peer: PeerDetail) -> some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack {
                Text("\(peer.ipAddress):\(peer.port)")
                    .font(AlpineTheme.Typography.headline)
                Spacer()
                statusBadge(for: peer)
            }
            HStack(spacing: 16) {
                Label(
                    formatBandwidth(peer.avgBandwidth),
                    systemImage: "arrow.up.arrow.down"
                )
                Label(
                    formatBandwidth(peer.peakBandwidth),
                    systemImage: "chart.line.uptrend.xyaxis"
                )
            }
            .font(AlpineTheme.Typography.caption)
            .foregroundStyle(.secondary)
        }
        .padding(.vertical, 4)
    }

    private func statusBadge(for peer: PeerDetail) -> some View {
        let isActive = peer.lastRecvTime > 0
        return Text(isActive ? "Active" : "Inactive")
            .font(.caption2.weight(.medium))
            .padding(.horizontal, 8)
            .padding(.vertical, 3)
            .background(isActive ? Color.green.opacity(0.15) : Color.gray.opacity(0.15))
            .foregroundStyle(isActive ? .green : .secondary)
            .clipShape(Capsule())
    }

    private func formatBandwidth(_ bytes: Int64) -> String {
        ByteCountFormatter.string(fromByteCount: bytes, countStyle: .binary) + "/s"
    }

    // MARK: - Add Peer Sheet

    private var addPeerSheet: some View {
        NavigationStack {
            Form {
                Section("New Peer") {
                    TextField("IP Address", text: $newPeerIP)
                        .autocorrectionDisabled()
                        .textInputAutocapitalization(.never)
                        .keyboardType(.decimalPad)
                    TextField("Port", text: $newPeerPort)
                        .keyboardType(.numberPad)
                }
                Section {
                    Button {
                        guard let port = Int64(newPeerPort), port > 0 else { return }
                        Task {
                            await viewModel.addPeer(ipAddress: newPeerIP, port: port)
                            newPeerIP = ""
                            newPeerPort = ""
                            showAddSheet = false
                        }
                    } label: {
                        Text("Add Peer")
                            .frame(maxWidth: .infinity)
                            .font(AlpineTheme.Typography.headline)
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(AlpineTheme.alpineGreen)
                    .disabled(newPeerIP.isEmpty || newPeerPort.isEmpty)
                    .listRowBackground(Color.clear)
                    .listRowInsets(EdgeInsets())
                }
            }
            .navigationTitle("Add Peer")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { showAddSheet = false }
                }
            }
        }
        .presentationDetents([.medium])
    }
}
