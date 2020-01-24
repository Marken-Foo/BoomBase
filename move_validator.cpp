#include "move_validator.h"

#include "atomic_move_rules.h"
#include "chess_types.h"
#include "move.h"
#include "ortho_move_rules.h"
#include "position.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>


void MoveValidator::setVariant(Variant var) {
    // TODO: avoid wasteful creation/deletion of MoveRules objects
    // Dependency injection?
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
    // Recurse.
    for (int i = 0; i < sz; ++i) {
        pos.makeMove(mvlist[i]);
        int childN = perft(depth-1, pos);
        nodes += childN;
        pos.unmakeMove(mvlist[i]);
    }
    return nodes;
}

std::vector<std::pair<Move, uint64_t> > MoveValidator::perftSplit(int depth, Position& pos) {
    uint64_t nodes = 0;
    Movelist mvlist = generateLegalMoves(pos);
    int sz = mvlist.size();
    std::vector<std::pair<Move, uint64_t> > res {};
    for (int i = 0; i < sz; ++i) {
        Move mv = mvlist[i];
        pos.makeMove(mv);
        uint64_t splitRes = perft(depth - 1, pos);
        pos.unmakeMove(mv);
        res.emplace_back(std::pair<Move, uint64_t>(mv, splitRes));
    }
    return res;
}

