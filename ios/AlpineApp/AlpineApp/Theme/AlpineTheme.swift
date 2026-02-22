import SwiftUI

enum AlpineTheme {
    static let alpineGreen = Color("AccentColor")
    static let alpineBlue = Color(red: 0x15 / 255.0, green: 0x65 / 255.0, blue: 0xC0 / 255.0)

    static let cardBackground = Color(.systemBackground)
    static let secondaryBackground = Color(.secondarySystemBackground)

    enum Typography {
        static let title = Font.title2.weight(.semibold)
        static let headline = Font.headline
        static let body = Font.body
        static let caption = Font.caption
        static let statValue = Font.system(size: 28, weight: .bold, design: .rounded)
    }
}
