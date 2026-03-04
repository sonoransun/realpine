import Foundation
import Security

enum ProtectionLevel {
    case standard
    case biometricRequired
    case devicePasscode
}

enum SecureStorageError: Error, LocalizedError {
    case accessControlCreationFailed
    case biometricAuthFailed
    case itemNotFound
    case unexpectedError(OSStatus)

    var errorDescription: String? {
        switch self {
        case .accessControlCreationFailed:
            return "Failed to create access control for keychain item."
        case .biometricAuthFailed:
            return "Biometric authentication failed."
        case .itemNotFound:
            return "The requested keychain item was not found."
        case .unexpectedError(let status):
            return "Unexpected keychain error: \(status)"
        }
    }
}

@Observable
final class SecureStorage {
    private let service = "com.alpine.app"
    private var biometricService: String { service + ".bio" }

    // MARK: - Standard Storage

    func store(key: String, value: String) -> Bool {
        // Remove any existing item first
        remove(key: key)

        guard let data = value.data(using: .utf8) else {
            return false
        }

        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: service,
            kSecAttrAccount as String: key,
            kSecValueData as String: data,
            kSecAttrAccessible as String: kSecAttrAccessibleAfterFirstUnlock
        ]

        let status = SecItemAdd(query as CFDictionary, nil)
        return status == errSecSuccess
    }

    func read(key: String) -> String? {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: service,
            kSecAttrAccount as String: key,
            kSecReturnData as String: true,
            kSecMatchLimit as String: kSecMatchLimitOne
        ]

        var result: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &result)

        guard status == errSecSuccess,
              let data = result as? Data,
              let string = String(data: data, encoding: .utf8) else {
            return nil
        }

        return string
    }

    func remove(key: String) {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: service,
            kSecAttrAccount as String: key
        ]

        SecItemDelete(query as CFDictionary)
    }

    func storeData(key: String, value: Data) -> Bool {
        remove(key: key)

        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: service,
            kSecAttrAccount as String: key,
            kSecValueData as String: value,
            kSecAttrAccessible as String: kSecAttrAccessibleAfterFirstUnlock
        ]

        let status = SecItemAdd(query as CFDictionary, nil)
        return status == errSecSuccess
    }

    func readData(key: String) -> Data? {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: service,
            kSecAttrAccount as String: key,
            kSecReturnData as String: true,
            kSecMatchLimit as String: kSecMatchLimitOne
        ]

        var result: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &result)

        guard status == errSecSuccess,
              let data = result as? Data else {
            return nil
        }

        return data
    }

    // MARK: - Biometric-Protected Storage

    func storeBiometricProtected(key: String, value: String) -> Bool {
        removeBiometricProtected(key: key)

        guard let data = value.data(using: .utf8) else { return false }
        guard let access = accessControl(for: .biometricRequired) else { return false }

        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: biometricService,
            kSecAttrAccount as String: key,
            kSecValueData as String: data,
            kSecAttrAccessControl as String: access
        ]

        let status = SecItemAdd(query as CFDictionary, nil)
        return status == errSecSuccess
    }

    func readBiometricProtected(key: String) async throws -> String? {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: biometricService,
            kSecAttrAccount as String: key,
            kSecReturnData as String: true,
            kSecMatchLimit as String: kSecMatchLimitOne
        ]

        var result: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &result)

        switch status {
        case errSecSuccess:
            guard let data = result as? Data,
                  let string = String(data: data, encoding: .utf8) else { return nil }
            return string
        case errSecAuthFailed:
            throw SecureStorageError.biometricAuthFailed
        case errSecItemNotFound:
            throw SecureStorageError.itemNotFound
        default:
            throw SecureStorageError.unexpectedError(status)
        }
    }

    func storeBiometricProtectedData(key: String, value: Data) -> Bool {
        removeBiometricProtected(key: key)

        guard let access = accessControl(for: .biometricRequired) else { return false }

        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: biometricService,
            kSecAttrAccount as String: key,
            kSecValueData as String: value,
            kSecAttrAccessControl as String: access
        ]

        let status = SecItemAdd(query as CFDictionary, nil)
        return status == errSecSuccess
    }

    func readBiometricProtectedData(key: String) async throws -> Data? {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: biometricService,
            kSecAttrAccount as String: key,
            kSecReturnData as String: true,
            kSecMatchLimit as String: kSecMatchLimitOne
        ]

        var result: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &result)

        switch status {
        case errSecSuccess:
            guard let data = result as? Data else { return nil }
            return data
        case errSecAuthFailed:
            throw SecureStorageError.biometricAuthFailed
        case errSecItemNotFound:
            throw SecureStorageError.itemNotFound
        default:
            throw SecureStorageError.unexpectedError(status)
        }
    }

    func removeBiometricProtected(key: String) {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: biometricService,
            kSecAttrAccount as String: key
        ]

        SecItemDelete(query as CFDictionary)
    }

    // MARK: - Access Control Helper

    private func accessControl(for level: ProtectionLevel) -> SecAccessControl? {
        var error: Unmanaged<CFError>?
        let access: SecAccessControl?

        switch level {
        case .standard:
            return nil
        case .biometricRequired:
            access = SecAccessControlCreateWithFlags(
                kCFAllocatorDefault,
                kSecAttrAccessibleWhenPasscodeSetThisDeviceOnly,
                .biometryCurrentSet,
                &error
            )
        case .devicePasscode:
            access = SecAccessControlCreateWithFlags(
                kCFAllocatorDefault,
                kSecAttrAccessibleWhenPasscodeSetThisDeviceOnly,
                .devicePasscode,
                &error
            )
        }

        if let error {
            _ = error.takeRetainedValue()
            return nil
        }

        return access
    }
}
