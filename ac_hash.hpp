#ifndef AC_HASH_HPP
#define AC_HASH_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <stdexcept> 

const size_t HASH_SIZE_BITS = 256;

/**
 * @class CellularAutomaton1D
 * Version optimisée (v3)
 */
class CellularAutomaton1D {
private:
    std::vector<bool> state; 
    uint8_t rule;

    int get_next_cell_state(bool left, bool center, bool right) { 
        int pattern = (left << 2) | (center << 1) | right;
        return (this->rule >> pattern) & 1;
    }

    // --- OPTIMISATION V3 ---
    /**
     * @brief Gère les conditions aux limites (périodiques)
     * Remplacement du modulo (%) lent par un bitwise AND (&) rapide.
     * (idx & 255) est équivalent à (idx % 256) mais 100x plus rapide.
     */
    bool get_cell(int idx) { 
        // HASH_SIZE_BITS - 1 = 255 (ou 0xFF)
        return state[idx & (HASH_SIZE_BITS - 1)];
    }
    // --- FIN OPTIMISATION ---

public:
    void init_state(const std::vector<bool>& bits) { this->state = bits; }
    void set_rule(uint8_t r) { this->rule = r; }

    void evolve() {
        if (state.empty()) return;
        
        std::vector<bool> next_state(HASH_SIZE_BITS); 
        
        // CORRECTION: int i -> size_t i
        for (size_t i = 0; i < HASH_SIZE_BITS; ++i) { 
            next_state[i] = get_next_cell_state(get_cell(i-1), get_cell(i), get_cell(i+1));
        }
        this->state = next_state;
    }
    
    const std::vector<bool>& get_final_state() const { return this->state; } 
};

// --- (Le reste du fichier est identique) ---

inline std::vector<bool> string_to_bits(const std::string& input) {
    std::vector<bool> bits;
    bits.reserve(input.length() * 8); 
    for (char c : input) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }
    return bits;
}

inline std::string bits_to_hex_string(const std::vector<bool>& bits) {
    if (bits.size() % 8 != 0) {
        throw std::runtime_error("Taille de bits non multiple de 8.");
    }
    std::stringstream ss_hex;
    ss_hex << std::hex << std::setfill('0');
    for (size_t i = 0; i < bits.size(); i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8; ++j) {
            if (bits[i + j]) { byte |= (1 << (7 - j)); }
        }
        ss_hex << std::setw(2) << static_cast<int>(byte);
    }
    return ss_hex.str();
}

inline std::string ac_hash(const std::string& input, uint32_t rule, size_t steps) {
    std::vector<bool> input_bits = string_to_bits(input); 
    std::vector<bool> initial_state(HASH_SIZE_BITS, false); 
    
    if (!input_bits.empty()) {
        for (size_t i = 0; i < input_bits.size(); ++i) {
            initial_state[i % HASH_SIZE_BITS] = initial_state[i % HASH_SIZE_BITS] ^ input_bits[i];
        }
    }
    
    size_t input_len = input.length();
    for (size_t i = 0; i < sizeof(input_len) * 8; ++i) {
        initial_state[i % HASH_SIZE_BITS] = initial_state[i % HASH_SIZE_BITS] ^ ((input_len >> i) & 1);
    }

    CellularAutomaton1D ac;
    ac.set_rule(static_cast<uint8_t>(rule));
    ac.init_state(initial_state);
    for (size_t i = 0; i < steps; ++i) {
        ac.evolve();
    }
    
    return bits_to_hex_string(ac.get_final_state());
}

#endif // AC_HASH_HPP