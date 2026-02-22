import SwiftUI

/// Displays a TOTP code with a circular countdown timer.
struct TOTPCodeView: View {
    let secret: Data
    let config: TOTPConfig

    @State private var code = ""
    @State private var secondsRemaining = 0
    @State private var timer: Timer?

    var body: some View {
        HStack(spacing: 16) {
            // Countdown ring
            ZStack {
                Circle()
                    .stroke(Color.gray.opacity(0.2), lineWidth: 3)
                    .frame(width: 40, height: 40)
                Circle()
                    .trim(from: 0, to: CGFloat(secondsRemaining) / CGFloat(config.period))
                    .stroke(timerColor, style: StrokeStyle(lineWidth: 3, lineCap: .round))
                    .frame(width: 40, height: 40)
                    .rotationEffect(.degrees(-90))
                    .animation(.linear(duration: 1), value: secondsRemaining)
                Text("\(secondsRemaining)")
                    .font(.system(.caption, design: .monospaced))
                    .foregroundStyle(timerColor)
            }

            // Code display
            Text(formattedCode)
                .font(.system(size: 32, weight: .medium, design: .monospaced))
                .tracking(4)

            // Copy button
            Button {
                UIPasteboard.general.string = code
            } label: {
                Image(systemName: "doc.on.doc")
                    .font(.body)
            }
            .buttonStyle(.bordered)
        }
        .onAppear { startTimer() }
        .onDisappear { stopTimer() }
    }

    private var formattedCode: String {
        guard code.count == config.digits else { return code }
        let mid = code.index(code.startIndex, offsetBy: config.digits / 2)
        return "\(code[code.startIndex..<mid]) \(code[mid...])"
    }

    private var timerColor: Color {
        secondsRemaining <= 5 ? .red : AlpineTheme.alpineGreen
    }

    private func startTimer() {
        updateCode()
        timer = Timer.scheduledTimer(withTimeInterval: 1, repeats: true) { _ in
            updateCode()
        }
    }

    private func stopTimer() {
        timer?.invalidate()
        timer = nil
    }

    private func updateCode() {
        code = TOTPGenerator.generateCode(secret: secret, config: config)
        secondsRemaining = TOTPGenerator.secondsRemaining(config: config)
    }
}
