#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <Arduino.h>
#include "fram_programmer.h"

#define SHA256_HASH_SIZE    32          // 256 bits

// Encryption functions
bool generateEncryptionKey(const String& device_name, uint8_t* key);
bool generateRandomIV(uint8_t* iv);
bool encryptData(const uint8_t* plaintext, size_t plaintext_len, 
                 const uint8_t* key, const uint8_t* iv,
                 uint8_t* ciphertext, size_t* ciphertext_len);
bool decryptData(const uint8_t* ciphertext, size_t ciphertext_len,
                 const uint8_t* key, const uint8_t* iv,
                 uint8_t* plaintext, size_t* plaintext_len);

// Utility functions
bool sha256Hash(const String& input, uint8_t* hash);
bool sha256Hash(const uint8_t* data, size_t len, uint8_t* hash);
size_t addPKCS7Padding(uint8_t* data, size_t data_len, size_t block_size);
size_t removePKCS7Padding(const uint8_t* data, size_t data_len);

// High-level credential encryption
bool encryptCredentials(const DeviceCredentials& creds, FRAMCredentials& fram_creds);
bool decryptCredentials(const FRAMCredentials& fram_creds, DeviceCredentials& creds);

// Validation functions
bool validateDeviceName(const String& name);
bool validateWiFiSSID(const String& ssid);
bool validateWiFiPassword(const String& password);
bool validateVPSToken(const String& token);

#endif // ENCRYPTION_H