#ifndef AC_HASH_HPP
#define AC_HASH_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstring>

const size_t HASH_SIZE_BITS = 256;
const size_t HASH_SIZE_BYTES = 32; // 256 / 8

class CellularAutomaton1D {
private:
    uint8_t state[HASH_SIZE_BYTES];
    uint8_t next_state[HASH_SIZE_BYTES];
    uint8_t rule;
    uint8_t rule_lookup[8];

    void init_rule_lookup() {
        for (int i = 0; i < 8; ++i) {
            rule_lookup[i] = (rule >> i) & 1;
        }
    }

    inline bool get_bit(const uint8_t* data, size_t bit_index) const {
        size_t byte_index = bit_index >> 3;
        size_t bit_offset = 7 - (bit_index & 7);
        return (data[byte_index] >> bit_offset) & 1;
    }

    // --- OPTIMISATION v5 (1/2) : 'set_bit' sans branche ---
    // Supprime le 'if-else', qui est lent pour le CPU.
    // Utilise la logique binaire pour forcer le bit à 0 ou 1.
    inline void set_bit(uint8_t* data, size_t bit_index, bool value) {
        size_t byte_index = bit_index >> 3;
        size_t bit_offset = 7 - (bit_index & 7);
        uint8_t mask = 1 << bit_offset;
        
        // (data & ~mask) -> met le bit à 0
        // (value << bit_offset) -> met le bit à 'value'
        data[byte_index] = (data[byte_index] & ~mask) | (value << bit_offset);
    }
    // --- FIN OPTIMISATION ---

public:
    CellularAutomaton1D() {
        std::memset(state, 0, HASH_SIZE_BYTES);
        std::memset(next_state, 0, HASH_SIZE_BYTES);
    }

    void set_rule(uint8_t r) { 
        this->rule = r;
        init_rule_lookup();
    }

    void init_state(const uint8_t* bits, size_t size) {
        std::memset(state, 0, HASH_SIZE_BYTES);
        size_t copy_size = (size < HASH_SIZE_BYTES) ? size : HASH_SIZE_BYTES;
        std::memcpy(state, bits, copy_size);
    }

    // --- OPTIMISATION v5 (2/2) : Boucle 'evolve' sans branche ---
    void evolve() {
        // HASH_SIZE_BITS - 1 = 255 (en binaire: 11111111)
        const size_t mask = HASH_SIZE_BITS - 1;

        for (size_t bit_idx = 0; bit_idx < HASH_SIZE_BITS; ++bit_idx) {
            
            // Remplacement des 'if (bit_idx == 0) ? ...' par
            // un AND bitwise, ce qui est 100x plus rapide.
            size_t left_idx  = (bit_idx - 1) & mask; // (0-1)&255 = 255
            size_t right_idx = (bit_idx + 1) & mask; // (255+1)&255 = 0
            
            bool left   = get_bit(state, left_idx);
            bool center = get_bit(state, bit_idx);
            bool right  = get_bit(state, right_idx);
            
            int pattern = (left << 2) | (center << 1) | right;
            
            bool new_value = rule_lookup[pattern];
            set_bit(next_state, bit_idx, new_value);
        }
        
        std::memcpy(state, next_state, HASH_SIZE_BYTES);
    }
    // --- FIN OPTIMISATION ---
    
    const uint8_t* get_final_state() const { 
        return state; 
    }
};


// --- (Le reste du fichier est identique à v4) ---

inline void string_to_bytes(const std::string& input, uint8_t* output, size_t output_size) {
    std::memset(output, 0, output_size);
    
    for (size_t i = 0; i < input.length(); ++i) {
        output[i % output_size] ^= static_cast<uint8_t>(input[i]);
    }
    
    size_t input_len = input.length();
    for (size_t i = 0; i < sizeof(input_len); ++i) {
        output[i % output_size] ^= static_cast<uint8_t>((input_len >> (i * 8)) & 0xFF);
    }
}

inline std::string bytes_to_hex_string(const uint8_t* bytes, size_t size) {
    static const char hex_chars[] = "0123456789abcdef";
    std::string result;
    result.reserve(size * 2);
    
    for (size_t i = 0; i < size; ++i) {
        result.push_back(hex_chars[(bytes[i] >> 4) & 0xF]);
        result.push_back(hex_chars[bytes[i] & 0xF]);
    }
    
    return result;
}

inline std::string ac_hash(const std::string& input, uint32_t rule, size_t steps) {
    uint8_t initial_state[HASH_SIZE_BYTES];
    
    string_to_bytes(input, initial_state, HASH_SIZE_BYTES);
    
    CellularAutomaton1D ac;
    ac.set_rule(static_cast<uint8_t>(rule));
    ac.init_state(initial_state, HASH_SIZE_BYTES);
    
    for (size_t i = 0; i < steps; ++i) {
        ac.evolve();
    }
    
    return bytes_to_hex_string(ac.get_final_state(), HASH_SIZE_BYTES);
}

#endif // AC_HASH_HPP