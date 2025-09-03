# FRAM Programmer for ESP32 Credentials

**Offline ESP32 credentials programmer using Beetle RP2350 and FRAM memory**

[![PlatformIO CI](https://img.shields.io/badge/PlatformIO-Compatible-orange)](https://platformio.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Hardware](https://img.shields.io/badge/Hardware-RP2350-blue)](https://www.raspberrypi.org/documentation/microcontrollers/rp2040.html)

## Overview

This project provides a secure, offline method to program encrypted ESP32 credentials into FRAM memory. Built for IoT systems requiring secure credential storage without exposing sensitive data in plain text.

### Key Features

- **🔐 AES-256-CBC Encryption** with SHA-256 key derivation
- **📡 I2C FRAM Support** (32KB modules like FM25CL64, MB85RC256V)
- **🖥️ CLI Interface** with interactive and JSON input modes
- **🔧 Hardware-based Programming** using Beetle RP2350
- **✅ Complete Verification** with decryption testing
- **🔄 Backup/Restore** functionality for safe operation
- **🎯 ESP32 Compatible** - works with existing ESP32 FRAM code

## Hardware Requirements

| Component | Description | Notes |
|-----------|-------------|--------|
| **Beetle RP2350** | DFRobot microcontroller | Main programmer |
| **FRAM Module** | 32KB I2C FRAM (0x50) | FM25CL64, MB85RC256V |
| **Connections** | SDA, SCL, VCC, GND | 3.3V, 100kHz I2C |

### Wiring Diagram

```
Beetle RP2350          FRAM Module
=============          ============
3V3            →       VCC (Pin 8)
GND            →       GND (Pin 4)
GPIO 4 (SDA)   →       SDA (Pin 5)
GPIO 5 (SCL)   →       SCL (Pin 6)
               →       WP  (Pin 3) → GND
               →       HOLD(Pin 7) → VCC
```

## Quick Start

### 1. Setup PlatformIO Environment

```bash
git clone https://github.com/yourusername/fram-programmer.git
cd fram-programmer
pio run -t upload
pio device monitor
```

### 2. Basic Usage

```bash
# Interactive programming
FRAM> program

# JSON configuration  
FRAM> config
{"device_name": "DEVICE_001", "wifi_ssid": "Network", ...}

# Verification
FRAM> verify

# Full system test
FRAM> test
```

### 3. Example Session

```
FRAM> program
Device Name: DOLEWKA_001
WiFi SSID: MyNetwork
WiFi Password: SecurePass123
Admin Password: admin123
VPS Token: sha256:abc123def456...

Program these credentials to FRAM? (YES/no): YES
[SUCCESS] Credentials programmed successfully!

FRAM> verify
[SUCCESS] Credentials verification PASSED
=== DECRYPTED CREDENTIALS ===
Device Name: DOLEWKA_001
WiFi SSID: MyNetwork
Admin Hash: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
VPS Token: sha256:abc123def456...
```

## CLI Commands

| Command | Short | Description |
|---------|-------|-------------|
| `help` | `h` | Show available commands |
| `detect` | `d` | Detect FRAM device |
| `info` | `i` | Show FRAM information |
| `program` | `p` | Interactive credential programming |
| `config` | `c` | JSON-based configuration |
| `verify` | `v` | Verify and decrypt credentials |
| `backup` | `b` | Backup entire FRAM content |
| `restore` | `r` | Restore FRAM from backup |
| `test` | `t` | Run diagnostic tests |

## Project Structure

```
fram-programmer/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp            # Main application entry
│   ├── fram_programmer.cpp # FRAM operations
│   ├── encryption.cpp      # AES-256-CBC + SHA-256
│   ├── cli_handler.cpp     # Command-line interface
│   ├── aes.cpp            # AES implementation
│   └── sha256.cpp         # SHA-256 implementation
├── include/
│   ├── fram_programmer.h   # FRAM API definitions
│   ├── encryption.h        # Crypto functions
│   ├── cli_handler.h       # CLI interface
│   ├── aes.h              # AES headers
│   └── sha256.h           # SHA-256 headers
├── docs/
│   └── FRAM_ESP32_Specification.md  # Technical specification
└── examples/
    ├── credentials.json    # Example JSON config
    └── backup_example.txt  # Example backup format
```

## Technical Specifications

### FRAM Layout
- **Address:** 0x0018 (24-byte offset)  
- **Size:** 1024 bytes
- **Structure:** See [FRAM_ESP32_Specification.md](docs/FRAM_ESP32_Specification.md)

### Encryption Details
- **Algorithm:** AES-256-CBC with PKCS#7 padding
- **Key Derivation:** SHA-256(device_name + salt + seed)
- **IV:** 8-byte random per session, extended to 16-byte
- **Integrity:** 16-bit checksum over unencrypted structure

### Security Features
- Device-specific encryption keys
- Offline credential programming
- No plain-text storage on ESP32
- Tamper detection via checksum

## ESP32 Integration

The programmed FRAM can be directly used in ESP32 projects:

```cpp
#include "fram_controller.h"

void setup() {
    if (initFRAM() && verifyFRAM()) {
        // Load credentials from FRAM
        loadCredentialsFromFRAM();
        Serial.println("Credentials loaded successfully");
    }
}
```

See [FRAM_ESP32_Specification.md](docs/FRAM_ESP32_Specification.md) for complete integration details.

## Dependencies

- **PlatformIO Core** 6.0+
- **Adafruit FRAM I2C** ^2.0.3
- **ArduinoJson** ^6.21.3
- **Custom crypto libraries** (included)

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## Testing

Run the built-in test suite:

```bash
FRAM> test
=== FRAM Test Sequence ===
Test 0: Structure Alignment Check - PASS
Test 1: Basic Read/Write - PASS  
Test 2: Checksum Function - PASS
Test 3: Encryption/Decryption - PASS
=== TEST SUMMARY: ALL TESTS PASSED ===
```

## Troubleshooting

### Common Issues

**FRAM not detected:**
- Check I2C connections (SDA=GPIO4, SCL=GPIO5)
- Verify 3.3V power supply
- Ensure WP pin connected to GND

**Programming fails:**
- Run `backup` before programming
- Check input data validation
- Verify FRAM space availability

**Verification errors:**
- Confirm correct device_name
- Test with `info` command
- Check for I2C interference

See [FRAM_ESP32_Specification.md](docs/FRAM_ESP32_Specification.md) for detailed troubleshooting.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history and updates.

## Support

- **Documentation:** [docs/](docs/)
- **Issues:** [GitHub Issues](https://github.com/yourusername/fram-programmer/issues)
- **Technical Spec:** [FRAM_ESP32_Specification.md](docs/FRAM_ESP32_Specification.md)

## Acknowledgments

- Built for ESP32 IoT water management systems
- Compatible with DFRobot Beetle RP2350
- Tested with Adafruit FRAM modules

---

**Status:** Production Ready  
**Tested Hardware:** Beetle RP2350 + FM25CL64/MB85RC256V  
**ESP32 Compatibility:** Verified