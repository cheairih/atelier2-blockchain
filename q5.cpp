#include <iostream>
#include <string>
#include <vector>
#include <iomanip> // Pour std::setprecision
#include <stdexcept>

// Inclut notre fonction de hachage de la Q2
#include "ac_hash.hpp"

/**
 * @brief Fonction utilitaire pour convertir un hash hexadécimal en un vecteur de bits (bools).
 * C'est l'inverse de bits_to_hex_string.
 */
std::vector<bool> hex_hash_to_bits(const std::string& hex_hash) {
    if (hex_hash.length() != HASH_SIZE_BITS / 4) {
        throw std::runtime_error("Taille du hash hexadécimal incorrecte.");
    }

    std::vector<bool> bits;
    bits.reserve(HASH_SIZE_BITS);

    for (char c : hex_hash) {
        uint8_t val;
        // Convertit le caractère hexadécimal en sa valeur numérique (0-15)
        if (c >= '0' && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            throw std::runtime_error("Caractere hexadecimal invalide.");
        }

        // Extrait les 4 bits de cette valeur
        for (int i = 3; i >= 0; --i) {
            bits.push_back((val >> i) & 1);
        }
    }
    return bits;
}

/**
 * @brief Compare deux vecteurs de bits et retourne le nombre de bits différents.
 * C'est ce qu'on appelle la "distance de Hamming".
 */
int calculate_bit_differences(const std::vector<bool>& h1_bits, const std::vector<bool>& h2_bits) {
    if (h1_bits.size() != h2_bits.size()) {
        throw std::runtime_error("Les hashes n'ont pas la meme taille de bits.");
    }

    int diff_count = 0;
    for (size_t i = 0; i < h1_bits.size(); ++i) {
        if (h1_bits[i] != h2_bits[i]) {
            diff_count++;
        }
    }
    return diff_count;
}


int main() {
    std::cout << "--- TEST DE L'EFFET AVALANCHE (Q5) ---" << std::endl;

    // Paramètres du test
    const uint32_t rule = 30;
    const size_t steps = 128;

    // 1. Créer deux messages ne différant que par UN SEUL bit.
    std::string message1 = "Bonjour le monde de la blockchain.";
    
    std::string message2 = message1;
    // On inverse le dernier bit (le bit le moins significatif) du dernier caractère.
    // 'e' (0110 0101) XOR 1 -> 'd' (0110 0100)
    // C'est un changement d'un seul bit dans l'entrée.
    message2.back() = message2.back() ^ 0x01; 

    std::cout << "Message 1: \"" << message1 << "\"" << std::endl;
    std::cout << "Message 2: \"" << message2 << "\"" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 2. Hacher les deux messages
    std::string hash1_hex = ac_hash(message1, rule, steps);
    std::string hash2_hex = ac_hash(message2, rule, steps);

    std::cout << "Hash 1 (hex): " << hash1_hex << std::endl;
    std::cout << "Hash 2 (hex): " << hash2_hex << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 3. Convertir les hashes hexadécimaux en vecteurs de bits
    std::vector<bool> hash1_bits = hex_hash_to_bits(hash1_hex);
    std::vector<bool> hash2_bits = hex_hash_to_bits(hash2_hex);

    // 4. Calculer les différences
    int differences = calculate_bit_differences(hash1_bits, hash2_bits);

    // 5.1. Calculer le pourcentage
    double percentage = (static_cast<double>(differences) / HASH_SIZE_BITS) * 100.0;

    // 5.2. Donner le résultat numérique
    std::cout << "RESULTAT DE L'EFFET AVALANCHE :" << std::endl;
    std::cout << "Nombre total de bits : " << HASH_SIZE_BITS << std::endl;
    std::cout << "Nombre de bits differents : " << differences << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Pourcentage de difference : " << percentage << " %" << std::endl;

    // Un bon résultat est proche de 50%
    if (percentage > 45.0 && percentage < 55.0) {
        std::cout << "\nConclusion : EXCELLENT effet avalanche." << std::endl;
    } else if (percentage > 40.0 && percentage < 60.0) {
        std::cout << "\nConclusion : Bon effet avalanche." << std::endl;
    } else {
        std::cout << "\nConclusion : FAIBLE effet avalanche. (Resultat non ideal)" << std::endl;
    }

    return 0;
}