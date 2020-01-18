#include "move_validator.h"

#include "atomic_move_rules.h"
#include "chess_types.h"
#include "move.h"
#include "ortho_move_rules.h"
#include "position.h"

#include <cstdint>
#include <memory>

void MoveValidator::setVariant(Variant var) {
    // TODO: avoid wasteful creation/deletion of MoveRules objects
    if (var == ORTHO) {
        rules.reset(new OrthoMoveRules());
    } else if (var == ATOMIC) {
        rules.reset(new AtomicMoveRules());
    }
    currentVariant = var;
    return;
}

uint64_t MoveValidator::perft(int depth, Position& pos) {
    /// Recursive function to count all legal moves (nodes) at depth n.
    /// 
    uint64_t nodes = 0;
    // Terminating condition
    if (depth == 0) {return 1;}
    
    Movelist mvlist = generateLegalMoves(pos);
    int sz = mvlist.size();
    //std::cout<<std::to_string(sz)<<"\n"; // DEBUGGING
    // Recurse.
    for (int i = 0; i < sz; ++i) {
        pos.makeMove(mvlist[i]);
        int childN = perft(depth-1, pos);
        nodes += childN;
        pos.unmakeMove(mvlist[i]);
    }
    return nodes;
}

