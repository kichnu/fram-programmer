#include "cli_handler.h"
#include "fram_programmer.h"
#include "encryption.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_FRAM_I2C.h>

// CLI state
static String inputBuffer = "";
static bool waitingForInput = false;

void initCLI() {
    Serial.println("CLI initialized");
    inputBuffer.reserve(1024); // Reserve space for input buffer
}

void handleCLI() {
    if (Serial.available()) {
        char c = Serial.read();
        
        // Debug: pokaż otrzymany znak
        Serial.print("Got char: '");
        Serial.print(c);
        Serial.print("' (");
        Serial.print((int)c);
        Serial.println(")");
        
        if (c == '\n' || c == '\r') {
            if (inputBuffer.length() > 0) {
                // Process command
                inputBuffer.trim();
                
                Serial.print("Processing command: '");
                Serial.print(inputBuffer);
                Serial.println("'");
                
                CLICommand cmd = parseCommand(inputBuffer);
                executeCommand(cmd, inputBuffer);
                
                inputBuffer = "";
                if (!waitingForInput) {
                    printPrompt();
                }
            }
        } else if (c == '\b' || c == 127) { // Backspace
            if (inputBuffer.length() > 0) {
                inputBuffer.remove(inputBuffer.length() - 1);
                Serial.print("\b \b");
            }
        } else if (c >= 32 && c < 127) { // Printable characters
            inputBuffer += c;
            Serial.print(c);
        }
    }
}

CLICommand parseCommand(const String& input) {
    String cmd = input;
    int spaceIndex = cmd.indexOf(' ');
    if (spaceIndex > 0) {
        cmd = cmd.substring(0, spaceIndex);
    }
    cmd.toLowerCase();
    
    if (cmd == "help" || cmd == "h") return CMD_HELP;
    if (cmd == "detect" || cmd == "d") return CMD_DETECT;
    if (cmd == "info" || cmd == "i") return CMD_INFO;
    if (cmd == "backup" || cmd == "b") return CMD_BACKUP;
    if (cmd == "restore" || cmd == "r") return CMD_RESTORE;
    if (cmd == "program" || cmd == "p") return CMD_PROGRAM;
    if (cmd == "verify" || cmd == "v") return CMD_VERIFY;
    if (cmd == "config" || cmd == "c") return CMD_CONFIG;
    if (cmd == "test" || cmd == "t") return CMD_TEST;
    
    return CMD_UNKNOWN;
}

void executeCommand(CLICommand cmd, const String& args) {
    Serial.println(); // New line after command
    
    switch (cmd) {
        case CMD_HELP:      cmdHelp(); break;
        case CMD_DETECT:    cmdDetect(); break;
        case CMD_INFO:      cmdInfo(); break;
        case CMD_BACKUP:    cmdBackup(); break;
        case CMD_RESTORE:   cmdRestore(); break;
        case CMD_PROGRAM:   cmdProgram(); break;
        case CMD_VERIFY:    cmdVerify(); break;
        case CMD_CONFIG:    cmdConfig(); break;
        case CMD_TEST:      cmdTest(); break;
        case CMD_UNKNOWN:
        default:
            printError("Unknown command. Type 'help' for available commands.");
            break;
    }
}

void cmdHelp() {
    Serial.println("FRAM Programmer Commands:");
    Serial.println("========================");
    Serial.println("  help (h)     - Show this help");
    Serial.println("  detect (d)   - Detect FRAM device");
    Serial.println("  info (i)     - Show FRAM information");
    Serial.println("  backup (b)   - Backup entire FRAM content");
    Serial.println("  restore (r)  - Restore FRAM from backup");
    Serial.println("  program (p)  - Program credentials to FRAM");
    Serial.println("  verify (v)   - Verify stored credentials");
    Serial.println("  config (c)   - Configure via JSON input");
    Serial.println("  test (t)     - Test FRAM read/write");
    Serial.println();
    Serial.println("Examples:");
    Serial.println("  program      - Interactive credential input");
    Serial.println("  config       - JSON configuration mode");
    Serial.println("  backup       - Creates hex dump for external storage");
}

void cmdDetect() {
    Serial.print("Scanning for FRAM at I2C address 0x");
    Serial.print(FRAM_I2C_ADDR, HEX);
    Serial.print("... ");
    
    if (detectFRAM()) {
        printSuccess("FRAM detected successfully");
    } else {
        printError("FRAM not found");
        
        // I2C scan for any devices  
        Serial.println("Scanning I2C bus for any devices:");
        bool found = false;
        
        for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
            Wire.beginTransmission(addr);
            uint8_t error = Wire.endTransmission();
            
            if (error == 0) {
                Serial.print("  Device found at address 0x");
                if (addr < 16) Serial.print("0");
                Serial.println(addr, HEX);
                found = true;
            }
        }
        
        if (!found) {
            Serial.println("  No I2C devices found!");
        }
    }
}

void cmdInfo() {
    printFRAMInfo();
}

void cmdBackup() {
    printInfo("Starting FRAM backup (output as hex dump)");
    Serial.println("Copy the following output to save your backup:");
    Serial.println();
    
    backupFRAM();
    
    printInfo("Backup complete. Save the hex dump above for restore.");
}

void cmdRestore() {
    printWarning("FRAM restore will overwrite ALL data!");
    Serial.print("Type 'YES' to confirm: ");
    
    waitingForInput = true;
    String confirmation = readSerialLine();
    waitingForInput = false;
    
    if (confirmation != "YES") {
        printInfo("Restore cancelled");
        return;
    }
    
    printInfo("Paste hex backup data below (end with 'END'):");
    
    // Simple hex restore - for demo purposes
    // Real implementation would parse the full backup format
    printWarning("Hex restore not implemented - use backup format");
    printInfo("Restore cancelled");
}

void cmdProgram() {
    printInfo("=== Interactive Credential Programming ===");
    
    DeviceCredentials creds;
    
    // Get device name
    Serial.print("Device Name (1-31 chars, alphanumeric + _): ");
    creds.device_name = readSerialLine();
    
    if (!validateDeviceName(creds.device_name)) {
        printError("Invalid device name");
        return;
    }
    
    // Get WiFi SSID
    Serial.print("WiFi SSID (1-63 chars): ");
    creds.wifi_ssid = readSerialLine();
    
    if (!validateWiFiSSID(creds.wifi_ssid)) {
        printError("Invalid WiFi SSID");
        return;
    }
    
    // Get WiFi password
    Serial.print("WiFi Password (1-127 chars): ");
    creds.wifi_password = readSerialLine();
    
    if (!validateWiFiPassword(creds.wifi_password)) {
        printError("Invalid WiFi password");
        return;
    }
    
    // Get admin password
    Serial.print("Admin Password: ");
    creds.admin_password = readSerialLine();
    
    if (creds.admin_password.length() == 0) {
        printError("Admin password cannot be empty");
        return;
    }
    
    // Get VPS token
    Serial.print("VPS Token: ");
    creds.vps_token = readSerialLine();
    
    if (!validateVPSToken(creds.vps_token)) {
        printError("Invalid VPS token");
        return;
    }
    
    // Confirm programming
    Serial.println();
    Serial.println("=== CREDENTIALS SUMMARY ===");
    Serial.print("Device Name: "); Serial.println(creds.device_name);
    Serial.print("WiFi SSID: "); Serial.println(creds.wifi_ssid);
    Serial.print("WiFi Password: "); Serial.println("******* (hidden)");
    Serial.print("Admin Password: "); Serial.println("******* (will be hashed)");
    Serial.print("VPS Token: "); Serial.println(creds.vps_token);
    Serial.println();
    
    Serial.print("Program these credentials to FRAM? (YES/no): ");
    String confirm = readSerialLine();
    
    if (confirm == "YES" || confirm == "yes" || confirm == "y" || confirm == "") {
        if (programCredentials(creds)) {
            printSuccess("Credentials programmed successfully!");
        } else {
            printError("Failed to program credentials");
        }
    } else {
        printInfo("Programming cancelled");
    }
}

void cmdVerify() {
    printInfo("Verifying FRAM credentials...");
    
    if (verifyCredentials()) {
        printSuccess("Credentials verification PASSED");
        
        // Try to decrypt and show info
        FRAMCredentials fram_creds;
        if (readCredentialsSection(fram_creds)) {
            DeviceCredentials creds;
            if (decryptCredentials(fram_creds, creds)) {
                Serial.println();
                Serial.println("=== DECRYPTED CREDENTIALS ===");
                Serial.print("Device Name: "); Serial.println(creds.device_name);
                Serial.print("WiFi SSID: "); Serial.println(creds.wifi_ssid);
                Serial.print("WiFi Password: "); Serial.println("******* (hidden)");
                Serial.print("Admin Hash: "); Serial.println(creds.admin_password); // This is the hash
                Serial.print("VPS Token: "); Serial.println(creds.vps_token);
            } else {
                printWarning("Could not decrypt credentials (incorrect key?)");
            }
        }
    } else {
        printError("Credentials verification FAILED");
    }
}

void cmdConfig() {
    printInfo("=== JSON Configuration Mode ===");
    Serial.println("Paste JSON configuration below (single line):");
    Serial.println("Format:");
    Serial.println("{");
    Serial.println("  \"device_name\": \"DOLEWKA_001\",");
    Serial.println("  \"wifi_ssid\": \"MyNetwork\",");
    Serial.println("  \"wifi_password\": \"MyPassword\",");
    Serial.println("  \"admin_password\": \"admin123\",");
    Serial.println("  \"vps_token\": \"sha256:abc123...\"");
    Serial.println("}");
    Serial.println();
    Serial.print("JSON: ");
    
    String jsonInput = readSerialLine();
    
    DeviceCredentials creds;
    if (parseJSONCredentials(jsonInput, creds)) {
        Serial.println();
        Serial.println("=== PARSED CREDENTIALS ===");
        Serial.print("Device Name: "); Serial.println(creds.device_name);
        Serial.print("WiFi SSID: "); Serial.println(creds.wifi_ssid);
        Serial.print("WiFi Password: "); Serial.println("******* (hidden)");
        Serial.print("Admin Password: "); Serial.println("******* (will be hashed)");
        Serial.print("VPS Token: "); Serial.println(creds.vps_token);
        Serial.println();
        
        Serial.print("Program these credentials? (YES/no): ");
        String confirm = readSerialLine();
        
        if (confirm == "YES" || confirm == "yes" || confirm == "y" || confirm == "") {
            if (programCredentials(creds)) {
                printSuccess("JSON credentials programmed successfully!");
            } else {
                printError("Failed to program JSON credentials");
            }
        } else {
            printInfo("Programming cancelled");
        }
    } else {
        printError("Invalid JSON format");
    }
}

void cmdTest() {
    printInfo("=== FRAM Test Sequence ===");
    
    if (!detectFRAM()) {
        printError("FRAM not detected - cannot run tests");
        return;
    }
    
    // Test 0: Structure alignment
    Serial.println("Test 0: Structure Alignment Check");
    size_t expected_size = 1024;
    size_t actual_size = sizeof(FRAMCredentials);
    Serial.print("  Expected: "); Serial.print(expected_size); Serial.println(" bytes");
    Serial.print("  Actual: "); Serial.print(actual_size); Serial.println(" bytes");
    
    // Check field offsets
    FRAMCredentials test_struct;
    Serial.print("  magic offset: "); Serial.println((size_t)&test_struct.magic - (size_t)&test_struct);
    Serial.print("  version offset: "); Serial.println((size_t)&test_struct.version - (size_t)&test_struct);
    Serial.print("  device_name offset: "); Serial.println((size_t)&test_struct.device_name - (size_t)&test_struct);
    Serial.print("  iv offset: "); Serial.println((size_t)&test_struct.iv - (size_t)&test_struct);
    Serial.print("  checksum offset: "); Serial.println((size_t)&test_struct.checksum - (size_t)&test_struct);
    
    bool test0_pass = (actual_size == expected_size);
    Serial.print("  Result: ");
    if (test0_pass) {
        printSuccess("PASS");
    } else {
        printError("FAIL - Structure size mismatch!");
    }
    
    // Test 1: Basic read/write
    Serial.println("Test 1: Basic Read/Write");
    uint8_t test_data[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                             0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8_t read_data[16];
    
    fram.write(0x7000, test_data, 16); // Safe test area
    fram.read(0x7000, read_data, 16);
    
    bool test1_pass = (memcmp(test_data, read_data, 16) == 0);
    Serial.print("  Result: ");
    if (test1_pass) {
        printSuccess("PASS");
    } else {
        printError("FAIL");
    }
    
    // Test 2: Checksum function
    Serial.println("Test 2: Checksum Function");
    uint8_t test_checksum_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    uint16_t checksum1 = calculateChecksum(test_checksum_data, 10);
    uint16_t checksum2 = calculateChecksum(test_checksum_data, 10);
    uint16_t expected_checksum = 0x01 + 0x02 + 0x03 + 0x04 + 0x05 + 0x06 + 0x07 + 0x08 + 0x09 + 0x0A; // = 55
    
    Serial.print("  Expected checksum: "); Serial.println(expected_checksum);
    Serial.print("  Calculated checksum 1: "); Serial.println(checksum1);
    Serial.print("  Calculated checksum 2: "); Serial.println(checksum2);
    
    bool test2_pass = (checksum1 == checksum2 && checksum1 == expected_checksum);
    Serial.print("  Result: ");
    if (test2_pass) {
        printSuccess("PASS");
    } else {
        printError("FAIL");
    }
    
    // Test 3: Encryption/Decryption
    Serial.println("Test 3: Encryption/Decryption");
    String test_string = "Hello, FRAM!";
    uint8_t key[32] = {0}; // Test key
    uint8_t iv[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    
    uint8_t encrypted[64];
    size_t enc_len = 64;
    bool enc_ok = encryptData((uint8_t*)test_string.c_str(), test_string.length(),
                              key, iv, encrypted, &enc_len);
    
    uint8_t decrypted[64];
    size_t dec_len = 64;
    bool dec_ok = decryptData(encrypted, enc_len, key, iv, decrypted, &dec_len);
    
    decrypted[dec_len] = '\0';
    String decrypted_string = String((char*)decrypted);
    
    bool test3_pass = (enc_ok && dec_ok && decrypted_string == test_string);
    Serial.print("  Original: "); Serial.println(test_string);
    Serial.print("  Decrypted: "); Serial.println(decrypted_string);
    Serial.print("  Result: ");
    if (test3_pass) {
        printSuccess("PASS");
    } else {
        printError("FAIL");
    }
    
    // Summary
    Serial.println();
    Serial.print("=== TEST SUMMARY: ");
    if (test0_pass && test1_pass && test2_pass && test3_pass) {
        printSuccess("ALL TESTS PASSED");
    } else {
        printError("SOME TESTS FAILED");
    }
}

bool parseJSONCredentials(const String& json, DeviceCredentials& creds) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.print("JSON parsing error: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Extract fields
    if (!doc.containsKey("device_name") || !doc.containsKey("wifi_ssid") ||
        !doc.containsKey("wifi_password") || !doc.containsKey("admin_password") ||
        !doc.containsKey("vps_token")) {
        Serial.println("Missing required JSON fields");
        return false;
    }
    
    creds.device_name = doc["device_name"].as<String>();
    creds.wifi_ssid = doc["wifi_ssid"].as<String>();
    creds.wifi_password = doc["wifi_password"].as<String>();
    creds.admin_password = doc["admin_password"].as<String>();
    creds.vps_token = doc["vps_token"].as<String>();
    
    // Validate all fields
    return validateDeviceName(creds.device_name) &&
           validateWiFiSSID(creds.wifi_ssid) &&
           validateWiFiPassword(creds.wifi_password) &&
           creds.admin_password.length() > 0 &&
           validateVPSToken(creds.vps_token);
}

String readSerialLine() {
    waitingForInput = true;
    String input = "";
    
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            
            if (c == '\n' || c == '\r') {
                if (input.length() > 0) {
                    Serial.println();
                    waitingForInput = false;
                    return input;
                }
            } else if (c == '\b' || c == 127) { // Backspace
                if (input.length() > 0) {
                    input.remove(input.length() - 1);
                    Serial.print("\b \b");
                }
            } else if (c >= 32 && c < 127) { // Printable characters
                input += c;
                Serial.print(c);
            }
        }
        delay(1);
    }
}

void printPrompt() {
    Serial.print("FRAM> ");
}

void printSuccess(const String& message) {
    Serial.print("[SUCCESS] ");
    Serial.println(message);
}

void printError(const String& message) {
    Serial.print("[ERROR] ");
    Serial.println(message);
}

void printWarning(const String& message) {
    Serial.print("[WARNING] ");
    Serial.println(message);
}

void printInfo(const String& message) {
    Serial.print("[INFO] ");
    Serial.println(message);
}

void printHexDump(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (i % 16 == 0) {
            char addr_str[16];
            sprintf(addr_str, "%08X: ", (unsigned int)i);
            Serial.print(addr_str);
        }
        
        char hex_str[4];
        sprintf(hex_str, "%02X ", data[i]);
        Serial.print(hex_str);
        
        if ((i + 1) % 16 == 0 || i == length - 1) {
            // Print ASCII representation
            Serial.print(" |");
            size_t start = i - (i % 16);
            for (size_t j = start; j <= i; j++) {
                char c = (data[j] >= 32 && data[j] < 127) ? data[j] : '.';
                Serial.print(c);
            }
            Serial.println("|");
        }
    }
}