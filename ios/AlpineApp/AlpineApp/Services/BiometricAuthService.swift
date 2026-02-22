import LocalAuthentication
import Observation

/// Provides biometric authentication (Face ID / Touch ID) capability checks and evaluation.
@Observable
final class BiometricAuthService {
    enum BiometricType {
        case none
        case touchID
        case faceID
    }

    enum BiometricError: Error, LocalizedError {
        case notAvailable
        case notEnrolled
        case cancelled
        case failed(String)

        var errorDescription: String? {
            switch self {
            case .notAvailable:
                "Biometric authentication is not available on this device"
            case .notEnrolled:
                "No biometric credentials are enrolled"
            case .cancelled:
                "Authentication was cancelled"
            case .failed(let message):
                "Authentication failed: \(message)"
            }
        }
    }

    private(set) var biometricType: BiometricType = .none
    private(set) var isAvailable: Bool = false
    private(set) var error: String?

    init() {
        checkAvailability()
    }

    /// Evaluates whether biometric authentication is available and determines the type.
    func checkAvailability() {
        let context = LAContext()
        var authError: NSError?
        let canEvaluate = context.canEvaluatePolicy(
            .deviceOwnerAuthenticationWithBiometrics,
            error: &authError
        )

        isAvailable = canEvaluate

        if canEvaluate {
            switch context.biometryType {
            case .touchID:
                biometricType = .touchID
            case .faceID:
                biometricType = .faceID
            default:
                biometricType = .none
            }
            error = nil
        } else {
            biometricType = .none
            error = authError?.localizedDescription
        }
    }

    /// Prompts the user for biometric authentication with the given reason string.
    func authenticate(reason: String) async throws -> Bool {
        let context = LAContext()
        var authError: NSError?

        guard context.canEvaluatePolicy(
            .deviceOwnerAuthenticationWithBiometrics,
            error: &authError
        ) else {
            if let nsError = authError {
                switch LAError.Code(rawValue: nsError.code) {
                case .biometryNotEnrolled:
                    throw BiometricError.notEnrolled
                default:
                    throw BiometricError.notAvailable
                }
            }
            throw BiometricError.notAvailable
        }

        do {
            let success = try await context.evaluatePolicy(
                .deviceOwnerAuthenticationWithBiometrics,
                localizedReason: reason
            )
            return success
        } catch let laError as LAError {
            switch laError.code {
            case .userCancel, .appCancel, .systemCancel:
                throw BiometricError.cancelled
            case .biometryNotEnrolled:
                throw BiometricError.notEnrolled
            case .biometryNotAvailable:
                throw BiometricError.notAvailable
            default:
                throw BiometricError.failed(laError.localizedDescription)
            }
        }
    }
}
