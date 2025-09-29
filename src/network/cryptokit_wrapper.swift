import Foundation
import CryptoKit

@objc public class CryptoKitWrapper: NSObject
{
    // MARK: - Static Utility Methods

    @objc public static func base64Encode(_ data: Data) -> String
    {
        return data.base64EncodedString()
    }

    @objc public static func base64Decode(_ string: String) -> Data?
    {
        return Data(base64Encoded: string)
    }

    @objc public static func sha256Hash(_ data: Data) -> Data
    {
        let hash = SHA256.hash(data: data)
        return Data(hash)
    }

    // MARK: - AES-GCM Encryption/Decryption Methods
    @objc public static func aesGcmEncryptWithPlaintext(_ plaintext: Data, key: Data, iv: Data) -> Data?
    {
        return autoreleasepool
        {
            guard key.count == 16, iv.count == 12
            else
            {
                return nil
            }
            do
            {
                let symmetricKey = SymmetricKey(data: key)
                let nonce = try AES.GCM.Nonce(data: iv)
                let sealedBox = try AES.GCM.seal(plaintext, using: symmetricKey, nonce: nonce)

                // Return combined ciphertext + tag (tag is 16 bytes)
                var result = Data()
                result.append(sealedBox.tag)
                result.append(sealedBox.ciphertext)
                return result
            }
            catch
            {
                return nil
            }
        }
    }
    
    @objc public static func aesGcmDecryptWithCiphertext(_ ciphertext: Data, key: Data, iv: Data) -> Data?
    {
        return autoreleasepool
        {
            guard key.count == 16, iv.count == 12, ciphertext.count >= 16
            else
            {
                return nil
            }
            do
            {
                let symmetricKey = SymmetricKey(data: key)
                let nonce = try AES.GCM.Nonce(data: iv)
                // Extract tag (first 16 bytes) and ciphertext (remaining bytes)
                let tag = ciphertext.prefix(16)
                let actualCiphertext = ciphertext.dropFirst(16)
                let sealedBox = try AES.GCM.SealedBox(nonce: nonce, 
                                                      ciphertext: actualCiphertext, 
                                                      tag: tag)
                let plaintext = try AES.GCM.open(sealedBox, using: symmetricKey)
                return plaintext
            }
            catch
            {
                return nil
            }
        }
    }

    // MARK: - Key Generation
    @objc public static func generateSymmetricKey() -> Data
    {
        let key = SymmetricKey(size: .bits128)
        return key.withUnsafeBytes { Data($0) }
    }

    @objc public static func generateRandomIV() -> Data
    {
        var iv = Data(count: 12)
        let result = iv.withUnsafeMutableBytes
        {
            bytes in SecRandomCopyBytes(kSecRandomDefault, 12, bytes.bindMemory(to: UInt8.self).baseAddress!)
        }
        return result == errSecSuccess ? iv : Data()
    }
}
