#include <iostream>
#include <vector>
#include <string>
#include <cstdint> // Pour uint8_t, uint32_t
#include <sstream> // Pour la conversion en hexadécimal
#include <iomanip> // Pour std::setw, std::setfill
#include <stdexcept> // Pour std::runtime_error

// Définition de la taille fixe de notre hash en bits
const size_t HASH_SIZE_BITS = 256;

// --- CLASSE DE LA QUESTION 1 ---
/**
 * @class CellularAutomaton1D
 * Implémente un automate cellulaire 1D avec état binaire et voisinage r=1.
 */
class CellularAutomaton1D {
private:
    std::vector<int> state; 
    uint8_t rule;           

    int get_next_cell_state(int left, int center, int right) {
        int pattern = (left << 2) | (center << 1) | (right << 0);
        return (this->rule >> pattern) & 1;
    }

    int get_cell(int idx) {
        int size = state.size();
        return state[(idx % size + size) % size];
    }

public:
    void init_state(const std::vector<int>& initial_bit_vector) {
        this->state = initial_bit_vector;
    }

    void set_rule(uint8_t rule_number) {
        this->rule = rule_number;
    }

    void evolve() {
        if (state.empty()) return;
        int size = state.size();
        std::vector<int> next_state(size); 
        for (int i = 0; i < size; ++i) {
            int left   = get_cell(i - 1);
            int center = get_cell(i);
            int right  = get_cell(i + 1);
            next_state[i] = get_next_cell_state(left, center, right);
        }
        this->state = next_state;
    }
    
    // Accesseur pour récupérer l'état final
    const std::vector<int>& get_final_state() const {
        return this->state;
    }
};
// --- FIN CLASSE Q1 ---


/**
 * @brief Convertit une chaîne de caractères en un vecteur de bits (int).
 */
std::vector<int> string_to_bits(const std::string& input) {
    std::vector<int> bits;
    // Réserve la place pour 8 bits par caractère
    bits.reserve(input.length() * 8); 

    for (char c : input) {
        // Pour chaque caractère (qui fait 8 bits)...
        for (int i = 7; i >= 0; --i) {
            // ...on extrait chaque bit, du plus fort au plus faible
            bits.push_back((c >> i) & 1);
        }
    }
    return bits;
}

/**
 * @brief Convertit un vecteur de bits en une chaîne hexadécimale.
 */
std::string bits_to_hex_string(const std::vector<int>& bits) {
    if (bits.size() % 8 != 0) {
        throw std::runtime_error("La taille du vecteur de bits n'est pas un multiple de 8.");
    }

    std::stringstream ss_hex;
    ss_hex << std::hex << std::setfill('0');

    // Regroupe les bits 8 par 8 pour former des octets (bytes)
    for (size_t i = 0; i < bits.size(); i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8; ++j) {
            if (bits[i + j] == 1) {
                byte |= (1 << (7 - j));
            }
        }
        // Ajoute l'octet au flux, formaté sur 2 caractères hexa
        ss_hex << std::setw(2) << static_cast<int>(byte);
    }
    return ss_hex.str();
}


/**
 * 2.1. Fonction demandée
 * Implémente une fonction de hachage basée sur l'automate cellulaire.
 */
std::string ac_hash(const std::string& input, uint32_t rule, size_t steps) {
    
    // --- 2.2. Conversion du texte d'entrée en bits ---
    // 1. Convertit le string d'entrée en un long vecteur de bits
    std::vector<int> input_bits = string_to_bits(input);

    // --- 2.3. Processus pour produire un hash final fixe de 256 bits ---
    // 2. Initialise un état de 256 bits (taille fixe)
    std::vector<int> initial_state(HASH_SIZE_BITS, 0);

    // 3. "Plie" (fold) les bits de l'entrée dans l'état initial
    // On utilise l'opération XOR pour "mélanger" les bits d'entrée
    // de manière à ce que chaque bit d'entrée ait une influence.
    if (input_bits.empty()) {
        // Gérer le cas d'une entrée vide
    } else {
        for (size_t i = 0; i < input_bits.size(); ++i) {
            initial_state[i % HASH_SIZE_BITS] ^= input_bits[i];
        }
    }
    // On ajoute aussi la taille de l'entrée pour éviter les collisions
    // (ex: "A" et "A\0" pourraient sinon donner le même hash).
    // C'est une forme simple de "padding" (remplissage).
    size_t input_len = input.length();
    for (size_t i = 0; i < sizeof(input_len) * 8; ++i) {
        initial_state[i % HASH_SIZE_BITS] ^= (input_len >> i) & 1;
    }


    // 4. Crée et configure l'automate
    CellularAutomaton1D ac;
    ac.set_rule(static_cast<uint8_t>(rule)); // Utilise la règle donnée
    ac.init_state(initial_state); // Définit l'état initial de 256 bits

    // 5. Fait évoluer l'automate pendant 'steps' générations
    for (size_t i = 0; i < steps; ++i) {
        ac.evolve();
    }

    // 6. L'état final de 256 bits EST notre hash
    std::vector<int> final_hash_bits = ac.get_final_state();

    // 7. Convertit les 256 bits en une chaîne hexadécimale (64 chars)
    return bits_to_hex_string(final_hash_bits);
}


/**
 * 2.4. Vérifie par un test que deux entrées différentes 
 * donnent deux sorties différentes.
 */
int main() {
    std::string input1 = "Bonjour le monde";
    std::string input2 = "Bonjour le monde."; // Juste un point de différence !

    // Paramètres de hachage
    uint32_t rule = 30; // Règle 30 (chaotique)
    size_t steps = 128; // Nombre d'itérations

    std::cout << "--- Test de la fonction ac_hash ---" << std::endl;
    std::cout << "Regle        : " << rule << std::endl;
    std::cout << "Iterations   : " << steps << std::endl;
    std::cout << "Taille du hash: " << HASH_SIZE_BITS << " bits" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    // Hachage de l'entrée 1
    std::string hash1 = ac_hash(input1, rule, steps);
    std::cout << "Input 1: \"" << input1 << "\"" << std::endl;
    std::cout << "Hash 1 : " << hash1 << std::endl;
    std::cout << std::endl;

    // Hachage de l'entrée 2
    std::string hash2 = ac_hash(input2, rule, steps);
    std::cout << "Input 2: \"" << input2 << "\"" << std::endl;
    std::cout << "Hash 2 : " << hash2 << std::endl;
    std::cout << std::endl;

    // Vérification
    if (hash1 != hash2) {
        std::cout << "VERIFICATION REUSSIE: Les deux hashs sont differents." << std::endl;
    } else {
        std::cout << "VERIFICATION ECHOUEE : Les deux hashs sont identiques !" << std::endl;
    }

    return 0;
}