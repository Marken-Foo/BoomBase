#include "bitboard_lookup.h"
#include "chess_types.h"
#include "move.h"
#include "position.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// test format:
// [position];[fromSq,toSq,special,promopiece];[final position]

class SingleMoveTest {
    public:
    Position posTest;
    Move mv;
    
    std::string strFenBefore, strFenAfter;
    Position posBefore;
    Position posAfter;
    
    SingleMoveTest(std::istringstream& issline) {
        std::string strFromSq, strToSq, strSpecial, strPromoPiece;
        
        std::getline(issline, strFenBefore, ';');
        
        std::getline(issline, strFromSq, ' ');
        std::getline(issline, strToSq, ' ');
        std::getline(issline, strSpecial, ' ');
        std::getline(issline, strPromoPiece, ';');
        
        std::getline(issline, strFenAfter, ';');
        
        // initialise move
        Square fromSq = square(strFromSq);
        Square toSq = square(strToSq);
        PieceType pcty = NO_PCTY; // will error if falls to makePromotion
        
        if (strPromoPiece == "-") { //do nothing
        } else if (strPromoPiece == "N") {
            pcty = KNIGHT;
        } else if (strPromoPiece == "B") {
            pcty = BISHOP;
        } else if (strPromoPiece == "R") {
            pcty = ROOK;
        } else if (strPromoPiece == "Q") {
            pcty = QUEEN;
        }
        
        if (strSpecial == "-") {
            mv = buildMove(fromSq, toSq);
        } else if (strSpecial == "promo") {
            mv = buildPromotion(fromSq, toSq, pcty);
        } else if (strSpecial == "castle") {
            mv = buildCastling(fromSq, toSq);
        } else if (strSpecial == "ep") {
            mv = buildEp(fromSq, toSq);
        }
        
        posBefore.fromFen(strFenBefore);
        posAfter.fromFen(strFenAfter);
    }
    
    bool runMake() {
        bool isPassed = true;
        posTest.fromFen(strFenBefore);
        posTest.makeMove(mv);
        if (posTest != posAfter) {
            isPassed = false;
        }
        return isPassed;
    }
    
    bool runUnmake() {
        bool isPassed = true;
        posTest.fromFen(strFenBefore);
        posTest.makeMove(mv);
        posTest.unmakeMove(mv);
        if (posTest != posBefore) {
            isPassed = false;
        }
        return isPassed;
    }
    
    private:
    // To refactor for future use if needed
    Square square(std::string cn) {
        //assert xn is of form "e5" or "c1" etc.
        return ::square(static_cast<int>(cn[0] - 'a'),
                        static_cast<int>(cn[1] - '1'));
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Run the perft tests with the command [filename] "
                     "[EPD file path] [0 for Make, 1 for Unmake] "
                     "(all arguments required).\n";
        return 0;
    }
    
    // Open EPD file.
    std::string epdFile {argv[1]};
    std::ifstream testSuite;
    testSuite.open(epdFile);
    
    // Setup
    int mode {std::atoi(argv[2])};
    if (mode != 0 && mode != 1) {
        std::cout << "Invalid mode (Make = 0 / Unmake = 1).";
        return 0;
    }
    
    std::string strTest;
    int testId = 0;
    int numTests = 0;
    std::vector<int> idFails;
    
    initialiseBbLookup();
    
    // Run each test in the testSuite (parsed from EPD).
    while (std::getline(testSuite, strTest)) {
        ++numTests;
        ++testId;
        bool isTestCorrect = true;
        std::istringstream iss {strTest};
        SingleMoveTest test {iss};
        if (mode == 0) {
            isTestCorrect = test.runMake();
        } else if (mode == 1) {
            isTestCorrect = test.runUnmake();
        }
        if (!isTestCorrect) {
            idFails.push_back(testId);
        }
    }
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
    }
    return 0;
}


// Test equality function is working correctly
// > initialise 2 positions from FENs differing in 50/half counters (expect equal)
// > test that equality is reflexive and symmetric

// Changes needed for atomic chess:
// > Captures: check exploded pawns/nonpawns are correct.
// > Captures: check capturer/capturee pawns are destroyed.
// > Promocaps: same deal
// > Castlings: check exploded rooks cause castling rights to be lost.