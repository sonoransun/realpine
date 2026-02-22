import SwiftUI

struct DiscoveredBridgeCard: View {
    let beacon: BridgeBeacon
    let onSelect: () -> Void

    var body: some View {
        Button(action: onSelect) {
            VStack(alignment: .leading, spacing: 4) {
                HStack {
                    Image(systemName: "network")
                        .foregroundStyle(AlpineTheme.alpineBlue)
                    Text(beacon.hostAddress)
                        .font(AlpineTheme.Typography.headline)
                }
                HStack {
                    Text("Port: \(beacon.restPort)")
                    Spacer()
                    Text("v\(beacon.bridgeVersion)")
                }
                .font(AlpineTheme.Typography.caption)
                .foregroundStyle(.secondary)
            }
            .padding()
            .background(AlpineTheme.secondaryBackground)
            .clipShape(RoundedRectangle(cornerRadius: 10))
        }
        .buttonStyle(.plain)
    }
}
