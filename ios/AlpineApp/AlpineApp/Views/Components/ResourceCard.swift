import SwiftUI

struct ResourceCard: View {
    let resource: ResourceDesc
    @State private var isExpanded = false

    var body: some View {
        DisclosureGroup(isExpanded: $isExpanded) {
            VStack(alignment: .leading, spacing: 4) {
                Text("Size: \(ByteCountFormatter.string(fromByteCount: resource.size, countStyle: .file))")
                    .font(AlpineTheme.Typography.caption)
                ForEach(resource.locators, id: \.self) { locator in
                    if let url = URL(string: locator) {
                        Link(locator, destination: url)
                            .font(AlpineTheme.Typography.caption)
                            .lineLimit(1)
                            .truncationMode(.middle)
                    } else {
                        Text(locator)
                            .font(AlpineTheme.Typography.caption)
                    }
                }
            }
        } label: {
            HStack {
                Image(systemName: "doc")
                    .foregroundStyle(AlpineTheme.alpineBlue)
                Text(resource.description)
                    .font(AlpineTheme.Typography.body)
                    .lineLimit(1)
            }
        }
    }
}
