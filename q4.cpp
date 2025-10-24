#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <numeric> 
#include <random>
#include <stdexcept>
#include <sstream>
#include <iomanip> // Pour std::setw, std::setprecision, std::fixed

// --- 1. Inclusions des fichiers HPP ---
// (Au lieu de coller le code)
#include "sha256.hpp"
#include "ac_hash.hpp"
// ------------------------------------


// --- Enum et Structs ---
enum class HashMethod {
    SHA256,
    AC_HASH
};
struct Validator {
    std::string address;
    double stake;
};
// --- FIN Enum et Structs ---


// --- Classe Block (modifiée pour Q4) ---
class Block {
private:
    uint32_t _nIndex;
    std::string _sData;
    time_t _tTime;
    std::string _sValidatorAddress; 
    int64_t _nNonce;                
    HashMethod _hMethod;            

    std::string _CalculateHashPoS() const {
        std::string ss = std::to_string(_nIndex) + std::to_string(_tTime) + _sData + sPrevHash + _sValidatorAddress;
        switch(_hMethod) {
            case HashMethod::AC_HASH: return ac_hash(ss, 30, 128); 
            case HashMethod::SHA256: default: return sha256(ss);
        }
    }

public:
    std::string sPrevHash;
    std::string sHash;

    Block(uint32_t nIndexIn, const std::string &sDataIn, HashMethod method) 
        : _nIndex(nIndexIn), _sData(sDataIn), _tTime(time(nullptr)), _nNonce(0), _hMethod(method) {
    }

    void ValidateBlock(const std::string& validatorAddress) {
        _sValidatorAddress = validatorAddress;
        sHash = _CalculateHashPoS();
    }

    void MineBlock(uint32_t nDifficulty) {
        std::string str(nDifficulty, '0');
        _nNonce = 0; 
        do {
            _nNonce++;
            std::string ss = std::to_string(_nIndex) + std::to_string(_tTime) + _sData + sPrevHash + std::to_string(_nNonce);
            switch(_hMethod) {
                case HashMethod::AC_HASH: sHash = ac_hash(ss, 30, 128); break;
                case HashMethod::SHA256: default: sHash = sha256(ss); break;
            }
        } while (sHash.substr(0, nDifficulty) != str);
    }

    std::string recalculatePoWHash() const {
        std::string ss = std::to_string(_nIndex) + std::to_string(_tTime) + _sData + sPrevHash + std::to_string(_nNonce);
        switch(_hMethod) {
            case HashMethod::AC_HASH: return ac_hash(ss, 30, 128);
            case HashMethod::SHA256: default: return sha256(ss);
        }
    }

    // --- AJOUT POUR Q4.2 ---
    /**
     * @brief Retourne le nombre d'itérations (nonce) utilisées pour miner ce bloc.
     */
    int64_t getNonce() const {
        return _nNonce;
    }
    // --- FIN AJOUT Q4.2 ---
};


// --- Classe Blockchain (modifiée pour Q4) ---
class Blockchain {
private:
    std::vector<Block> _vChain;
    std::vector<Validator> _vValidators;
    HashMethod _hMethod; 

    const Block& _GetLastBlock() const {
        return _vChain.back();
    }

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
    Blockchain(HashMethod method) : _hMethod(method) {
        Block genesisBlock(0, "Genesis Block", _hMethod);
        genesisBlock.MineBlock(1); // Mine le bloc Genesis avec difficulte 1
        _vChain.push_back(genesisBlock);
    }

    void AddValidator(const std::string& address, double stake) {
        _vValidators.push_back({address, stake});
    }

    void AddBlockPoS(const std::string& sData) {
        if (_vValidators.empty()) { return; }
        Block bNew(_vChain.size(), sData, _hMethod); 
        Validator& chosenValidator = SelectValidator();
        bNew.sPrevHash = _GetLastBlock().sHash;
        bNew.ValidateBlock(chosenValidator.address);
        _vChain.push_back(bNew);
    }

    // --- MODIFIÉ POUR Q4.1 et Q4.2 ---
    /**
     * @brief Ajoute un bloc PoW et retourne le nombre d'itérations (nonce) utilisées.
     */
    int64_t AddBlockPoW(const std::string& sData, uint32_t difficulty) {
        Block bNew(_vChain.size(), sData, _hMethod);
        bNew.sPrevHash = _GetLastBlock().sHash;
        
        std::string methodName = (_hMethod == HashMethod::AC_HASH ? "AC_HASH" : "SHA256");
        std::cout << "Minage bloc " << _vChain.size() << " (" << methodName << ")... ";

        bNew.MineBlock(difficulty); 
        
        std::cout << "OK (Nonce=" << bNew.getNonce() << ")" << std::endl;
        _vChain.push_back(bNew);
        
        // Retourne le nombre d'itérations
        return bNew.getNonce();
    }
    // --- FIN MODIFICATION Q4 ---


    bool isChainValidPoW() const {
        for (size_t i = 1; i < _vChain.size(); ++i) {
            const Block& currentBlock = _vChain[i];
            const Block& previousBlock = _vChain[i - 1];
            if (currentBlock.sHash != currentBlock.recalculatePoWHash()) {
                return false;
            }
            if (currentBlock.sPrevHash != previousBlock.sHash) {
                return false;
            }
        }
        return true;
    }
};


// --- FONCTION MAIN (Réponse à la Q4) ---
int main() {
    // Paramètres du test
    const int num_blocks_to_test = 10; // 4.1: Minage de 10 blocs
    const uint32_t difficulty = 4;     // 4.2: Difficulté fixe
    
    std::cout << "--- DEBUT DU TEST DE PERFORMANCE (Q4) ---" << std::endl;
    std::cout << "Parametres: " << num_blocks_to_test << " blocs, difficulte = " << difficulty << std::endl;

    // --- Test 1: SHA256 ---
    std::cout << "\n--- Test 1: SHA256 ---" << std::endl;
    Blockchain bChainSHA256(HashMethod::SHA256);
    int64_t total_nonces_sha256 = 0;
    
    auto t_start_sha256 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_blocks_to_test; ++i) {
        // 4.2: Mesure le nombre moyen d'itérations
        total_nonces_sha256 += bChainSHA256.AddBlockPoW("Bloc de test SHA256", difficulty);
    }
    auto t_end_sha256 = std::chrono::high_resolution_clock::now();
    
    // 4.1: Mesure le temps moyen de minage
    double time_taken_sha256 = std::chrono::duration<double>(t_end_sha256 - t_start_sha256).count();
    double avg_time_sha256 = time_taken_sha256 / num_blocks_to_test;
    double avg_nonces_sha256 = static_cast<double>(total_nonces_sha256) / num_blocks_to_test;


    // --- Test 2: AC_HASH ---
    std::cout << "\n--- Test 2: AC_HASH (Rule 30, 128 steps) ---" << std::endl;
    Blockchain bChainAC(HashMethod::AC_HASH);
    int64_t total_nonces_ac = 0;

    auto t_start_ac = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_blocks_to_test; ++i) {
        // 4.2: Mesure le nombre moyen d'itérations
        total_nonces_ac += bChainAC.AddBlockPoW("Bloc de test AC_HASH", difficulty);
    }
    auto t_end_ac = std::chrono::high_resolution_clock::now();

    // 4.1: Mesure le temps moyen de minage
    double time_taken_ac = std::chrono::duration<double>(t_end_ac - t_start_ac).count();
    double avg_time_ac = time_taken_ac / num_blocks_to_test;
    double avg_nonces_ac = static_cast<double>(total_nonces_ac) / num_blocks_to_test;


    // --- 4.3: Donne les résultats dans un tableau ---
    std::cout << "\n--- RESULTATS DE LA COMPARAISON (Q4.3) ---" << std::endl;
    std::cout << std::fixed << std::setprecision(4); // 4 décimales pour le temps
    
    std::cout << "+------------+----------------------+----------------------+" << std::endl;
    std::cout << "| Metrique   | SHA256               | AC_HASH (Rule 30)    |" << std::endl;
    std::cout << "+------------+----------------------+----------------------+" << std::endl;
    
    std::cout << "| Temps total| " << std::setw(20) << time_taken_sha256 << " s | " 
              << std::setw(20) << time_taken_ac << " s |" << std::endl;
              
    std::cout << "| Temps moyen| " << std::setw(20) << avg_time_sha256 << " s | " 
              << std::setw(20) << avg_time_ac << " s |" << std::endl;
              
    std::cout << "| Iter. moy. | " << std::setw(20) << std::setprecision(0) << avg_nonces_sha256 << " | " 
              << std::setw(20) << std::setprecision(0) << avg_nonces_ac << " |" << std::endl;
              
    std::cout << "+------------+----------------------+----------------------+" << std::endl;

    return 0;
}