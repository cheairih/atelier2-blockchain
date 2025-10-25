#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <numeric> // Pour std::accumulate
#include <random>  // Pour la sélection aléatoire

#include "sha256.hpp"     // Votre hachage SHA256 existant
#include "ac_hash.hpp"    // <-- INCLUSION DU FICHIER DE LA Q2

/**
 * 3.1. Ajoute une option de sélection du mode de hachage
 */
enum class HashMethod {
    SHA256,
    AC_HASH
};


// Structure simple pour représenter un validateur
struct Validator {
    std::string address;
    double stake;
};

// --- Classe Block (modifiée pour Q3) ---
class Block {
private:
    uint32_t _nIndex;
    std::string _sData;
    time_t _tTime;
    std::string _sValidatorAddress; // Adresse du validateur choisi

    int64_t _nNonce;
    HashMethod _hMethod;

    // Fonction de hachage privée (pour PoS)
    std::string _CalculateHash() const {
        std::string ss = std::to_string(_nIndex) + std::to_string(_tTime) + _sData + sPrevHash + _sValidatorAddress;
        
        switch(_hMethod) {
            case HashMethod::AC_HASH:
                return ac_hash(ss, 30, 128);
            case HashMethod::SHA256:
            default:
                return sha256(ss);
        }
    }

public:
    std::string sPrevHash;
    std::string sHash;

    Block(uint32_t nIndexIn, const std::string &sDataIn, HashMethod method) 
        : _nIndex(nIndexIn), _sData(sDataIn), _tTime(time(nullptr)), _nNonce(0), _hMethod(method) {
    }

    // Fonction de validation PoS (remplace le minage PoW)
    void ValidateBlock(const std::string& validatorAddress) {
        _sValidatorAddress = validatorAddress;
        sHash = _CalculateHash();
    }

    // ============================================================================
    // --- OPTIMISATION (MineBlock) ---
    // ============================================================================
    /**
     * 3.2. Fonction de minage optimisée
     */
    void MineBlock(uint32_t nDifficulty) {
        std::string str(nDifficulty, '0');
        _nNonce = 0; 
        
        // --- OPTIMISATION 1: Créer la chaîne de base EN DEHORS de la boucle ---
        // Évite des millions d'allocations mémoire.
        std::string base_ss = std::to_string(_nIndex) + std::to_string(_tTime) + _sData + sPrevHash;
        
        do {
            _nNonce++; 
            
            // --- OPTIMISATION 1 (suite): N'ajoute que le nonce ---
            // C'est beaucoup plus rapide que de tout recréer.
            std::string ss = base_ss + std::to_string(_nNonce);
            
            // Q3.2: Sélectionne la bonne méthode de hachage
            switch(_hMethod) {
                case HashMethod::AC_HASH:
                    sHash = ac_hash(ss, 30, 128); 
                    break;
                case HashMethod::SHA256:
                default:
                    sHash = sha256(ss);
                    break;
            }

        // --- OPTIMISATION 2: Utilise sHash.compare() au lieu de sHash.substr() ---
        // .compare() n'alloue pas de nouvelle mémoire, .substr() le fait.
        } while (sHash.compare(0, nDifficulty, str) != 0);
    }
    // ============================================================================
    // --- FIN OPTIMISATION ---
    // ============================================================================


    /**
     * Q3.3: Fonction de recalcul du hash (nécessaire pour la validation)
     */
    std::string recalculatePoWHash() const {
        // L'optimisation n'est pas critique ici car ce n'est pas une boucle,
        // mais utiliser stringstream est plus propre.
        std::stringstream ss;
        ss << _nIndex << _tTime << _sData << sPrevHash << _nNonce;
        std::string data = ss.str();
        
        // Doit utiliser la MÊME méthode que celle utilisée pour le minage
        switch(_hMethod) {
            case HashMethod::AC_HASH:
                return ac_hash(data, 30, 128);
            case HashMethod::SHA256:
            default:
                return sha256(data);
        }
    }
};


// Classe Blockchain (modifiée pour Q3)
class Blockchain {
private:
    std::vector<Block> _vChain;
    std::vector<Validator> _vValidators;
    HashMethod _hMethod; // Q3.1: La chaîne connaît sa méthode

    const Block& _GetLastBlock() const {
        return _vChain.back();
    }

    // ... (votre fonction SelectValidator reste inchangée) ...
    Validator& SelectValidator() {
        double totalStake = 0.0;
        for (const auto& v : _vValidators) { totalStake += v.stake; }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distrib(0, totalStake);
        double randomPoint = distrib(gen);
        double currentSum = 0.0;
        for (auto& v : _vValidators) {
            currentSum += v.stake;
            if (randomPoint <= currentSum) { return v; }
        }
        return _vValidators.back();
    }


public:
    // Q3.1: Le constructeur choisit la méthode de hachage
    Blockchain(HashMethod method) : _hMethod(method) {
        // Crée le bloc Genesis avec la bonne méthode
        Block genesisBlock(0, "Genesis Block", _hMethod);
        std::cout << "Minage du bloc Genesis (difficulte 1)..." << std::endl;
        // Le bloc Genesis doit être valide pour que la chaîne soit valide
        genesisBlock.MineBlock(1); 
        _vChain.push_back(genesisBlock);
    }

    // Ajouter des validateurs au réseau
    void AddValidator(const std::string& address, double stake) {
        _vValidators.push_back({address, stake});
    }

    // Ajout d'un bloc avec PoS
    // (Modifié pour créer le bloc en interne)
    void AddBlockPoS(const std::string& sData) {
        if (_vValidators.empty()) {
            std::cout << "Erreur: Aucun validateur dans le reseau !" << std::endl;
            return;
        }
        // Crée le bloc avec la méthode de la chaîne
        Block bNew(_vChain.size(), sData, _hMethod);

        Validator& chosenValidator = SelectValidator();
        std::cout << "Validateur choisi: " << chosenValidator.address << " (Enjeu: " << chosenValidator.stake << ")" << std::endl;
        
        bNew.sPrevHash = _GetLastBlock().sHash;
        bNew.ValidateBlock(chosenValidator.address);
        _vChain.push_back(bNew);
    }

    // Ajout d'un bloc avec PoW
    // (Modifié pour créer le bloc en interne)
    void AddBlockPoW(const std::string& sData, uint32_t difficulty) {
        // Crée le bloc avec la méthode de la chaîne
        Block bNew(_vChain.size(), sData, _hMethod);

        bNew.sPrevHash = _GetLastBlock().sHash;

        std::string methodName = (_hMethod == HashMethod::AC_HASH ? "AC_HASH" : "SHA256");
        std::cout << "Minage du bloc " << _vChain.size() << " avec " 
                  << methodName << " (diff=" << difficulty << ")..." << std::endl;
        
        bNew.MineBlock(difficulty); // Appelle la fonction optimisée
        
        std::cout << "Bloc mine: " << bNew.sHash << std::endl;
        _vChain.push_back(bNew);
    }

    /**
     * 3.3. Vérifie que la validation de bloc reste fonctionnelle
     */
    bool isChainValidPoW() const {
        for (size_t i = 1; i < _vChain.size(); ++i) {
            const Block& currentBlock = _vChain[i];
            const Block& previousBlock = _vChain[i - 1];

            // 1. Vérifie si le hash stocké est le bon (en le recalculant)
            if (currentBlock.sHash != currentBlock.recalculatePoWHash()) {
                std::cout << "Validation echouee (Hash incorrect): Bloc " << i << std::endl;
                std::cout << "Attendu: " << currentBlock.recalculatePoWHash() << std::endl;
                std::cout << "Obtenu:  " << currentBlock.sHash << std::endl;
                return false;
            }

            // 2. Vérifie si le bloc pointe bien vers le hash précédent
            if (currentBlock.sPrevHash != previousBlock.sHash) {
                std::cout << "Validation echouee (Chaine rompue): Bloc " << i << std::endl;
                return false;
            }
        }
        return true;
    }
};


// --- Main (modifié pour tester la Q3) ---
int main() {
    // Difficulté pour le minage
    // Mettez 3 si 4 est trop lent
    uint32_t difficulty = 3; 

    std::cout << "--- TEST D'INTEGRATION AC_HASH (Q3) ---" << std::endl;
    
    // Q3.1: On choisit AC_HASH au lancement
    Blockchain bChain(HashMethod::AC_HASH);

    // Q3.2: Ajout de blocs minés avec AC_HASH
    bChain.AddBlockPoW("Donnees de transaction 1", difficulty);
    bChain.AddBlockPoW("Donnees de transaction 2", difficulty);
    
    std::cout << "\n----------------------------------------\n" << std::endl;
    
    // Q3.3: Vérification de la validation
    std::cout << "Verification de la validite de la chaine (PoW)..." << std::endl;
    if (bChain.isChainValidPoW()) {
        std::cout << "VERIFICATION REUSSIE : La blockchain est valide." << std::endl;
    } else {
        std::cout << "VERIFICATION ECHOUEE : La blockchain est invalide !" << std::endl;
    }
    
    return 0;
}