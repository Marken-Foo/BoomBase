#include "bitboard.h"
#include "chess_types.h"

#include <string>

const std::string pretty(Bitboard bb) {
    std::string outStr {"+--------+\n"};
    for (int y = 7; y >= 0; y--) {
        outStr.push_back('|');
        for (int x = 0; x < 8; ++x) {
            outStr.push_back((bb & square(x, y)) ? 'X' : '.');
        }
        outStr += "|\n";
    }
    outStr += "+--------+\n";
    return outStr;
}
