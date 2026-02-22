import SwiftUI

struct MediaInfoBadge: View {
    let metadata: MediaMetadata

    var body: some View {
        HStack(spacing: 6) {
            if let w = metadata.width, let h = metadata.height {
                Label("\(w)\u{00D7}\(h)", systemImage: "rectangle.arrowtriangle.2.outward")
            }
            if let duration = metadata.durationSeconds {
                Label(formatDuration(duration), systemImage: "clock")
            }
            if let codec = metadata.codec {
                Text(codec)
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
        }
        .font(.caption2)
        .foregroundStyle(.secondary)
    }

    private func formatDuration(_ seconds: Double) -> String {
        let totalSeconds = Int(seconds)
        let hours = totalSeconds / 3600
        let minutes = (totalSeconds % 3600) / 60
        let secs = totalSeconds % 60
        if hours > 0 {
            return String(format: "%d:%02d:%02d", hours, minutes, secs)
        }
        return String(format: "%d:%02d", minutes, secs)
    }
}
