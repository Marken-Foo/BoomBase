#include "bitboard_lookup.h"
#include "move_validator.h"
#include "position.h"
#include "ortho_position.h"
#include "atomic_position.h"
#include "atomic_capture_masks.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


class SinglePosition {
    public:
    std::string strFen;
    std::vector<uint64_t> perfts;
    MoveValidator arbiter;
    
    SinglePosition(std::istringstream& issline, Variant var)
        : arbiter(var)
    {
            strFen = issline.str();
    }
    
    void run(int maxDepth) {
        // Make position, then run perft to a specified depth, saving results.
        std::unique_ptr<Position> pos;
        Variant var = arbiter.getVariant();
        if (var == ORTHO) {
            pos.reset(new OrthoPosition);
        } else if (var == ATOMIC) {
            pos.reset(new AtomicPosition);
        }
        
        pos->fromFen(strFen);
        for (int depth = 1; depth <= maxDepth; ++depth) {
            perfts.push_back(arbiter.perft(depth, *pos));
        }
        return;
    }
    
    void write(std::ofstream& ofs) {
        ofs << strFen;
        int len = perfts.size();
        for (int i = 0; i < len; ++i) {
            ofs << " ;D" << std::to_string(i + 1);
            ofs << " " << std::to_string(perfts[i]); 
        }
        return;
    }
};


int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        std::cout << "Run the perft tests with the command [filename] "
            "[input file] [output file]\n" "Optional argument [maxDepth].\n";
        return 0;
    }
    
    // Open input file.
    std::string inputFile {argv[1]};
    std::ifstream ifs;
    ifs.open(inputFile);
    
    // Prepare output file.
    std::string outputFile {argv[2]};
    std::ofstream ofs;
    ofs.open(outputFile);
    
    // Setup, initialise everything needed
    int maxDepth = 6;
    if (argc == 4) {
        maxDepth = std::atoi(argv[3]);
    }
    std::string strTest;
    int testId = 0;
    std::vector<int> idFails;
    Variant var {argc == 3 ? ORTHO : ATOMIC};
    
    initialiseBbLookup();
    initialiseAtomicMasks();
    
    // run each perft, write results to output file
    while (std::getline(ifs, strTest)) {
        ++testId;
        std::istringstream iss {strTest};
        SinglePosition test {iss, var};
        std::cout << "Running position " << std::to_string(testId) << "...\r";
        test.run(maxDepth);
        test.write(ofs);
        ofs << "\n";
    }
    
    std::cout << "\n";
    ifs.close();
    ofs.close();
    
    return 0;
}