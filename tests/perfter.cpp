#include "bitboard_lookup.h"
#include "move_validator.h"
#include "position.h"
#include "atomic_position.h"
#include "atomic_capture_masks.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

/// Rudimentary console I/O to allow testing of atomic chess perft values.
///

int main() {
    initialiseBbLookup();
    initialiseAtomicMasks();
    
    MoveValidator arbiter;
    std::unique_ptr<Position> pos = std::make_unique<AtomicPosition>();
    arbiter.setVariant(ATOMIC);
    
    while (true) {
        std::cout << "Enter FEN position:\n";
        std::string strFen;
        std::getline(std::cin, strFen);
        if (strFen == "exit") {break;}
        
        pos->fromFen(strFen);
        while (true) {
            std::cout << "Enter depth (15 or less):\n";
            std::string str;
            std::getline(std::cin, str);
            int depth = std::stoi(str);
            if (depth > 15) {break;}
            
            std::cout << "Calculating...\r";
            std::vector<std::pair<Move, uint64_t> > res = arbiter.perftSplit(depth, *pos);
            
            std::cout << pos->pretty();
            std::cout << "Perft result for depth " << std::to_string(depth)
                      << ":\n";
            for (std::pair<Move, uint64_t> split : res) {
                std::cout << toString(split.first) << ": "
                          << std::to_string(split.second) << "\n";
            }
        }
    }
    return 0;
}