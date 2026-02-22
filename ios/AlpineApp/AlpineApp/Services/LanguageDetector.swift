import Foundation
import NaturalLanguage

final class LanguageDetector: Sendable {

    /// Detect dominant language from path tokens using NLLanguageRecognizer.
    func detectLanguage(tokens: [String]) -> LanguageInfo? {
        let text = tokens.joined(separator: " ")
        guard text.count >= 3 else { return nil }

        let recognizer = NLLanguageRecognizer()
        recognizer.processString(text)

        guard let dominant = recognizer.dominantLanguage else { return nil }

        let hypotheses = recognizer.languageHypotheses(withMaximum: 1)
        let confidence = hypotheses[dominant] ?? 0.0

        guard confidence >= 0.3 else { return nil }

        return LanguageInfo(languageCode: dominant.rawValue, confidence: confidence)
    }
}
