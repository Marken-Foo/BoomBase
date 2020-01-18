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
            uint64_t res = arbiter.perft(depth, *pos);
            
            std::cout << pos->pretty();
            std::cout << "Perft result for depth " << std::to_string(depth) << ": " << std::to_string(res) << "\n";
        }
    }
    return 0;
}