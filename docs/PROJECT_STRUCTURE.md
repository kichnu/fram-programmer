# Project Structure Summary

This document provides a complete overview of all files needed for the FRAM Programmer GitHub repository.

## Core Project Files

### Root Directory
```
fram-programmer/
├── README.md                    # Main project documentation
├── LICENSE                      # MIT License
├── .gitignore                   # Git ignore rules
├── CHANGELOG.md                 # Version history
├── CONTRIBUTING.md              # Contribution guidelines
├── PROJECT_STRUCTURE.md         # This file
└── platformio.ini               # PlatformIO configuration
```

### Source Code
```
src/
├── main.cpp                     # Application entry point
├── fram_programmer.cpp          # FRAM operations and I2C handling
├── encryption.cpp               # AES-256-CBC + SHA-256 + validation
├── cli_handler.cpp              # Command-line interface
├── aes.cpp                      # AES-256 implementation
└── sha256.cpp                   # SHA-256 implementation
```

### Header Files
```
include/
├── fram_programmer.h            # FRAM API and structure definitions
├── encryption.h                 # Cryptographic function declarations
├── cli_handler.h                # CLI interface definitions
├── aes.h                        # AES algorithm headers
└── sha256.h                     # SHA-256 algorithm headers
```

### Documentation
```
docs/
└── FRAM_ESP32_Specification.md  # Technical specification and integration guide
```

### Examples
```
examples/
├── credentials.json             # Example JSON configuration
└── backup_example.txt           # Example FRAM backup format
```

### GitHub Configuration
```
.github/
├── workflows/
│   └── build.yml               # CI/CD pipeline for automated building
└── ISSUE_TEMPLATE/
    └── bug_report.md           # Bug report template
```

## File Descriptions

### Configuration Files

**platformio.ini**
- Defines build environment for Beetle RP2350
- Specifies required libraries (Adafruit FRAM I2C, ArduinoJson)
- Sets serial monitor configuration and build flags

**README.md**
- Project overview and key features
- Hardware requirements and wiring diagram
- Quick start guide and usage examples
- CLI commands reference
- Technical specifications summary
- ESP32 integration overview

### Source Implementation

**main.cpp**
- Application initialization and main loop
- I2C setup for RP2350 (SDA=GPIO4, SCL=GPIO5)
- FRAM detection and CLI initialization
- Serial communication setup

**fram_programmer.cpp**
- FRAM detection and verification functions
- Read/write operations for credential structure
- Backup and restore functionality
- Data integrity checking with checksums

**encryption.cpp**
- AES-256-CBC encryption and decryption
- SHA-256 key derivation and password hashing
- PKCS#7 padding implementation
- Input validation for all credential fields

**cli_handler.cpp**
- Command parsing and execution
- Interactive credential input
- JSON configuration processing
- User interface and error messaging

**aes.cpp** & **sha256.cpp**
- Self-contained cryptographic implementations
- No external dependencies
- Optimized for embedded systems

### Documentation and Support

**FRAM_ESP32_Specification.md**
- Complete technical specification
- Memory layout and data structures
- Encryption algorithm details
- ESP32 integration code examples
- Security considerations and troubleshooting

**CONTRIBUTING.md**
- Development setup instructions
- Coding standards and style guide
- Testing requirements and procedures
- Pull request and review process

**CHANGELOG.md**
- Version history and release notes
- Feature additions and bug fixes
- Breaking changes and migration notes

## Dependencies

### Required Libraries
- Adafruit FRAM I2C (^2.0.3) - FRAM communication
- ArduinoJson (^6.21.3) - JSON configuration parsing

### Custom Implementation
- AES-256-CBC encryption (self-contained)
- SHA-256 hashing (self-contained)
- PKCS#7 padding (self-contained)

## Build and Deployment

### Local Development
```bash
git clone https://github.com/yourusername/fram-programmer.git
cd fram-programmer
pio run -t upload
pio device monitor
```

### Continuous Integration
- Automated building for multiple environments
- Static code analysis with cppcheck
- Documentation link checking
- Artifact generation and storage

### Hardware Testing
All code changes require validation on actual hardware:
- Beetle RP2350 microcontroller
- Compatible FRAM modules (FM25CL64, MB85RC256V)
- Complete credential programming and verification cycle

## Security Considerations

### Sensitive Files (Never Commit)
- Real credential configurations
- Production backup files
- Private keys or certificates
- Actual device passwords

### Protected Information
- Device-specific encryption keys (derived from device_name)
- FRAM backup contents (may contain encrypted credentials)
- Serial monitor outputs (may reveal debugging information)

## Project Status

Current version: 1.0.0
- Production ready
- Full hardware compatibility verified
- Complete ESP32 integration support
- Comprehensive documentation and testing

All files listed above are required for a complete GitHub repository setup.