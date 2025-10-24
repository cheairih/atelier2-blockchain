

#ifndef SHA256_H
#define SHA256_H

#include <string>
#include <vector>
#include <cstdint>

class SHA256 {
public:
    SHA256();
    void update(const uint8_t* data, size_t length);
    void update(const std::string& data);
    uint8_t* digest();
    static std::string toString(const uint8_t* digest);

private:
    void transform(const uint8_t* message, unsigned int block_nb);
    uint32_t m_h[8];
    uint8_t m_block[64];
    unsigned int m_len;
};

std::string sha256(const std::string& input);

#include <cstring>
#include <sstream>
#include <iomanip>

#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

SHA256::SHA256() {
    m_h[0] = 0x6a09e667;
    m_h[1] = 0xbb67ae85;
    m_h[2] = 0x3c6ef372;
    m_h[3] = 0xa54ff53a;
    m_h[4] = 0x510e527f;
    m_h[5] = 0x9b05688c;
    m_h[6] = 0x1f83d9ab;
    m_h[7] = 0x5be0cd19;
    m_len = 0;
}

void SHA256::transform(const uint8_t* message, unsigned int block_nb) {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;

    for (unsigned int i = 0; i < block_nb; i++) {
        for (unsigned int j = 0; j < 16; j++) {
            w[j] = (message[i * 64 + j * 4] << 24) | (message[i * 64 + j * 4 + 1] << 16) | (message[i * 64 + j * 4 + 2] << 8) | (message[i * 64 + j * 4 + 3]);
        }
        for (unsigned int j = 16; j < 64; j++) {
            w[j] = SIG1(w[j - 2]) + w[j - 7] + SIG0(w[j - 15]) + w[j - 16];
        }

        a = m_h[0]; b = m_h[1]; c = m_h[2]; d = m_h[3];
        e = m_h[4]; f = m_h[5]; g = m_h[6]; h = m_h[7];

        for (unsigned int j = 0; j < 64; j++) {
            uint32_t t1 = h + EP1(e) + CH(e, f, g) + k[j] + w[j];
            uint32_t t2 = EP0(a) + MAJ(a, b, c);
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }

        m_h[0] += a; m_h[1] += b; m_h[2] += c; m_h[3] += d;
        m_h[4] += e; m_h[5] += f; m_h[6] += g; m_h[7] += h;
    }
}

void SHA256::update(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        m_block[m_len++] = data[i];
        if (m_len == 64) {
            transform(m_block, 1);
            m_len = 0;
        }
    }
}

void SHA256::update(const std::string& data) {
    update(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
}

uint8_t* SHA256::digest() {
    static uint8_t hash[32];
    unsigned int i;
    uint64_t L = m_len * 8;

    m_block[m_len++] = 0x80;
    if (m_len > 56) {
        memset(m_block + m_len, 0, 64 - m_len);
        transform(m_block, 1);
        m_len = 0;
    }
    memset(m_block + m_len, 0, 56 - m_len);

    for (i = 0; i < 8; i++) {
        m_block[63 - i] = (L >> (i * 8)) & 0xff;
    }
    transform(m_block, 1);

    for (i = 0; i < 8; i++) {
        hash[i * 4] = (m_h[i] >> 24) & 0xff;
        hash[i * 4 + 1] = (m_h[i] >> 16) & 0xff;
        hash[i * 4 + 2] = (m_h[i] >> 8) & 0xff;
        hash[i * 4 + 3] = m_h[i] & 0xff;
    }
    return hash;
}

std::string SHA256::toString(const uint8_t* digest) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < 32; i++) {
        ss << std::setw(2) << static_cast<unsigned int>(digest[i]);
    }
    return ss.str();
}

std::string sha256(const std::string& input) {
    SHA256 sha;
    sha.update(input);
    return SHA256::toString(sha.digest());
}

#endif