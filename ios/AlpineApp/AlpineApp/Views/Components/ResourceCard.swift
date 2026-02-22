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

                // Entity chips
                if let entities = resource.matchedEntities, !entities.isEmpty {
                    VStack(alignment: .leading, spacing: 4) {
                        Text("Entities")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        FlowLayout(spacing: 4) {
                            ForEach(entities) { entity in
                                entityChip(entity)
                            }
                        }
                    }
                    .padding(.top, 4)
                }

                // Matched keywords
                if let keywords = resource.matchedKeywords, !keywords.isEmpty {
                    HStack {
                        Text("Keywords:")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        Text(keywords.joined(separator: ", "))
                            .font(.caption2)
                    }
                    .padding(.top, 2)
                }

                // Content category badge
                if let category = resource.contentCategory {
                    HStack(spacing: 4) {
                        Image(systemName: categoryIcon(category))
                        Text(category.rawValue.capitalized)
                    }
                    .font(.caption2)
                    .padding(.horizontal, 8)
                    .padding(.vertical, 2)
                    .background(categoryColor(category).opacity(0.15))
                    .clipShape(Capsule())
                }

                // Language badge
                if let lang = resource.language {
                    Text(Locale.current.localizedString(forLanguageCode: lang.languageCode) ?? lang.languageCode)
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }

                // Media info
                if let media = resource.mediaMetadata {
                    MediaInfoBadge(metadata: media)
                }

                // RDF triples
                if let triples = resource.rdfTriples, !triples.isEmpty {
                    DisclosureGroup("RDF Triples (\(triples.count))") {
                        VStack(alignment: .leading, spacing: 2) {
                            ForEach(triples) { triple in
                                Text(tripleDescription(triple))
                                    .font(.system(.caption2, design: .monospaced))
                            }
                        }
                    }
                    .font(AlpineTheme.Typography.caption)
                    .padding(.top, 4)
                }
            }
        } label: {
            HStack {
                Image(systemName: "doc")
                    .foregroundStyle(AlpineTheme.alpineBlue)
                Text(resource.description)
                    .font(AlpineTheme.Typography.body)
                    .lineLimit(1)
                Spacer()
                if let score = resource.score {
                    scoreBadge(score)
                }
            }
        }
    }

    // MARK: - Score Badge

    private func scoreBadge(_ score: Float) -> some View {
        Text("\(Int(score * 100))%")
            .font(.caption2.bold())
            .padding(.horizontal, 8)
            .padding(.vertical, 2)
            .background(scoreColor(score))
            .foregroundStyle(.white)
            .clipShape(Capsule())
    }

    private func scoreColor(_ score: Float) -> Color {
        if score > 0.8 { return .green }
        if score > 0.5 { return .orange }
        return .red
    }

    // MARK: - Entity Chip

    private func entityChip(_ entity: EntityAnnotation) -> some View {
        HStack(spacing: 4) {
            Image(systemName: entityIcon(entity.entityType))
                .font(.caption2)
            Text(entity.text)
                .font(.caption2)
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 4)
        .background(entityColor(entity.entityType).opacity(0.2))
        .foregroundStyle(entityColor(entity.entityType))
        .clipShape(Capsule())
    }

    private func entityIcon(_ type: EntityType) -> String {
        switch type {
        case .personalName: return "person.fill"
        case .placeName: return "mappin"
        case .organizationName: return "building.2.fill"
        case .date: return "calendar"
        case .fileType: return "doc.fill"
        }
    }

    private func entityColor(_ type: EntityType) -> Color {
        switch type {
        case .personalName: return .blue
        case .placeName: return .green
        case .organizationName: return .purple
        case .date: return .orange
        case .fileType: return .teal
        }
    }

    // MARK: - Category Helpers

    private func categoryIcon(_ category: ContentCategory) -> String {
        switch category {
        case .document: return "doc.text"
        case .image: return "photo"
        case .audio: return "waveform"
        case .video: return "film"
        case .archive: return "archivebox"
        case .code: return "chevron.left.forwardslash.chevron.right"
        case .data: return "tablecells"
        case .other: return "questionmark.folder"
        }
    }

    private func categoryColor(_ category: ContentCategory) -> Color {
        switch category {
        case .document: return .blue
        case .image: return .green
        case .audio: return .purple
        case .video: return .red
        case .archive: return .orange
        case .code: return .teal
        case .data: return .indigo
        case .other: return .gray
        }
    }

    // MARK: - RDF Triple Display

    private func tripleDescription(_ triple: RDFTriple) -> String {
        let s = nodeDescription(triple.subject)
        let o = nodeDescription(triple.object)
        return "\(s) \(triple.predicate) \(o)"
    }

    private func nodeDescription(_ node: RDFNode) -> String {
        switch node {
        case .uri(let uri): return "<\(uri)>"
        case .literal(let value, _): return "\"\(value)\""
        case .blank(let id): return "_:\(id)"
        }
    }
}
