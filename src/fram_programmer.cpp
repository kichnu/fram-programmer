#include "fram_programmer.h"
#include "encryption.h"
#include <Wire.h>
#include <Adafruit_FRAM_I2C.h>
#include <stddef.h>
// Global FRAM object
Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();

bool initFRAM() {
    Serial.print("Scanning I2C bus for FRAM at 0x");
    Serial.print(FRAM_I2C_ADDR, HEX);
    Serial.print("... ");
    
    // Initialize FRAM with specified address
    if (!fram.begin(FRAM_I2C_ADDR)) {
        Serial.println("NOT FOUND");
        return false;
    }
    
    Serial.println("FOUND");
    
    // Verify FRAM is working by reading/writing test pattern
    uint8_t test_data = 0xAA;
    uint8_t read_data = 0x00;
    
    // Write test byte to a safe location (near end of FRAM)
    fram.write(0x7FFE, &test_data, 1);
    fram.read(0x7FFE, &read_data, 1);
    
    if (read_data != test_data) {
        Serial.println("FRAM verification failed");
        return false;
    }
    
    // Clear test location
    test_data = 0x00;
    fram.write(0x7FFE, &test_data, 1);
    
    return true;
}

bool detectFRAM() {
    Wire.beginTransmission(FRAM_I2C_ADDR);
    uint8_t error = Wire.endTransmission();
    return (error == 0);
}

bool backupFRAM() {
    Serial.println("Starting FRAM backup...");
    
    if (!detectFRAM()) {
        Serial.println("ERROR: FRAM not detected");
        return false;
    }
    
    // Read entire FRAM content (32KB = 32768 bytes)
    const size_t fram_size = 32768;
    const size_t chunk_size = 64;  // Read in 64-byte chunks
    
    Serial.println("BACKUP_START");
    Serial.print("SIZE:");
    Serial.println(fram_size);
    
    for (size_t addr = 0; addr < fram_size; addr += chunk_size) {
        uint8_t buffer[chunk_size];
        size_t read_size = min(chunk_size, fram_size - addr);
        
        fram.read(addr, buffer, read_size);
        
        // Send address
        Serial.print("ADDR:");
        Serial.println(addr, HEX);
        
        // Send data as hex
        Serial.print("DATA:");
        for (size_t i = 0; i < read_size; i++) {
            if (buffer[i] < 16) Serial.print("0");
            Serial.print(buffer[i], HEX);
        }
        Serial.println();
        
        delay(10); // Small delay for serial processing
    }
    
    Serial.println("BACKUP_END");
    return true;
}

bool restoreFRAM(const uint8_t* backup_data, size_t data_size) {
    Serial.println("Starting FRAM restore...");
    
    if (!detectFRAM()) {
        Serial.println("ERROR: FRAM not detected");
        return false;
    }
    
    if (data_size > 32768) {
        Serial.println("ERROR: Backup data too large");
        return false;
    }
    
    // Write data in chunks
    const size_t chunk_size = 32;
    
    for (size_t addr = 0; addr < data_size; addr += chunk_size) {
        size_t write_size = min(chunk_size, data_size - addr);
        
        // Cast away const for fram.write (it doesn't modify the data)
        fram.write(addr, (uint8_t*)&backup_data[addr], write_size);
        
        // Verify written data
        uint8_t verify_buffer[chunk_size];
        fram.read(addr, verify_buffer, write_size);
        
        if (memcmp(&backup_data[addr], verify_buffer, write_size) != 0) {
            Serial.print("ERROR: Verification failed at address 0x");
            Serial.println(addr, HEX);
            return false;
        }
        
        // Progress indicator
        if (addr % 1024 == 0) {
            Serial.print(".");
        }
    }
    
    Serial.println();
    Serial.println("FRAM restore completed successfully");
    return true;
}

bool readCredentialsSection(FRAMCredentials& creds) {
    if (!detectFRAM()) {
        Serial.println("ERROR: FRAM not detected");
        return false;
    }
    
    // Read credentials structure from FRAM
    fram.read(FRAM_CREDENTIALS_ADDR, (uint8_t*)&creds, sizeof(FRAMCredentials));
    
    return true;
}

bool writeCredentialsSection(const FRAMCredentials& creds) {
    if (!detectFRAM()) {
        Serial.println("ERROR: FRAM not detected");
        return false;
    }
    
    // Debug: show what we're writing
    Serial.println("DEBUG: Writing structure to FRAM:");
    const uint8_t* raw = (const uint8_t*)&creds;
    Serial.println("First 50 bytes being written:");
    for (int i = 0; i < 50; i++) {
        if (i % 16 == 0) Serial.print("  ");
        if (raw[i] < 16) Serial.print("0");
        Serial.print(raw[i], HEX);
        Serial.print(" ");
        if ((i + 1) % 16 == 0) Serial.println();
    }
    Serial.println();
    
    Serial.print("DEBUG: Checksum being written: ");
    Serial.println(creds.checksum);
    Serial.print("DEBUG: Checksum field bytes: 0x");
    Serial.print(raw[497], HEX);
    Serial.println(raw[496], HEX);
    
    // Write credentials structure to FRAM
    fram.write(FRAM_CREDENTIALS_ADDR, (uint8_t*)&creds, sizeof(FRAMCredentials));
    
    // Verify write by reading back
    FRAMCredentials verify_creds;
    fram.read(FRAM_CREDENTIALS_ADDR, (uint8_t*)&verify_creds, sizeof(FRAMCredentials));
    
    // Compare written data
    if (memcmp(&creds, &verify_creds, sizeof(FRAMCredentials)) != 0) {
        Serial.println("ERROR: FRAM write verification failed!");
        
        // Debug: find where they differ
        const uint8_t* written = (const uint8_t*)&creds;
        const uint8_t* read = (const uint8_t*)&verify_creds;
        
        Serial.println("DEBUG: Differences found:");
        for (size_t i = 0; i < sizeof(FRAMCredentials); i++) {
            if (written[i] != read[i]) {
                Serial.print("  Byte ");
                Serial.print(i);
                Serial.print(": written=0x");
                if (written[i] < 16) Serial.print("0");
                Serial.print(written[i], HEX);
                Serial.print(", read=0x");
                if (read[i] < 16) Serial.print("0");
                Serial.println(read[i], HEX);
            }
        }
        return false;
    }
    
    Serial.println("DEBUG: FRAM write verification passed");
    return true;
}

bool programCredentials(const DeviceCredentials& creds) {
    Serial.println("Programming credentials to FRAM...");
    
    // Validate input credentials
    if (!validateDeviceName(creds.device_name)) {
        Serial.println("ERROR: Invalid device name");
        return false;
    }
    
    if (!validateWiFiSSID(creds.wifi_ssid)) {
        Serial.println("ERROR: Invalid WiFi SSID");
        return false;
    }
    
    if (!validateWiFiPassword(creds.wifi_password)) {
        Serial.println("ERROR: Invalid WiFi password");
        return false;
    }
    
    if (!validateVPSToken(creds.vps_token)) {
        Serial.println("ERROR: Invalid VPS token");
        return false;
    }
    
    // Read current FRAM content to preserve other data
    Serial.println("Backing up existing FRAM content...");
    uint8_t backup_before[FRAM_CREDENTIALS_SIZE];
    fram.read(FRAM_CREDENTIALS_ADDR, backup_before, FRAM_CREDENTIALS_SIZE);
    
    // Create and encrypt credentials structure
    FRAMCredentials fram_creds;
    if (!encryptCredentials(creds, fram_creds)) {
        Serial.println("ERROR: Credential encryption failed");
        return false;
    }
    
    // Write to FRAM
    Serial.println("Writing encrypted credentials to FRAM...");
    if (!writeCredentialsSection(fram_creds)) {
        // Restore backup on failure
        Serial.println("FAILED: Restoring backup...");
        fram.write(FRAM_CREDENTIALS_ADDR, backup_before, FRAM_CREDENTIALS_SIZE);
        return false;
    }
    
    Serial.println("SUCCESS: Credentials programmed to FRAM");
    
    // Verify by reading back and checking magic/checksum
    return verifyCredentials();
}

bool verifyCredentials() {
    Serial.println("Verifying FRAM credentials...");
    
    FRAMCredentials creds;
    if (!readCredentialsSection(creds)) {
        return false;
    }
    
    // Debug: pokaż podstawowe informacje
    Serial.print("DEBUG: Magic = 0x");
    Serial.println(creds.magic, HEX);
    Serial.print("DEBUG: Version = ");
    Serial.println(creds.version);
    Serial.print("DEBUG: Device name = '");
    Serial.print(creds.device_name);
    Serial.println("'");
    Serial.print("DEBUG: Structure size = ");
    Serial.println(sizeof(FRAMCredentials));
    
    // Debug: show some key structure bytes
    uint8_t* raw = (uint8_t*)&creds;
    Serial.println("DEBUG: First 50 bytes of structure:");
    for (int i = 0; i < 50; i++) {
        if (i % 16 == 0) Serial.print("  ");
        if (raw[i] < 16) Serial.print("0");
        Serial.print(raw[i], HEX);
        Serial.print(" ");
        if ((i + 1) % 16 == 0) Serial.println();
    }
    Serial.println();
    
    // Check magic number
    if (creds.magic != FRAM_MAGIC_NUMBER) {
        Serial.print("ERROR: Invalid magic number: 0x");
        Serial.print(creds.magic, HEX);
        Serial.print(", expected: 0x");
        Serial.println(FRAM_MAGIC_NUMBER, HEX);
        return false;
    }
    
    // Check version
    if (creds.version != FRAM_DATA_VERSION) {
        Serial.print("ERROR: Invalid version: ");
        Serial.print(creds.version);
        Serial.print(", expected: ");
        Serial.println(FRAM_DATA_VERSION);
        return false;
    }
    
    // Debug checksum calculation
    uint16_t stored_checksum = creds.checksum;
    
    // Calculate checksum only over bytes BEFORE checksum field (0 to checksum_offset-1)
    size_t checksum_offset = offsetof(FRAMCredentials, checksum);
    uint16_t calculated_checksum = calculateChecksum((uint8_t*)&creds, checksum_offset);
    
    Serial.print("DEBUG: Checksum field at offset ");
    Serial.println(checksum_offset);
    Serial.print("DEBUG: Calculating checksum over ");
    Serial.print(checksum_offset);
    Serial.println(" bytes (before checksum field)");
    Serial.print("DEBUG: Stored checksum = ");
    Serial.println(stored_checksum);
    Serial.print("DEBUG: Calculated checksum = ");
    Serial.println(calculated_checksum);
    Serial.print("DEBUG: Difference = ");
    Serial.println(abs(stored_checksum - calculated_checksum));
    
    if (stored_checksum != calculated_checksum) {
        Serial.print("ERROR: Checksum mismatch - stored: ");
        Serial.print(stored_checksum);
        Serial.print(", calculated: ");
        Serial.println(calculated_checksum);
        
        // Debug: show bytes around checksum field
        Serial.println("DEBUG: Bytes around checksum field (490-505):");
        for (int i = 490; i < 506; i++) {
            Serial.print("  [");
            Serial.print(i);
            Serial.print("] = 0x");
            if (raw[i] < 16) Serial.print("0");
            Serial.println(raw[i], HEX);
        }
        
        return false;
    }
    
    Serial.println("SUCCESS: Credentials verification passed");
    return true;
}

uint16_t calculateChecksum(const uint8_t* data, size_t size) {
    uint16_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

void printFRAMInfo() {
    Serial.println();
    Serial.println("FRAM Information:");
    Serial.print("  I2C Address: 0x");
    Serial.println(FRAM_I2C_ADDR, HEX);
    Serial.print("  Credentials Address: 0x");
    Serial.println(FRAM_CREDENTIALS_ADDR, HEX);
    Serial.print("  Credentials Size: ");
    Serial.print(FRAM_CREDENTIALS_SIZE);
    Serial.println(" bytes");
    
    // Check if credentials are present
    FRAMCredentials creds;
    if (readCredentialsSection(creds)) {
        Serial.print("  Magic Number: 0x");
        Serial.println(creds.magic, HEX);
        
        if (creds.magic == FRAM_MAGIC_NUMBER) {
            Serial.println("  Status: CREDENTIALS PRESENT");
            printCredentialsInfo(creds);
        } else {
            Serial.println("  Status: NO VALID CREDENTIALS");
        }
    }
}

void printCredentialsInfo(const FRAMCredentials& creds) {
    Serial.println("  Credential Details:");
    Serial.print("    Version: ");
    Serial.println(creds.version);
    Serial.print("    Device Name: ");
    Serial.println(creds.device_name);
    Serial.print("    Checksum: 0x");
    Serial.println(creds.checksum, HEX);
    
    // Show encryption info
    Serial.print("    IV: ");
    for (int i = 0; i < AES_IV_SIZE; i++) {
        if (creds.iv[i] < 16) Serial.print("0");
        Serial.print(creds.iv[i], HEX);
    }
    Serial.println();
    
    Serial.println("    Encrypted Data Present:");
    Serial.println("      - WiFi SSID: YES");
    Serial.println("      - WiFi Password: YES");
    Serial.println("      - Admin Hash: YES");
    Serial.println("      - VPS Token: YES");
}