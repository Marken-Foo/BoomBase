#include "bitboard_lookup.h"
#include "move_validator.h"
#include "position.h"
#include "atomic_position.h"
#include "atomic_capture_masks.h"

#include <chrono>
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
            
            auto timeStart = std::chrono::steady_clock::now();
            std::vector<std::pair<Move, uint64_t> > res = arbiter.perftSplit(depth, *pos);
            auto timeEnd = std::chrono::steady_clock::now();
            auto timeTaken = timeEnd - timeStart;
            
            std::cout << pos->pretty();
            std::cout << "Perft result for depth " << std::to_string(depth)
                      << ":\n";
            uint64_t total {};
            for (std::pair<Move, uint64_t> split : res) {
                std::cout << toString(split.first) << ": "
                          << std::to_string(split.second) << "\n";
                total += split.second;
            }
            std::cout << "Total: " << std::to_string(total) << "\n";
            std::cout << "Time taken: " << std::chrono::duration<double, std::milli>(timeTaken).count() << " ms\n";
        }
    }
    return 0;
}