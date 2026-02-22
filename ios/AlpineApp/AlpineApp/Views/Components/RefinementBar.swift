import SwiftUI

struct RefinementBar: View {
    let refinements: [QueryRefinement]
    let onApply: (QueryRefinement) -> Void

    var body: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 8) {
                ForEach(refinements) { refinement in
                    Button {
                        onApply(refinement)
                    } label: {
                        HStack(spacing: 4) {
                            Image(systemName: refinementIcon(refinement.refinementType))
                            Text(refinement.label)
                        }
                        .font(.caption)
                        .padding(.horizontal, 12)
                        .padding(.vertical, 6)
                        .background(AlpineTheme.alpineBlue.opacity(0.15))
                        .foregroundStyle(AlpineTheme.alpineBlue)
                        .clipShape(Capsule())
                    }
                }
            }
            .padding(.horizontal)
        }
    }

    private func refinementIcon(_ type: RefinementType) -> String {
        switch type {
        case .addCategoryFilter: return "square.grid.2x2"
        case .addLanguageFilter: return "globe"
        case .addEntityFilter: return "person.text.rectangle"
        case .addKeywordFilter: return "plus.magnifyingglass"
        case .adjustThreshold: return "slider.horizontal.3"
        case .removeAmbiguity: return "arrow.triangle.branch"
        }
    }
}
