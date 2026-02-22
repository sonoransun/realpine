import SwiftUI

enum AlpineTheme {
    static let alpineGreen = Color("AccentColor")
    static let alpineBlue = Color(red: 0x15 / 255.0, green: 0x65 / 255.0, blue: 0xC0 / 255.0)

    // MARK: - Semantic Colors (light/dark adaptive)

    static let background = Color(.systemBackground)
    static let secondaryBackground = Color(.secondarySystemBackground)
    static let cardBackground = Color(.tertiarySystemBackground)
    static let textPrimary = Color(.label)
    static let textSecondary = Color(.secondaryLabel)
    static let destructive = Color(.systemRed)
    static let warning = Color(.systemOrange)

    static let success = alpineGreen

    // MARK: - Typography

    enum Typography {
        static let title = Font.title2.weight(.semibold)
        static let headline = Font.headline
        static let body = Font.body
        static let caption = Font.caption
        static let statValue = Font.system(size: 28, weight: .bold, design: .rounded)
    }

    // MARK: - Spacing

    enum Spacing: CGFloat {
        case xs = 4
        case sm = 8
        case md = 16
        case lg = 24
        case xl = 32
    }

    // MARK: - Corner Radius

    enum CornerRadius: CGFloat {
        case sm = 8
        case md = 12
        case lg = 16
    }

    // MARK: - Shadows

    enum Shadows {
        static func small() -> some ViewModifier {
            ShadowModifier(color: .black.opacity(0.1), radius: 2, x: 0, y: 1)
        }

        static func medium() -> some ViewModifier {
            ShadowModifier(color: .black.opacity(0.15), radius: 4, x: 0, y: 2)
        }

        static func large() -> some ViewModifier {
            ShadowModifier(color: .black.opacity(0.2), radius: 8, x: 0, y: 4)
        }
    }

    // MARK: - Animation Constants

    enum Animations {
        static let standard = Animation.easeInOut(duration: 0.25)
        static let spring = Animation.spring(response: 0.4, dampingFraction: 0.75)
    }
}

// MARK: - Shadow ViewModifier

private struct ShadowModifier: ViewModifier {
    let color: Color
    let radius: CGFloat
    let x: CGFloat
    let y: CGFloat

    func body(content: Content) -> some View {
        content.shadow(color: color, radius: radius, x: x, y: y)
    }
}
