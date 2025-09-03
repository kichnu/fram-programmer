#include <Arduino.h>
#include <Wire.h>
#include "fram_programmer.h"
#include "cli_handler.h"

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    
    // Wait for serial connection (up to 3 seconds)
    unsigned long start = millis();
    while (!Serial && (millis() - start) < 3000) {
        delay(100);
    }
    
    // Print startup banner
    Serial.println();
    Serial.println("========================================");
    Serial.println("    FRAM Programmer v1.0");
    Serial.println("    Beetle RP2350 ESP32 Credentials");
    Serial.println("========================================");
    Serial.println();
    
    // Initialize I2C with custom pins for RP2350
    Wire.setSDA(SDA_PIN);
    Wire.setSCL(SCL_PIN);
    Wire.begin();
    Wire.setClock(100000); // 100kHz for FRAM compatibility
    
    Serial.print("I2C initialized (SDA=");
    Serial.print(SDA_PIN);
    Serial.print(", SCL=");
    Serial.print(SCL_PIN);
    Serial.println(")");
    
    // Initialize FRAM
    Serial.print("Initializing FRAM... ");
    if (initFRAM()) {
        Serial.println("SUCCESS");
        printFRAMInfo();
    } else {
        Serial.println("FAILED");
        Serial.println("WARNING: FRAM not detected. Some commands may not work.");
    }
    
    // Initialize CLI
    initCLI();
    
    Serial.println();
    Serial.println("Ready! Type 'help' for available commands.");
    printPrompt();
}

void loop() {
    // Handle CLI commands
    handleCLI();
    
    // Small delay to prevent overwhelming the system
    delay(10);
}