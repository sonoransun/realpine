import SwiftUI

struct ConnectionIndicator: View {
    let isConnected: Bool
    var showLabel: Bool = false

    var body: some View {
        HStack(spacing: 6) {
            Circle()
                .fill(isConnected ? Color.green : Color.red)
                .frame(width: 10, height: 10)
            if showLabel {
                Text(isConnected ? "Connected" : "Disconnected")
                    .font(AlpineTheme.Typography.caption)
                    .foregroundStyle(.secondary)
            }
        }
    }
}
