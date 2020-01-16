#include "bitboard_lookup.h"
#include "move_validator.h"
#include "position.h"
#include "ortho_position.h"
#include "atomic_position.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class SingleTest {
    /// Class representing a single test (position) from a single line in EPD.
    /// 
    public:
    std::string strFen;
    std::vector<int> depths;
    std::vector<uint64_t> correctPerfts;
    MoveValidator arbiter {ORTHO};
    
    SingleTest(std::istringstream& issline, Variant var) : arbiter(var) {
        /// Parse a single line passed from EPD.
        /// Each line should consist of the full FEN description of the position
        /// followed by substrings of the form "D[depth] [perft]", separated by
        /// semicolons ";".
        
        std::string str;
        // Set FEN string
        std::getline(issline, strFen, ';');
        
        while (std::getline(issline, str, ';')) {
            int iD = str.find('D');
            int ispace = str.find(' ');
            std::string strDepth = str.substr(iD + 1, ispace - iD);
            std::string strCorrectPerft = str.substr(ispace + 1);
            depths.push_back(std::stoi(strDepth));
            correctPerfts.push_back(std::stoull(strCorrectPerft));
        }
    }
    
    bool run(int maxDepth) {
        /// Runs perft to all depths smaller than maxDepth, printing results.
        /// 
        // TODO: can separate printing from logic.
        bool isTestCorrect = true;
        std::cout << "Position: " << strFen << "\n";
        OrthoPosition pos;
        int size = depths.size();
        for (int i = 0; i < size ; ++i) {
            if (depths[i] > maxDepth) {
                continue;
            }
            pos.fromFen(strFen);
            uint64_t res = arbiter.perft(depths[i], pos);
            uint64_t check = correctPerfts[i];
            std::cout << "perft at depth " << std::to_string(depths[i]) << ": "
                      << std::to_string(res)
                      << " (" << std::to_string(check) << ")" << "\n";
            
            if (res != check) {
                isTestCorrect = false;
                break;
            }
        }
        return isTestCorrect;
    }
};


int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        std::cout << "Run the perft tests with the command [filename] "
            "[EPD file path] [Maximum depth]\n" "Optional argument [] for atomic.\n";
        return 0;
    }
    
    // Open EPD file.
    std::string epdFile {argv[1]};
    std::ifstream testSuite;
    testSuite.open(epdFile);
    
    // Setup
    int maxDepth {std::atoi(argv[2])};
    std::string strTest;
    int testId = 0;
    int numTests = 0;
    std::vector<int> idFails;
    Variant var {argc == 3 ? ORTHO : ATOMIC};
    
    initialiseBbLookup();
    
    auto timeStart = std::chrono::steady_clock::now();
    // Run each test in the testSuite (parsed from EPD).
    while (std::getline(testSuite, strTest)) {
        ++numTests;
        ++testId;
        bool isTestCorrect = true;
        std::istringstream iss {strTest};
        SingleTest test {iss, var};
        std::cout << "======= Test " << std::to_string(testId) << " =======\n";
        isTestCorrect = test.run(maxDepth);
        if (!isTestCorrect) {
            idFails.push_back(testId);
        }
        std::cout << "\n";
    }
    auto timeEnd = std::chrono::steady_clock::now();
    auto timeTaken = timeEnd - timeStart;
    testSuite.close();
    
    // Print testing summary
    int numFails = idFails.size();
    float passRate = 100 * static_cast<float>(numTests - numFails) / static_cast<float>(numTests);
    std::cout << "\n======= Summary =======\n";
    std::cout << "Passrate = " << std::to_string(passRate) << "%\n";
    if (idFails.size() > 0) {
        std::cout << "Failed tests:";
        for (int idFail: idFails) {
            std::cout << " " << std::to_string(idFail);
        }
        std::cout << "\n";
    }
    std::cout << std::chrono::duration <double, std::milli> (timeTaken).count() << " ms\n";
    return 0;
}
