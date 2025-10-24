#include <iostream>
#include <string>
#include <vector>
#include <iomanip> // Pour std::setprecision
#include <stdexcept>
#include <chrono> // Pour le chronomètre

// Inclut notre fonction de hachage de la Q2
#include "ac_hash.hpp"

/**
 * @brief Fonction de test qui chronomètre la génération de 'num_hashes' hashes
 * en utilisant une règle spécifique.
 * @return Le temps total d'exécution en secondes.
 */
double run_performance_test(uint32_t rule_number, int num_hashes_to_generate) {
    
    std::cout << "Test de la Regle " << rule_number << " (generation de " 
              << num_hashes_to_generate << " hashes)..." << std::flush;

    // Paramètres constants
    const size_t steps = 128;

    auto t_start = std::chrono::high_resolution_clock::now();

    // Boucle de génération de hashes
    for (int i = 0; i < num_hashes_to_generate; ++i) {
        std::string input = "message_test_" + std::to_string(i);
        // On génère le hash, mais on ne le stocke pas pour ne pas
        // mesurer le coût de l'allocation mémoire.
        volatile std::string hash = ac_hash(input, rule_number, steps);
        (void)hash; // Empêche l'optimiseur de supprimer la ligne
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double time_taken = std::chrono::duration<double>(t_end - t_start).count();

    std::cout << " Termine en " << std::fixed << std::setprecision(4) 
              << time_taken << " secondes." << std::endl;
              
    return time_taken;
}


int main() {
    std::cout << "--- TEST DE PERFORMANCE DES REGLES (Q7) ---" << std::endl;

    // Un grand nombre pour avoir une mesure de temps significative
    const int num_hashes = 20000; 

    // --- 7.1. Exécute ac_hash avec les 3 règles ---
    // (Les citations [cite: 33] ont été supprimées d'ici)
    double time_rule_30 = run_performance_test(30, num_hashes);
    double time_rule_90 = run_performance_test(90, num_hashes);
    double time_rule_110 = run_performance_test(110, num_hashes);

    // --- 7.2. Compare les temps d'exécution ---
    std::cout << "\n--- COMPARAISON (Q7.2) ---" << std::endl;
    std::cout << "+----------+---------------------+" << std::endl;
    std::cout << "| Regle    | Temps d'execution   |" << std::endl;
    std::cout << "+----------+---------------------+" << std::endl;
    std::cout << "| Rule 30  | " << std::setw(19) << time_rule_30 << " s |" << std::endl;
    std::cout << "| Rule 90  | " << std::setw(19) << time_rule_90 << " s |" << std::endl;
    std::cout << "| Rule 110 | " << std::setw(19) << time_rule_110 << " s |" << std::endl;
    std::cout << "+----------+---------------------+" << std::endl;

    return 0;
}