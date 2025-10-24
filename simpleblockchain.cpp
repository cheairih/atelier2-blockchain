#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <numeric> // Pour std::accumulate
#include <random>  // Pour la sélection aléatoire
#include "sha256.hpp"

// Structure simple pour représenter un validateur
struct Validator {
    std::string address;
    double stake;
};

// --- Classe Block (légèrement modifiée pour PoS) ---
class Block {
private:
    uint32_t _nIndex;
    std::string _sData;
    time_t _tTime;
    std::string _sValidatorAddress; // Adresse du validateur choisi

    // Fonction de hachage privée
    std::string _CalculateHash() const {
        std::string ss = std::to_string(_nIndex) + std::to_string(_tTime) + _sData + sPrevHash + _sValidatorAddress;
        return sha256(ss);
    }

public:
    std::string sPrevHash;
    std::string sHash;

    Block(uint32_t nIndexIn, const std::string &sDataIn) : _nIndex(nIndexIn), _sData(sDataIn) {
        _tTime = time(nullptr);
    }

    // Fonction de validation PoS (remplace le minage PoW)
    void ValidateBlock(const std::string& validatorAddress) {
        _sValidatorAddress = validatorAddress;
        sHash = _CalculateHash();
    }

    // --- Fonction de minage PoW (gardée pour la comparaison) ---
    void MineBlock(uint32_t nDifficulty) {
        std::string str(nDifficulty, '0');
        int64_t nNonce = 0;
        
        do {
            nNonce++;
            std::string ss = std::to_string(_nIndex) + std::to_string(_tTime) + _sData + sPrevHash + std::to_string(nNonce);
            sHash = sha256(ss);
        } while (sHash.substr(0, nDifficulty) != str);
    }
};


// Classe Blockchain
class Blockchain {
private:
    std::vector<Block> _vChain;
    std::vector<Validator> _vValidators;

    const Block& _GetLastBlock() const {
        return _vChain.back();
    }

    // sélection du validateur 
    Validator& SelectValidator() {
        double totalStake = 0.0;
        for (const auto& v : _vValidators) {
            totalStake += v.stake;
        }

        // Générer un nombre aléatoire entre 0 et totalStake
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distrib(0, totalStake);
        double randomPoint = distrib(gen);

        double currentSum = 0.0;
        for (auto& v : _vValidators) {
            currentSum += v.stake;
            if (randomPoint <= currentSum) {
                return v;
            }
        }
        // En cas d'erreur de virgule flottante, retourner le dernier
        return _vValidators.back();
    }


public:
    Blockchain() {
        _vChain.emplace_back(Block(0, "Genesis Block"));
    }

    // Ajouter des validateurs au réseau
    void AddValidator(const std::string& address, double stake) {
        _vValidators.push_back({address, stake});
    }

    // Ajout d'un bloc avec PoS
    void AddBlockPoS(Block bNew) {
        if (_vValidators.empty()) {
            std::cout << "Erreur: Aucun validateur dans le reseau !" << std::endl;
            return;
        }
        Validator& chosenValidator = SelectValidator();
        std::cout << "Validateur choisi: " << chosenValidator.address << " (Enjeu: " << chosenValidator.stake << ")" << std::endl;
        
        bNew.sPrevHash = _GetLastBlock().sHash;
        bNew.ValidateBlock(chosenValidator.address);
        _vChain.push_back(bNew);
    }

    // Ajout d'un bloc avec PoW (pour la comparaison)
    void AddBlockPoW(Block bNew, uint32_t difficulty) {
        bNew.sPrevHash = _GetLastBlock().sHash;
        bNew.MineBlock(difficulty);
        _vChain.push_back(bNew);
    }
};

int main() {
    // EXEMPLE DE VALIDATION AVEC PROOF OF STAKE
    std::cout << "--- Simulation Proof of Stake (PoS) ---" << std::endl;
    Blockchain posChain = Blockchain();
    // Ajouter des validateurs avec leurs enjeux
    posChain.AddValidator("Alice", 100);
    posChain.AddValidator("Bob", 50);
    posChain.AddValidator("Charlie", 250);
    posChain.AddValidator("David", 20);

    auto t_start_pos = std::chrono::high_resolution_clock::now();
    posChain.AddBlockPoS(Block(1, "Transaction Data PoS"));
    auto t_end_pos = std::chrono::high_resolution_clock::now();
    double time_taken_pos = std::chrono::duration<double, std::milli>(t_end_pos - t_start_pos).count();
    
    std::cout << "Bloc PoS ajoute avec succes." << std::endl;
    std::cout << "Temps d'execution pour PoS: " << time_taken_pos << " ms" << std::endl;


    std::cout << "\n============================================\n" << std::endl;


    // EXEMPLE DE VALIDATION AVEC PROOF OF WORK 
    std::cout << "--- Simulation Proof of Work (PoW) ---" << std::endl;
    Blockchain powChain = Blockchain();
    uint32_t difficulty = 5; 

    auto t_start_pow = std::chrono::high_resolution_clock::now();
    std::cout << "Minage du bloc PoW avec difficulte " << difficulty << "..." << std::endl;
    powChain.AddBlockPoW(Block(1, "Transaction Data PoW"), difficulty);
    auto t_end_pow = std::chrono::high_resolution_clock::now();
    double time_taken_pow = std::chrono::duration<double, std::milli>(t_end_pow - t_start_pow).count();
    
    std::cout << "Bloc PoW ajoute avec succes." << std::endl;
    std::cout << "Temps d'execution pour PoW: " << time_taken_pow << " ms" << std::endl;


    std::cout << "\n============================================\n" << std::endl;

    //  COMPARAISON 
    std::cout << "--- Comparaison des performances ---" << std::endl;
    std::cout << "Temps PoS: " << time_taken_pos << " ms" << std::endl;
    std::cout << "Temps PoW (difficulte " << difficulty << "): " << time_taken_pow << " ms" << std::endl;

    if (time_taken_pos < time_taken_pow) {
        std::cout << "\nConclusion : Proof of Stake est nettement plus rapide que Proof of Work." << std::endl;
    } else {
        std::cout << "\nConclusion : Dans cette simulation, Proof of Work a ete plus rapide (ce qui est inhabituel)." << std::endl;
    }
    
    return 0;
}