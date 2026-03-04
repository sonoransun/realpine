/// Copyright (C) 2026 sonoransun — see LICENCE.txt

import Foundation
import Security

/// Manages Secure Enclave key pairs for device authentication, including key generation,
/// signing, and public key export using ECDSA P-256 keys stored in the Secure Enclave.
@Observable
final class SecureEnclaveManager {

    // MARK: - Constants

    private static let keyTag = "com.alpine.app.device-auth-key"

    // MARK: - Error Types

    enum SecureEnclaveError: Error, LocalizedError {
        case keyGenerationFailed(String)
        case keyNotFound
        case signingFailed(String)
        case publicKeyExportFailed
        case secureEnclaveNotAvailable

        var errorDescription: String? {
            switch self {
            case .keyGenerationFailed(let message):
                "Secure Enclave key generation failed: \(message)"
            case .keyNotFound:
                "No device authentication key found in Secure Enclave"
            case .signingFailed(let message):
                "Signing operation failed: \(message)"
            case .publicKeyExportFailed:
                "Failed to export public key representation"
            case .secureEnclaveNotAvailable:
                "Secure Enclave is not available on this device"
            }
        }
    }

    // MARK: - Availability

    /// Checks whether the Secure Enclave is available on this device by attempting
    /// to create access control flags that require the Secure Enclave token.
    static var isAvailable: Bool {
        var error: Unmanaged<CFError>?
        let accessControl = SecAccessControlCreateWithFlags(
            kCFAllocatorDefault,
            kSecAttrAccessibleWhenPasscodeSetThisDeviceOnly,
            .privateKeyUsage,
            &error
        )

        guard accessControl != nil else {
            return false
        }

        // Attempt to create a transient (non-permanent) key to verify Secure Enclave support
        let attributes: [String: Any] = [
            kSecAttrKeyType as String: kSecAttrKeyTypeECSECPrimeRandom,
            kSecAttrKeySizeInBits as String: 256,
            kSecAttrTokenID as String: kSecAttrTokenIDSecureEnclave,
            kSecAttrIsPermanent as String: false
        ]

        var keyError: Unmanaged<CFError>?
        let testKey = SecKeyCreateRandomKey(attributes as CFDictionary, &keyError)
        return testKey != nil
    }

    // MARK: - Key Generation

    /// Generates a new ECDSA P-256 key pair in the Secure Enclave. Any previously stored
    /// key pair with the same tag is deleted first.
    ///
    /// - Parameter biometricProtected: When `true`, private key operations require
    ///   biometric authentication with the currently enrolled biometric set.
    /// - Returns: The newly created private key reference.
    func generateKeyPair(biometricProtected: Bool) throws -> SecKey {
        deleteKeyPair()

        var accessControlFlags: SecAccessControlCreateFlags = .privateKeyUsage
        if biometricProtected {
            accessControlFlags.insert(.biometryCurrentSet)
        }

        var accessControlError: Unmanaged<CFError>?
        guard let accessControl = SecAccessControlCreateWithFlags(
            kCFAllocatorDefault,
            kSecAttrAccessibleWhenUnlockedThisDeviceOnly,
            accessControlFlags,
            &accessControlError
        ) else {
            let message = accessControlError?.takeRetainedValue().localizedDescription
                ?? "Unknown access control error"
            throw SecureEnclaveError.keyGenerationFailed(message)
        }

        let attributes: [String: Any] = [
            kSecAttrKeyType as String: kSecAttrKeyTypeECSECPrimeRandom,
            kSecAttrKeySizeInBits as String: 256,
            kSecAttrTokenID as String: kSecAttrTokenIDSecureEnclave,
            kSecAttrIsPermanent as String: true,
            kSecAttrApplicationTag as String: Self.keyTag.data(using: .utf8)!,
            kSecPrivateKeyAttrs as String: [
                kSecAttrAccessControl as String: accessControl
            ]
        ]

        var error: Unmanaged<CFError>?
        guard let privateKey = SecKeyCreateRandomKey(attributes as CFDictionary, &error) else {
            let message = error?.takeRetainedValue().localizedDescription
                ?? "Unknown key generation error"
            throw SecureEnclaveError.keyGenerationFailed(message)
        }

        return privateKey
    }

    // MARK: - Public Key Export

    /// Retrieves the public key corresponding to the stored Secure Enclave private key
    /// and returns its external (X9.63) representation.
    func publicKeyData() throws -> Data {
        let privateKey = try lookupPrivateKey()

        guard let publicKey = SecKeyCopyPublicKey(privateKey) else {
            throw SecureEnclaveError.publicKeyExportFailed
        }

        var error: Unmanaged<CFError>?
        guard let data = SecKeyCopyExternalRepresentation(publicKey, &error) as Data? else {
            throw SecureEnclaveError.publicKeyExportFailed
        }

        return data
    }

    // MARK: - Signing

    /// Signs the provided data using the Secure Enclave private key with ECDSA SHA-256.
    /// If the key was created with biometric protection, the system will automatically
    /// present a biometric prompt.
    ///
    /// - Parameter data: The data to sign.
    /// - Returns: The ECDSA signature in X9.62 format.
    func sign(data: Data) async throws -> Data {
        let privateKey = try lookupPrivateKey()

        var error: Unmanaged<CFError>?
        guard let signature = SecKeyCreateSignature(
            privateKey,
            .ecdsaSignatureMessageX962SHA256,
            data as CFData,
            &error
        ) as Data? else {
            let message = error?.takeRetainedValue().localizedDescription
                ?? "Unknown signing error"
            throw SecureEnclaveError.signingFailed(message)
        }

        return signature
    }

    // MARK: - Key Management

    /// Deletes the stored Secure Enclave key pair, if one exists.
    func deleteKeyPair() {
        let query: [String: Any] = [
            kSecClass as String: kSecClassKey,
            kSecAttrApplicationTag as String: Self.keyTag.data(using: .utf8)!,
            kSecAttrKeyType as String: kSecAttrKeyTypeECSECPrimeRandom
        ]

        SecItemDelete(query as CFDictionary)
    }

    /// Whether a device authentication key pair currently exists in the Secure Enclave.
    var hasKeyPair: Bool {
        let query: [String: Any] = [
            kSecClass as String: kSecClassKey,
            kSecAttrApplicationTag as String: Self.keyTag.data(using: .utf8)!,
            kSecAttrKeyType as String: kSecAttrKeyTypeECSECPrimeRandom,
            kSecReturnRef as String: false
        ]

        return SecItemCopyMatching(query as CFDictionary, nil) == errSecSuccess
    }

    // MARK: - Private Helpers

    /// Looks up the existing Secure Enclave private key from the keychain.
    private func lookupPrivateKey() throws -> SecKey {
        let query: [String: Any] = [
            kSecClass as String: kSecClassKey,
            kSecAttrApplicationTag as String: Self.keyTag.data(using: .utf8)!,
            kSecAttrKeyType as String: kSecAttrKeyTypeECSECPrimeRandom,
            kSecReturnRef as String: true,
            kSecMatchLimit as String: kSecMatchLimitOne
        ]

        var result: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &result)

        guard status == errSecSuccess, let key = result else {
            throw SecureEnclaveError.keyNotFound
        }

        // swiftlint:disable:next force_cast
        return key as! SecKey
    }
}
