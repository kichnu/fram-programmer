#ifndef FRAM_PROGRAMMER_H
#define FRAM_PROGRAMMER_H

#include <Arduino.h>

// Forward declaration instead of full include for IntelliSense
class Adafruit_FRAM_I2C;

// FRAM Configuration
#define FRAM_I2C_ADDR           0x50
#define FRAM_CREDENTIALS_ADDR   0x0018      // Start address for credentials (24 bytes offset)
#define FRAM_CREDENTIALS_SIZE   1024        // Size of credentials section

// FRAM Structure Constants
#define FRAM_MAGIC_NUMBER       0x43524544  // "CRED" in hex
#define FRAM_DATA_VERSION       0x0001      // Version 1

// Pin definitions for Beetle RP2350
#define SDA_PIN                 4
#define SCL_PIN                 5

// Input validation limits
#define MAX_DEVICE_NAME_LEN     31
#define MAX_WIFI_SSID_LEN       63
#define MAX_WIFI_PASSWORD_LEN   127
#define MAX_VPS_TOKEN_LEN       255

// Encryption constants
#define AES_BLOCK_SIZE          16
#define AES_KEY_SIZE            32          // 256 bits
#define AES_IV_SIZE             8           // 64 bits
#define SHA256_HASH_SIZE        32          // 256 bits

// Salt for key generation
#define ENCRYPTION_SALT         "ESP32_WATER_SYSTEM_2024_SECURE_SALT_V1"
#define ENCRYPTION_SEED         "WATER_DOLEWKA_FIXED_SEED_12345"

// FRAM Credentials Structure (1024 bytes total)
struct __attribute__((packed)) FRAMCredentials {
    uint32_t magic;                         // 4 bytes  (0-3)
    uint16_t version;                       // 2 bytes  (4-5) 
    uint8_t  reserved_header[2];            // 2 bytes  (6-7)
    char     device_name[32];               // 32 bytes (8-39)
    uint8_t  iv[8];                        // 8 bytes  (40-47)
    uint8_t  encrypted_wifi_ssid[64];      // 64 bytes (48-111)
    uint8_t  encrypted_wifi_password[128]; // 128 bytes (112-239)  
    uint8_t  encrypted_admin_hash[96];     // 96 bytes (240-335)
    uint8_t  encrypted_vps_token[160];     // 160 bytes (336-495)
    uint16_t checksum;                     // 2 bytes  (496-497)
    uint8_t  reserved_footer[14];          // 14 bytes (498-511)
    uint8_t  expansion[512];               // 512 bytes (512-1023) = 1024 total
};

// Input data structure
struct DeviceCredentials {
    String device_name;
    String wifi_ssid;
    String wifi_password;
    String admin_password;
    String vps_token;
};

// Function declarations
bool initFRAM();
bool detectFRAM();
bool backupFRAM();
bool restoreFRAM(const uint8_t* backup_data, size_t data_size);
bool programCredentials(const DeviceCredentials& creds);
bool verifyCredentials();
bool readCredentialsSection(FRAMCredentials& creds);
bool writeCredentialsSection(const FRAMCredentials& creds);
void printFRAMInfo();
void printCredentialsInfo(const FRAMCredentials& creds);
uint16_t calculateChecksum(const uint8_t* data, size_t size);

// Global FRAM object declaration
extern Adafruit_FRAM_I2C fram;

#endif // FRAM_PROGRAMMER_H