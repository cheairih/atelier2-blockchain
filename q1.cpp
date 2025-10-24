#include <iostream>
#include <vector>
#include <string>
#include <cstdint> // Pour uint8_t

/**
 * @class CellularAutomaton1D
 * Implémente un automate cellulaire 1D avec état binaire et voisinage r=1.
 */
class CellularAutomaton1D {
private:
    std::vector<int> state; // L'état binaire de l'automate
    uint8_t rule;           // La règle de transition (ex: 30, 90, 110)

    /**
     * Calcule la nouvelle valeur d'une cellule basée sur son voisinage.
     * Utilise des conditions aux limites périodiques (circulaires).
     */
    int get_next_cell_state(int left, int center, int right) {
        // 1. Convertit le voisinage (ex: 1, 0, 1) en un nombre (ex: 5)
        int pattern = (left << 2) | (center << 1) | (right << 0);
        
        // 2. Applique la règle : vérifie si le N-ième bit de la règle est à 1
        // C'est l'astuce principale pour appliquer n'importe quelle règle.
        return (this->rule >> pattern) & 1;
    }

    /**
     * Gère les conditions aux limites (périodiques)
     * L'opérateur % peut mal gérer les négatifs en C++,
     * (idx % size + size) % size est une formule robuste.
     */
    int get_cell(int idx) {
        int size = state.size();
        return state[(idx % size + size) % size];
    }

public:
    /**
     * 1.1. Crée une fonction init_state()
     * Initialise l'état à partir d'un vecteur de bits. 
     */
    void init_state(const std::vector<int>& initial_bit_vector) {
        this->state = initial_bit_vector;
    }

    /**
     * Définit la règle à utiliser (ex: 30, 90, 110)
     */
    void set_rule(uint8_t rule_number) {
        this->rule = rule_number;
    }

    /**
     * 1.2. Implémente une fonction evolve()
     * Applique la règle de transition pour faire évoluer l'automate d'un pas. [cite: 10, 11]
     */
    void evolve() {
        if (state.empty()) return;

        int size = state.size();
        // Nous avons besoin d'un nouveau vecteur, car toutes les cellules
        // doivent être mises à jour simultanément (basé sur l'ancien état).
        std::vector<int> next_state(size); 

        for (int i = 0; i < size; ++i) {
            // Récupère le voisinage r=1 
            int left   = get_cell(i - 1);
            int center = get_cell(i);
            int right  = get_cell(i + 1);

            // Calcule le nouvel état de la cellule 'i'
            next_state[i] = get_next_cell_state(left, center, right);
        }

        // Met à jour l'état de l'automate
        this->state = next_state;
    }

    /**
     * Fonction utilitaire pour afficher l'état actuel (pour la vérification)
     */
    void print_state() const {
        for (int cell : state) {
            // Affiche un bloc '■' pour 1, un espace pour 0
            std::cout << (cell == 1 ? "■" : " ");
        }
        std::cout << std::endl;
    }
};


/**
 * 1.3. Vérifie que ton automate reproduit correctly la règle
 * 
 */
int main() {
    int width = 71; // Largeur de l'automate (impair pour un joli centrage)
    int steps = 35; // Nombre de générations à simuler

    // 1. Créer un état initial simple : un seul bit à 1 au milieu
    std::vector<int> initial_state(width, 0);
    initial_state[width / 2] = 1;

    CellularAutomaton1D ca;

    // --- Test avec la Règle 30 ---
    std::cout << "=== Verification Règle 30 ===" << std::endl;
    ca.set_rule(30); // 
    ca.init_state(initial_state); // 
    
    ca.print_state();
    for (int i = 0; i < steps; ++i) {
        ca.evolve(); // 
        ca.print_state();
    }
    std::cout << "\n(Observation: La Règle 30 produit un motif chaotique et complexe)\n" << std::endl;


    // --- Test avec la Règle 90 ---
    std::cout << "=== Verification Règle 90 ===" << std::endl;
    ca.set_rule(90); // 
    ca.init_state(initial_state); // Réinitialiser l'état
    
    ca.print_state();
    for (int i = 0; i < steps; ++i) {
        ca.evolve();
        ca.print_state();
    }
    std::cout << "\n(Observation: La Règle 90 produit le triangle de Sierpinski, un motif régulier)\n" << std::endl;

    return 0;
}