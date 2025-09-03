# Changelog

All notable changes to the FRAM Programmer project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2024-12-03

### Added
- Initial release of FRAM Programmer for ESP32 credentials
- AES-256-CBC encryption with SHA-256 key derivation
- CLI interface with interactive and JSON input modes
- Support for Beetle RP2350 microcontroller
- FRAM detection and verification (I2C address 0x50)
- Complete credential structure (1024 bytes at offset 0x0018)
- Backup and restore functionality
- Built-in diagnostic tests
- Real-time verification with decryption testing

### Implemented
- Custom AES-256 implementation (self-contained)
- Custom SHA-256 implementation (self-contained)
- PKCS#7 padding with multi-block support
- Checksum validation using offsetof() calculation
- I2C communication at 100kHz
- Error handling and recovery procedures

### Security Features
- Device-specific encryption keys
- Random IV generation per session
- Offline credential programming
- Tamper detection via checksum
- No plain-text credential storage

### Hardware Support
- Beetle RP2350 (DFRobot)
- FM25CL64 FRAM (32KB)
- MB85RC256V FRAM (32KB)
- Generic 32KB I2C FRAM modules

### CLI Commands
- `help` - Show available commands
- `detect` - Detect FRAM device
- `info` - Show FRAM information and status
- `program` - Interactive credential programming
- `config` - JSON-based configuration
- `verify` - Verify and decrypt stored credentials
- `backup` - Create hex dump of entire FRAM
- `restore` - Restore FRAM from backup
- `test` - Run comprehensive diagnostic tests

### Documentation
- Complete technical specification
- ESP32 integration guide
- Hardware wiring diagrams
- Troubleshooting procedures
- API reference

### Tested Configurations
- Beetle RP2350 + FM25CL64
- Beetle RP2350 + MB85RC256V
- PlatformIO 6.0+ with Arduino framework
- VSCode with PlatformIO extension

## [Unreleased]

### Planned
- Support for larger FRAM modules (64KB+)
- Web interface for credential programming
- Encrypted backup file format
- Additional ESP32 integration examples
- Performance optimizations for AES operations

### Under Consideration
- Support for other microcontrollers (ESP32, Arduino)
- Multiple credential profiles per FRAM
- Wireless programming interface
- Hardware security module integration

---

## Version History Summary

- **v1.0.0**: Initial production release with full functionality
- **v0.x.x**: Development and testing phases (not published)

## Migration Notes

### From Development Versions
This is the first stable release. No migration required.

### Future Versions  
Backward compatibility will be maintained through the version field in FRAM structure. Upgrade procedures will be documented for each major version.

## Support

For issues and feature requests, please use the GitHub issue tracker.
For technical questions, refer to the documentation in the `docs/` directory.