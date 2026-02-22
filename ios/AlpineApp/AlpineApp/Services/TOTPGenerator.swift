import Foundation
import CryptoKit

enum TOTPGenerator {

    static func generateCode(secret: Data, time: Date = Date(), config: TOTPConfig = .default) -> String {
        let counter = UInt64(time.timeIntervalSince1970) / UInt64(config.period)
        var bigEndianCounter = counter.bigEndian
        let counterData = Data(bytes: &bigEndianCounter, count: 8)
        let key = SymmetricKey(data: secret)

        let hashBytes: [UInt8]
        switch config.algorithm {
        case .sha1:
            let mac = HMAC<Insecure.SHA1>.authenticationCode(for: counterData, using: key)
            hashBytes = Array(mac)
        case .sha256:
            let mac = HMAC<SHA256>.authenticationCode(for: counterData, using: key)
            hashBytes = Array(mac)
        case .sha512:
            let mac = HMAC<SHA512>.authenticationCode(for: counterData, using: key)
            hashBytes = Array(mac)
        }

        let offset = Int(hashBytes[hashBytes.count - 1] & 0x0f)
        let truncated = (UInt32(hashBytes[offset]) & 0x7f) << 24
            | UInt32(hashBytes[offset + 1]) << 16
            | UInt32(hashBytes[offset + 2]) << 8
            | UInt32(hashBytes[offset + 3])

        let mod = truncated % UInt32(pow(10.0, Double(config.digits)))
        return String(format: "%0\(config.digits)d", mod)
    }

    static func validateCode(_ code: String, secret: Data, config: TOTPConfig = .default, window: Int = 1) -> Bool {
        let now = Date()
        for offset in -window...window {
            let time = now.addingTimeInterval(Double(offset * config.period))
            if generateCode(secret: secret, time: time, config: config) == code {
                return true
            }
        }
        return false
    }

    static func secondsRemaining(config: TOTPConfig = .default) -> Int {
        let elapsed = Int(Date().timeIntervalSince1970) % config.period
        return config.period - elapsed
    }
}
