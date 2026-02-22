import SwiftUI

struct QueryStatusBar: View {
    let status: QueryStatusResponse?

    var body: some View {
        if let status {
            VStack(spacing: 8) {
                if status.inProgress {
                    ProgressView()
                        .progressViewStyle(.linear)
                }
                HStack {
                    Label("\(status.peersQueried)/\(status.totalPeers)", systemImage: "person.2")
                    Spacer()
                    Label("\(status.numPeerResponses)", systemImage: "arrow.down.circle")
                    Spacer()
                    Label("\(status.totalHits)", systemImage: "doc.text.magnifyingglass")
                }
                .font(AlpineTheme.Typography.caption)
            }
            .padding()
            .background(AlpineTheme.secondaryBackground)
            .clipShape(RoundedRectangle(cornerRadius: 10))
        }
    }
}
