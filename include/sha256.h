#ifndef SHA256_H
#define SHA256_H

#include <Arduino.h>

// Simple SHA-256 implementation
class SHA256 {
private:
    uint32_t m_data[16];
    uint32_t m_blocklen;
    uint64_t m_bitlen;
    uint32_t m_state[8];

    void transform();
    static uint32_t rotr(uint32_t x, uint32_t n);
    static uint32_t choose(uint32_t e, uint32_t f, uint32_t g);
    static uint32_t majority(uint32_t a, uint32_t b, uint32_t c);
    static uint32_t sig0(uint32_t x);
    static uint32_t sig1(uint32_t x);

public:
    SHA256();
    void init();
    void update(const uint8_t data[], size_t len);
    void final(uint8_t hash[]);
};

// Convenience function
void sha256_hash(const uint8_t* data, size_t len, uint8_t hash[32]);
void sha256_hash(const String& str, uint8_t hash[32]);

#endif // SHA256_H