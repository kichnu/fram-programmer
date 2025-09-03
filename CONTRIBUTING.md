# Contributing to FRAM Programmer

Thank you for considering contributing to the FRAM Programmer project. This document outlines the contribution process and coding standards.

## How to Contribute

### Reporting Bugs

Before submitting a bug report:
1. Check existing issues to avoid duplicates
2. Test with the latest version
3. Verify hardware connections and compatibility

Include in your bug report:
- Hardware configuration (microcontroller, FRAM model)
- PlatformIO version and environment details
- Steps to reproduce the issue
- Expected vs actual behavior
- Serial monitor output or logs
- Relevant code snippets

### Suggesting Features

Feature requests should include:
- Clear description of the proposed functionality
- Use case or problem it solves
- Compatibility considerations
- Implementation approach (if known)

### Pull Requests

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature-name`
3. Make your changes following the coding standards
4. Test thoroughly on actual hardware
5. Update documentation if needed
6. Commit with clear, descriptive messages
7. Push to your fork and submit a pull request

## Development Setup

### Prerequisites
- PlatformIO Core 6.0+
- VSCode with PlatformIO extension (recommended)
- Git for version control

### Hardware Requirements
- Beetle RP2350 microcontroller
- 32KB FRAM module (FM25CL64, MB85RC256V, or compatible)
- Jumper wires and breadboard
- USB cable for programming

### Environment Setup
```bash
git clone https://github.com/yourusername/fram-programmer.git
cd fram-programmer
pio run  # Build project and download dependencies
```

### Testing Setup
Connect hardware as specified in README.md and run:
```bash
pio run -t upload
pio device monitor
FRAM> test  # Run diagnostic tests
```

## Coding Standards

### C++ Style Guide

**File Organization:**
- Header files in `include/` directory
- Implementation files in `src/` directory  
- Use `#pragma once` or include guards
- Forward declarations where appropriate

**Naming Conventions:**
```cpp
// Functions and variables: camelCase
bool initFRAM();
size_t dataLength;

// Constants: UPPER_CASE
#define FRAM_I2C_ADDR 0x50

// Classes/Structs: PascalCase  
struct FRAMCredentials;
class AES256_CBC;

// File names: snake_case
fram_programmer.cpp
encryption.h
```

**Code Style:**
```cpp
// Use 4 spaces for indentation (no tabs)
if (condition) {
    doSomething();
    
    if (nestedCondition) {
        doNestedAction();
    }
}

// Function declarations
bool functionName(const uint8_t* data, 
                 size_t length,
                 uint8_t* output);

// Comments for complex logic
// Calculate PKCS#7 padding bytes needed
size_t padding = block_size - (data_len % block_size);
```

**Memory Management:**
- Use stack allocation when possible
- Always check `new` return values
- Match every `new` with `delete`
- Use RAII principles for resource management

### Error Handling

**Serial Output:**
```cpp
Serial.println("ERROR: Descriptive error message");
Serial.println("SUCCESS: Operation completed");
Serial.println("WARNING: Potential issue detected");
Serial.println("DEBUG: Development information");
```

**Return Values:**
- Use `bool` for success/failure operations
- Return error codes or lengths when appropriate
- Document return values in function comments

**Input Validation:**
```cpp
bool validateInput(const String& input) {
    if (input.length() == 0) {
        Serial.println("ERROR: Input cannot be empty");
        return false;
    }
    
    if (input.length() > MAX_LENGTH) {
        Serial.println("ERROR: Input too long");
        return false;
    }
    
    return true;
}
```

### Documentation Standards

**Function Documentation:**
```cpp
/**
 * Encrypt data using AES-256-CBC with PKCS#7 padding
 * @param plaintext Input data to encrypt
 * @param plaintext_len Length of input data
 * @param key 32-byte AES key  
 * @param iv 8-byte initialization vector
 * @param ciphertext Output buffer (must be >= field size)
 * @param ciphertext_len Input: buffer size, Output: encrypted length
 * @return true on success, false on error
 */
bool encryptData(const uint8_t* plaintext, size_t plaintext_len,
                 const uint8_t* key, const uint8_t* iv,
                 uint8_t* ciphertext, size_t* ciphertext_len);
```

**Code Comments:**
- Explain complex algorithms or business logic
- Document security-critical sections
- Explain hardware-specific constraints
- Update comments when changing code

## Testing Requirements

### Hardware Testing
All changes must be tested on actual hardware:
- Verify FRAM detection and basic I2C communication
- Test encryption/decryption with known test vectors
- Validate CLI commands and error handling
- Check power consumption and timing constraints

### Test Coverage
Key areas that must work:
- FRAM read/write operations
- AES-256-CBC encryption/decryption  
- SHA-256 key generation
- PKCS#7 padding/unpadding
- Checksum calculation and verification
- CLI command parsing and execution

### Regression Testing
Before submitting changes:
```bash
FRAM> test     # Run built-in diagnostic tests
FRAM> program  # Test interactive programming
FRAM> config   # Test JSON configuration
FRAM> verify   # Test credential verification
FRAM> backup   # Test backup functionality
```

## Security Considerations

### Cryptographic Code
- Use established algorithms (AES-256, SHA-256)
- Validate all cryptographic operations
- Use constant-time operations where possible
- Clear sensitive data from memory after use

### Input Validation
- Sanitize all user inputs
- Validate data lengths and formats
- Check for buffer overflows
- Handle malformed JSON gracefully

### Error Messages
- Avoid leaking sensitive information
- Provide helpful but not overly detailed errors
- Log security events appropriately

## Commit Guidelines

### Commit Messages
```
type(scope): brief description

Longer explanation of the change, if needed.
Include motivation and context.

Fixes #123
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix  
- `docs`: Documentation changes
- `style`: Code formatting changes
- `refactor`: Code restructuring without functionality changes
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

**Examples:**
```
feat(encryption): add support for AES-128-CBC mode
fix(cli): handle empty input in program command  
docs(readme): update hardware compatibility list
```

### Branch Naming
- `feature/add-web-interface`
- `fix/checksum-calculation-error`  
- `docs/update-api-reference`

## Review Process

### Code Review Checklist
- Code follows style guidelines
- Adequate test coverage
- Documentation is updated
- No security vulnerabilities
- Hardware compatibility verified
- Performance impact considered

### Approval Process
- All automated checks pass
- At least one maintainer review
- Hardware testing completed
- Documentation review (if applicable)

## Release Process

### Version Numbering
Follow Semantic Versioning (SemVer):
- Major: Breaking changes
- Minor: New features (backward compatible)
- Patch: Bug fixes

### Release Checklist
- Update CHANGELOG.md
- Tag version in git
- Update version in platformio.ini
- Test release build
- Update documentation

## Getting Help

- GitHub Issues: Bug reports and feature requests
- GitHub Discussions: General questions and ideas
- Documentation: Technical specifications in `docs/`
- Code Review: Ask questions in pull request comments

## License

By contributing to this project, you agree that your contributions will be licensed under the MIT License.