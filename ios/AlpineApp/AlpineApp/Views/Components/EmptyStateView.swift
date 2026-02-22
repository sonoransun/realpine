import SwiftUI

/// A centered placeholder view for empty content states with an optional call-to-action button.
struct EmptyStateView: View {
    let systemImage: String
    let title: String
    let description: String
    var actionLabel: String?
    var action: (() -> Void)?

    var body: some View {
        VStack(spacing: 16) {
            Image(systemName: systemImage)
                .font(.system(size: 56))
                .foregroundStyle(.secondary)

            Text(title)
                .font(AlpineTheme.Typography.headline)

            Text(description)
                .font(.subheadline)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 32)

            if let actionLabel, let action {
                Button(action: action) {
                    Text(actionLabel)
                }
                .buttonStyle(.borderedProminent)
                .padding(.top, 8)
            }
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }
}
