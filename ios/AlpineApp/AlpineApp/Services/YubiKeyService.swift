import Foundation
#if canImport(CoreNFC)
import CoreNFC
#endif

/// Errors from YubiKey operations.
enum YubiKeyError: Error, LocalizedError {
    case nfcNotAvailable
    case connectionFailed
    case timeout
    case invalidResponse
    case cancelled

    var errorDescription: String? {
        switch self {
        case .nfcNotAvailable: "NFC is not available on this device"
        case .connectionFailed: "Failed to connect to YubiKey"
        case .timeout: "YubiKey operation timed out"
        case .invalidResponse: "Invalid response from YubiKey"
        case .cancelled: "YubiKey operation was cancelled"
        }
    }
}

/// Service for YubiKey NFC and Lightning/USB-C communication.
@Observable
final class YubiKeyService {

    private(set) var isScanning = false

    /// Whether NFC reading is available on this device.
    var isAvailable: Bool {
        #if canImport(CoreNFC)
        if #available(iOS 13.0, *) {
            return NFCTagReaderSession.readingAvailable
        }
        #endif
        return false
    }

    /// Request an OTP from a YubiKey via NFC tap.
    func requestOTP() async throws -> String {
        // YubiKit integration point:
        // 1. Start NFC session
        // 2. Connect to YubiKey
        // 3. Read OTP slot
        // 4. Return OTP string
        throw YubiKeyError.nfcNotAvailable // Placeholder until YubiKit is integrated
    }

    /// Perform HMAC-SHA1 challenge-response via OATH applet.
    func performChallengeResponse(challenge: Data) async throws -> Data {
        // YubiKit integration point:
        // 1. Start NFC session
        // 2. Select OATH applet
        // 3. Send challenge
        // 4. Return response
        throw YubiKeyError.nfcNotAvailable // Placeholder until YubiKit is integrated
    }

    /// Cancel any in-progress NFC scanning.
    func cancelScan() {
        isScanning = false
    }
}
