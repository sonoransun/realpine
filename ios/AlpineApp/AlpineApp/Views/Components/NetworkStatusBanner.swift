import SwiftUI

/// A compact banner that slides in from the top when the network connection is lost.
struct NetworkStatusBanner: View {
    let isConnected: Bool

    var body: some View {
        if !isConnected {
            HStack(spacing: 8) {
                Image(systemName: "wifi.slash")
                    .font(.subheadline)
                Text("No Network Connection")
                    .font(.subheadline.weight(.medium))
            }
            .foregroundStyle(.white)
            .padding(.vertical, 8)
            .frame(maxWidth: .infinity)
            .background(Color.red.opacity(0.85))
            .transition(.move(edge: .top).combined(with: .opacity))
        }
    }
}
