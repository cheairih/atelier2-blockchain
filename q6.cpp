#include <iostream>
#include <string>
#include <vector>
#include <iomanip> // Pour std::setprecision
#include <stdexcept>
#include <numeric> // Pour std::accumulate

// Inclut notre fonction de hachage de la Q2
#include "ac_hash.hpp"

/**
 * @brief Fonction utilitaire (de la Q5) pour convertir un hash hexadécimal en bits.
 */
std::vector<bool> hex_hash_to_bits(const std::string& hex_hash) {
    if (hex_hash.length() != HASH_SIZE_BITS / 4) {
        throw std::runtime_error("Taille du hash hexadécimal incorrecte.");
    }
    std::vector<bool> bits;
    bits.reserve(HASH_SIZE_BITS);
    for (char c : hex_hash) {
        uint8_t val;
        if (c >= '0' && c <= '9') { val = c - '0'; }
        else if (c >= 'a' && c <= 'f') { val = c - 'a' + 10; }
        else if (c >= 'A' && c <= 'F') { val = c - 'A' + 10; }
        else { throw std::runtime_error("Caractere hexadecimal invalide."); }
        for (int i = 3; i >= 0; --i) {
            bits.push_back((val >> i) & 1);
        }
    }
    return bits;
}


int main() {
    std::cout << "--- TEST DE DISTRIBUTION DES BITS (Q6) ---" << std::endl;

    // Paramètres du test
    const uint32_t rule = 30;
    const size_t steps = 128;
    // 6.1: Nous avons besoin d'au moins 100 000 bits
    // 500 hashes * 256 bits/hash = 128 000 bits. C'est suffisant.
    const int num_hashes_to_generate = 500;

    long long total_bits_sampled = 0;
    long long total_ones_count = 0;

    std::cout << "Generation de " << num_hashes_to_generate << " hashes (echantillon de " 
              << num_hashes_to_generate * HASH_SIZE_BITS << " bits)..." << std::endl;

    for (int i = 0; i < num_hashes_to_generate; ++i) {
        // 1. Génère un input unique pour chaque hash
        std::string input = "un_message_different_pour_le_test_" + std::to_string(i);

        // 2. Calcule le hash
        std::string hash_hex = ac_hash(input, rule, steps);

        // 3. Convertit le hash en bits
        std::vector<bool> hash_bits = hex_hash_to_bits(hash_hex);

        // 4. Compte les bits à '1' dans ce hash
        int ones_in_this_hash = 0;
        for (bool bit : hash_bits) {
            if (bit) {
                ones_in_this_hash++;
            }
        }
        
        // 5. Ajoute aux totaux
        total_ones_count += ones_in_this_hash;
        total_bits_sampled += hash_bits.size(); // (ajoute 256)
    }

    // --- 6.1. Calcule le pourcentage ---
    double percentage = 0.0;
    if (total_bits_sampled > 0) {
        percentage = (static_cast<double>(total_ones_count) / total_bits_sampled) * 100.0;
    }

    std::cout << "\n--- RESULTATS DE L'ANALYSE ---" << std::endl;
    std::cout << "Nombre total de bits echantillonnes : " << total_bits_sampled << std::endl;
    std::cout << "Nombre total de bits a 1 : " << total_ones_count << std::endl;
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Pourcentage de bits a 1 : " << percentage << " %" << std::endl;

    // --- 6.2. Indique si la distribution est équilibrée ---
    if (percentage > 49.0 && percentage < 51.0) {
        std::cout << "\nConclusion : La distribution est TRES equilibree (proche de 50%)." << std::endl;
    } else if (percentage > 45.0 && percentage < 55.0) {
        std::cout << "\nConclusion : La distribution est acceptablement equilibree." << std::endl;
    } else {
        std::cout << "\nConclusion : La distribution N'EST PAS equilibree (loin de 50%)." << std::endl;
    }

    return 0;
}