#include "encryption.h"
#include "sha256.h"
#include "aes.h"
#include <stddef.h>

// AES-256-CBC instance
AES256_CBC aes_cbc;

bool generateEncryptionKey(const String& device_name, uint8_t* key) {
    // Create key material: device_name + salt + seed
    String key_material = device_name + ENCRYPTION_SALT + ENCRYPTION_SEED;
    
    // Hash to create 256-bit key
    return sha256Hash(key_material, key);
}

bool generateRandomIV(uint8_t* iv) {
    // Generate random IV using built-in random functions
    randomSeed(millis() ^ analogRead(A0));
    
    for (int i = 0; i < AES_IV_SIZE; i++) {
        iv[i] = random(0, 256);
    }
    
    // Add more entropy
    for (int i = 0; i < AES_IV_SIZE; i++) {
        iv[i] ^= (micros() & 0xFF);
        delay(1);
    }
    
    return true;
}

bool encryptData(const uint8_t* plaintext, size_t plaintext_len,
                 const uint8_t* key, const uint8_t* iv,
                 uint8_t* ciphertext, size_t* ciphertext_len) {
    
    // Calculate padded length (must be multiple of 16)
    size_t padded_len = ((plaintext_len + 15) / 16) * 16;
    
    // IMPORTANT: If plaintext_len is already multiple of 16, add full block
    if (plaintext_len % AES_BLOCK_SIZE == 0) {
        padded_len += AES_BLOCK_SIZE;
    }
    
    Serial.print("DEBUG encryptData: plaintext_len=");
    Serial.print(plaintext_len);
    Serial.print(", padded_len=");
    Serial.print(padded_len);
    Serial.print(", field_size=");
    Serial.println(*ciphertext_len);
    
    if (padded_len > *ciphertext_len) {
        Serial.println("ERROR encryptData: Padded data larger than field size!");
        return false;
    }
    
    // Create padded buffer
    uint8_t* padded_data = new uint8_t[*ciphertext_len];
    if (!padded_data) {
        return false; // Memory allocation failed
    }
    
    // Initialize entire buffer with zeros
    memset(padded_data, 0, *ciphertext_len);
    
    // Copy data and add PKCS7 padding
    memcpy(padded_data, plaintext, plaintext_len);
    size_t actual_padded_len = addPKCS7Padding(padded_data, plaintext_len, AES_BLOCK_SIZE);
    
    Serial.print("DEBUG encryptData: actual_padded_len=");
    Serial.println(actual_padded_len);
    
    // Set up AES-256-CBC
    aes_cbc.set_key(key);
    
    // Extend 8-byte IV to 16-byte IV for AES
    uint8_t full_iv[16];
    for (int i = 0; i < 16; i++) {
        full_iv[i] = iv[i % AES_IV_SIZE];
    }
    aes_cbc.set_iv(full_iv);
    
    // Encrypt the entire field (including any zeros at the end)
    if (!aes_cbc.encrypt(padded_data, *ciphertext_len, ciphertext)) {
        delete[] padded_data;
        return false;
    }
    
    // ciphertext_len stays the same (full field size)
    
    delete[] padded_data;
    return true;
}

bool decryptData(const uint8_t* ciphertext, size_t ciphertext_len,
                 const uint8_t* key, const uint8_t* iv,
                 uint8_t* plaintext, size_t* plaintext_len) {
    
    if (ciphertext_len % AES_BLOCK_SIZE != 0) {
        return false;
    }
    
    // Set up AES-256-CBC
    aes_cbc.set_key(key);
    
    // Extend 8-byte IV to 16-byte IV for AES
    uint8_t full_iv[16];
    for (int i = 0; i < 16; i++) {
        full_iv[i] = iv[i % AES_IV_SIZE];
    }
    aes_cbc.set_iv(full_iv);
    
    // Decrypt data
    if (!aes_cbc.decrypt(ciphertext, ciphertext_len, plaintext)) {
        return false;
    }
    
    // Debug: Show decrypted blocks for larger ciphertext
    if (ciphertext_len > 64) {
        Serial.print("DEBUG decryptData: Showing decrypted blocks for ");
        Serial.print(ciphertext_len);
        Serial.println(" byte field:");
        
        size_t blocks_to_show = min((size_t)6, ciphertext_len / 16);
        for (size_t block = 0; block < blocks_to_show; block++) {
            Serial.print("  Block ");
            Serial.print(block);
            Serial.print(": ");
            for (int i = 0; i < 16; i++) {
                if (plaintext[block * 16 + i] < 16) Serial.print("0");
                Serial.print(plaintext[block * 16 + i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
    }
    
    // New approach: Try to find valid PKCS7 padding by looking for non-zero data
    // and checking padding from that point onwards
    size_t actual_data_len = 0;
    
    // First, try to remove padding from the entire decrypted buffer
    // This handles cases where data + padding fits in the buffer
    for (size_t try_len = AES_BLOCK_SIZE; try_len <= ciphertext_len; try_len += AES_BLOCK_SIZE) {
        size_t unpadded_len = removePKCS7Padding(plaintext, try_len);
        if (unpadded_len > 0) {
            actual_data_len = unpadded_len;
            Serial.print("DEBUG decryptData: Found valid padding at length ");
            Serial.print(try_len);
            Serial.print(", unpadded length = ");
            Serial.println(actual_data_len);
            break;
        }
    }
    
    if (actual_data_len == 0) {
        Serial.println("DEBUG decryptData: No valid PKCS7 padding found");
        return false;
    }
    
    *plaintext_len = actual_data_len;
    return true;
}

bool sha256Hash(const String& input, uint8_t* hash) {
    return sha256Hash((const uint8_t*)input.c_str(), input.length(), hash);
}

bool sha256Hash(const uint8_t* data, size_t len, uint8_t* hash) {
    sha256_hash(data, len, hash);
    return true;
}

size_t addPKCS7Padding(uint8_t* data, size_t data_len, size_t block_size) {
    size_t padding_bytes = block_size - (data_len % block_size);
    
    // IMPORTANT: If data_len is already a multiple of block_size,
    // we still need to add a full block of padding (PKCS7 requirement)
    if (padding_bytes == 0) {
        padding_bytes = block_size;
    }
    
    Serial.print("DEBUG addPKCS7Padding: data_len=");
    Serial.print(data_len);
    Serial.print(", padding_bytes=");
    Serial.println(padding_bytes);
    
    for (size_t i = 0; i < padding_bytes; i++) {
        data[data_len + i] = (uint8_t)padding_bytes;
    }
    
    return data_len + padding_bytes;
}

size_t removePKCS7Padding(const uint8_t* data, size_t data_len) {
    if (data_len == 0) return 0;
    
    uint8_t padding_bytes = data[data_len - 1];
    
    // Validate padding
    if (padding_bytes == 0 || padding_bytes > AES_BLOCK_SIZE) {
        return 0; // Invalid padding
    }
    
    if (data_len < padding_bytes) {
        return 0; // Invalid padding
    }
    
    // Check all padding bytes
    for (size_t i = data_len - padding_bytes; i < data_len; i++) {
        if (data[i] != padding_bytes) {
            return 0; // Invalid padding
        }
    }
    
    return data_len - padding_bytes;
}

bool encryptCredentials(const DeviceCredentials& creds, FRAMCredentials& fram_creds) {
    Serial.println("Encrypting credentials...");
    
    // Clear the structure
    memset(&fram_creds, 0, sizeof(FRAMCredentials));
    
    // Set magic and version
    fram_creds.magic = FRAM_MAGIC_NUMBER;
    fram_creds.version = FRAM_DATA_VERSION;
    
    // Copy device name (plain text)
    strncpy(fram_creds.device_name, creds.device_name.c_str(), 31);
    fram_creds.device_name[31] = '\0';
    
    // Generate encryption key from device name
    uint8_t encryption_key[AES_KEY_SIZE];
    if (!generateEncryptionKey(creds.device_name, encryption_key)) {
        Serial.println("ERROR: Failed to generate encryption key");
        return false;
    }
    
    // Generate random IV
    if (!generateRandomIV(fram_creds.iv)) {
        Serial.println("ERROR: Failed to generate IV");
        return false;
    }
    
    // Hash admin password
    String admin_password = creds.admin_password;
    uint8_t admin_hash[SHA256_HASH_SIZE];
    if (!sha256Hash(admin_password, admin_hash)) {
        Serial.println("ERROR: Failed to hash admin password");
        return false;
    }
    
    // Convert hash to hex string
    String admin_hash_hex = "";
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
        if (admin_hash[i] < 16) admin_hash_hex += "0";
        admin_hash_hex += String(admin_hash[i], HEX);
    }
    
    Serial.print("DEBUG: Admin hash hex string: '");
    Serial.print(admin_hash_hex);
    Serial.println("'");
    Serial.print("DEBUG: Admin hash hex length: ");
    Serial.println(admin_hash_hex.length());
    
    // Encrypt WiFi SSID
    size_t ciphertext_len = 64;
    if (!encryptData((const uint8_t*)creds.wifi_ssid.c_str(), creds.wifi_ssid.length(),
                     encryption_key, fram_creds.iv,
                     fram_creds.encrypted_wifi_ssid, &ciphertext_len)) {
        Serial.println("ERROR: Failed to encrypt WiFi SSID");
        return false;
    }
    
    // Encrypt WiFi password
    ciphertext_len = 128;
    if (!encryptData((const uint8_t*)creds.wifi_password.c_str(), creds.wifi_password.length(),
                     encryption_key, fram_creds.iv,
                     fram_creds.encrypted_wifi_password, &ciphertext_len)) {
        Serial.println("ERROR: Failed to encrypt WiFi password");
        return false;
    }
    
    // Encrypt admin hash
    ciphertext_len = 96;
    if (!encryptData((const uint8_t*)admin_hash_hex.c_str(), admin_hash_hex.length(),
                     encryption_key, fram_creds.iv,
                     fram_creds.encrypted_admin_hash, &ciphertext_len)) {
        Serial.println("ERROR: Failed to encrypt admin hash");
        return false;
    }
    
    // Encrypt VPS token
    ciphertext_len = 160;
    if (!encryptData((const uint8_t*)creds.vps_token.c_str(), creds.vps_token.length(),
                     encryption_key, fram_creds.iv,
                     fram_creds.encrypted_vps_token, &ciphertext_len)) {
        Serial.println("ERROR: Failed to encrypt VPS token");
        return false;
    }
    
    // Calculate checksum (only bytes before checksum field: 0-495)
    size_t checksum_offset = offsetof(FRAMCredentials, checksum);
    uint16_t temp_checksum = calculateChecksum((uint8_t*)&fram_creds, checksum_offset);
    fram_creds.checksum = temp_checksum;
    
    Serial.print("DEBUG: Calculated checksum during encryption = ");
    Serial.println(fram_creds.checksum);
    Serial.print("DEBUG: Checksum calculated over ");
    Serial.print(checksum_offset);
    Serial.println(" bytes (before checksum field)");
    
    Serial.println("SUCCESS: Credentials encrypted");
    return true;
}

bool decryptCredentials(const FRAMCredentials& fram_creds, DeviceCredentials& creds) {
    Serial.println("Decrypting credentials...");
    
    // Generate encryption key from device name
    uint8_t encryption_key[AES_KEY_SIZE];
    String device_name = String(fram_creds.device_name);
    Serial.print("DEBUG: Device name for key generation: '");
    Serial.print(device_name);
    Serial.println("'");
    
    if (!generateEncryptionKey(device_name, encryption_key)) {
        Serial.println("ERROR: Failed to generate encryption key");
        return false;
    }
    
    Serial.print("DEBUG: Generated key (first 8 bytes): ");
    for (int i = 0; i < 8; i++) {
        if (encryption_key[i] < 16) Serial.print("0");
        Serial.print(encryption_key[i], HEX);
    }
    Serial.println();
    
    Serial.print("DEBUG: IV from FRAM: ");
    for (int i = 0; i < AES_IV_SIZE; i++) {
        if (fram_creds.iv[i] < 16) Serial.print("0");
        Serial.print(fram_creds.iv[i], HEX);
    }
    Serial.println();
    
    // Set device name
    creds.device_name = device_name;
    
    // Decrypt WiFi SSID
    Serial.println("DEBUG: Attempting to decrypt WiFi SSID...");
    uint8_t plaintext_buffer[256];
    size_t plaintext_len = sizeof(plaintext_buffer);
    
    if (!decryptData(fram_creds.encrypted_wifi_ssid, 64,
                     encryption_key, fram_creds.iv,
                     plaintext_buffer, &plaintext_len)) {
        Serial.println("ERROR: Failed to decrypt WiFi SSID");
        return false;
    }
    
    plaintext_buffer[plaintext_len] = '\0';
    creds.wifi_ssid = String((char*)plaintext_buffer);
    Serial.print("DEBUG: Decrypted SSID string: '");
    Serial.print(creds.wifi_ssid);
    Serial.println("'");
    
    // Decrypt WiFi password
    Serial.println("DEBUG: Attempting to decrypt WiFi password...");
    plaintext_len = sizeof(plaintext_buffer);
    if (!decryptData(fram_creds.encrypted_wifi_password, 128,
                     encryption_key, fram_creds.iv,
                     plaintext_buffer, &plaintext_len)) {
        Serial.println("ERROR: Failed to decrypt WiFi password");
        return false;
    }
    plaintext_buffer[plaintext_len] = '\0';
    creds.wifi_password = String((char*)plaintext_buffer);
    Serial.print("DEBUG: Decrypted WiFi password string: '");
    Serial.print(creds.wifi_password);
    Serial.println("'");
    
    // Decrypt admin hash (we return the hash, not original password)
    Serial.println("DEBUG: Attempting to decrypt admin hash...");
    Serial.print("DEBUG: Admin hash encrypted size: 96 bytes, first 16 bytes: ");
    for (int i = 0; i < 16; i++) {
        if (fram_creds.encrypted_admin_hash[i] < 16) Serial.print("0");
        Serial.print(fram_creds.encrypted_admin_hash[i], HEX);
    }
    Serial.println();
    
    plaintext_len = sizeof(plaintext_buffer);
    if (!decryptData(fram_creds.encrypted_admin_hash, 96,
                     encryption_key, fram_creds.iv,
                     plaintext_buffer, &plaintext_len)) {
        Serial.println("ERROR: Failed to decrypt admin hash");
        Serial.println("DEBUG: Continuing with other fields...");
        creds.admin_password = ""; // Set empty on failure
    } else {
        plaintext_buffer[plaintext_len] = '\0';
        creds.admin_password = String((char*)plaintext_buffer); // This is actually the hash
        Serial.print("DEBUG: Decrypted admin hash length: ");
        Serial.println(creds.admin_password.length());
        Serial.print("DEBUG: Admin hash (first 20 chars): ");
        Serial.println(creds.admin_password.substring(0, 20));
    }
    
    // Decrypt VPS token
    Serial.println("DEBUG: Attempting to decrypt VPS token...");
    plaintext_len = sizeof(plaintext_buffer);
    if (!decryptData(fram_creds.encrypted_vps_token, 160,
                     encryption_key, fram_creds.iv,
                     plaintext_buffer, &plaintext_len)) {
        Serial.println("ERROR: Failed to decrypt VPS token");
        creds.vps_token = ""; // Set empty on failure
    } else {
        plaintext_buffer[plaintext_len] = '\0';
        creds.vps_token = String((char*)plaintext_buffer);
        Serial.print("DEBUG: Decrypted VPS token string: '");
        Serial.print(creds.vps_token);
        Serial.println("'");
    }
    
    Serial.println("SUCCESS: Credential decryption completed");
    return true;
}

bool validateDeviceName(const String& name) {
    if (name.length() == 0 || name.length() > MAX_DEVICE_NAME_LEN) {
        Serial.print("Device name length invalid (1-");
        Serial.print(MAX_DEVICE_NAME_LEN);
        Serial.println(" characters required)");
        return false;
    }
    
    // Check for valid characters (alphanumeric and underscore)
    for (int i = 0; i < name.length(); i++) {
        char c = name.charAt(i);
        if (!isalnum(c) && c != '_') {
            Serial.println("Device name contains invalid characters (alphanumeric and _ only)");
            return false;
        }
    }
    
    return true;
}

bool validateWiFiSSID(const String& ssid) {
    if (ssid.length() == 0 || ssid.length() > MAX_WIFI_SSID_LEN) {
        Serial.print("WiFi SSID length invalid (1-");
        Serial.print(MAX_WIFI_SSID_LEN);
        Serial.println(" characters required)");
        return false;
    }
    return true;
}

bool validateWiFiPassword(const String& password) {
    if (password.length() == 0 || password.length() > MAX_WIFI_PASSWORD_LEN) {
        Serial.print("WiFi password length invalid (1-");
        Serial.print(MAX_WIFI_PASSWORD_LEN);
        Serial.println(" characters required)");
        return false;
    }
    return true;
}

bool validateVPSToken(const String& token) {
    if (token.length() == 0 || token.length() > MAX_VPS_TOKEN_LEN) {
        Serial.print("VPS token length invalid (1-");
        Serial.print(MAX_VPS_TOKEN_LEN);
        Serial.println(" characters required)");
        return false;
    }
    return true;
}