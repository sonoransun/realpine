import Foundation
import Security

enum TOTPEnrollment {

    /// Parse an otpauth:// URI into a TOTPSecret.
    static func parseOTPAuthURI(_ uri: String) -> TOTPSecret? {
        guard let components = URLComponents(string: uri),
              components.scheme == "otpauth",
              components.host == "totp" else {
            return nil
        }

        let params = Dictionary(
            uniqueKeysWithValues: (components.queryItems ?? []).compactMap { item in
                item.value.map { (item.name, $0) }
            }
        )

        guard let secretString = params["secret"],
              let secretData = base32Decode(secretString) else {
            return nil
        }

        let algorithm: TOTPAlgorithm
        switch params["algorithm"]?.uppercased() {
        case "SHA256": algorithm = .sha256
        case "SHA512": algorithm = .sha512
        default: algorithm = .sha1
        }

        let digits = params["digits"].flatMap(Int.init) ?? 6
        let period = params["period"].flatMap(Int.init) ?? 30

        let label = components.path.trimmingCharacters(in: CharacterSet(charactersIn: "/"))
        let parts = label.split(separator: ":", maxSplits: 1)
        let issuer = params["issuer"] ?? (parts.count > 1 ? String(parts[0]) : "")
        let account = parts.count > 1 ? String(parts[1]) : label

        return TOTPSecret(
            secret: secretData,
            config: TOTPConfig(period: period, digits: digits, algorithm: algorithm),
            issuer: issuer,
            account: account,
            enrolledAt: Date()
        )
    }

    /// Generate an otpauth:// URI from a TOTPSecret.
    static func generateQRPayload(secret: TOTPSecret) -> String {
        let encodedSecret = base32Encode(secret.secret)
        let label = secret.issuer.isEmpty
            ? secret.account
            : "\(secret.issuer):\(secret.account)"
        let escapedLabel = label.addingPercentEncoding(withAllowedCharacters: .urlPathAllowed) ?? label
        var uri = "otpauth://totp/\(escapedLabel)?secret=\(encodedSecret)"
        if !secret.issuer.isEmpty {
            uri += "&issuer=\(secret.issuer.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) ?? secret.issuer)"
        }
        uri += "&algorithm=\(secret.config.algorithm.rawValue.uppercased())"
        uri += "&digits=\(secret.config.digits)"
        uri += "&period=\(secret.config.period)"
        return uri
    }

    /// Generate cryptographically random secret bytes.
    static func generateRandomSecret(length: Int = 20) -> Data {
        var bytes = [UInt8](repeating: 0, count: length)
        _ = SecRandomCopyBytes(kSecRandomDefault, length, &bytes)
        return Data(bytes)
    }

    // MARK: - Base32

    private static let base32Alphabet = Array("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567")

    static func base32Decode(_ input: String) -> Data? {
        let cleaned = input.uppercased().filter { $0 != "=" }
        var output = [UInt8]()
        var buffer: UInt64 = 0
        var bitsLeft = 0

        for char in cleaned {
            guard let index = base32Alphabet.firstIndex(of: char) else { return nil }
            buffer = (buffer << 5) | UInt64(index)
            bitsLeft += 5
            if bitsLeft >= 8 {
                bitsLeft -= 8
                output.append(UInt8((buffer >> bitsLeft) & 0xff))
            }
        }
        return Data(output)
    }

    static func base32Encode(_ data: Data) -> String {
        var result = ""
        var buffer: UInt64 = 0
        var bitsLeft = 0

        for byte in data {
            buffer = (buffer << 8) | UInt64(byte)
            bitsLeft += 8
            while bitsLeft >= 5 {
                bitsLeft -= 5
                let index = Int((buffer >> bitsLeft) & 0x1f)
                result.append(base32Alphabet[index])
            }
        }
        if bitsLeft > 0 {
            let index = Int((buffer << (5 - bitsLeft)) & 0x1f)
            result.append(base32Alphabet[index])
        }
        return result
    }
}
