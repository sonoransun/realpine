import SwiftUI

struct TransportModeIndicator: View {
    let mode: TransportMode

    var body: some View {
        Label {
            Text(mode == .restBridge ? "REST Bridge" : "WiFi Broadcast")
                .font(AlpineTheme.Typography.caption)
        } icon: {
            Image(systemName: mode == .restBridge ? "server.rack" : "wifi")
                .foregroundStyle(
                    mode == .restBridge ? AlpineTheme.alpineBlue : AlpineTheme.alpineGreen
                )
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 4)
        .background(AlpineTheme.secondaryBackground)
        .clipShape(Capsule())
    }
}
