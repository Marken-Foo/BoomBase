#ifndef MOVETREE_INCLUDED
#define MOVETREE_INCLUDED

#include "move.h"

#include <deque>
#include <memory>
#include <set>
#include <string>
#include <utility> //move, swap

// MoveTree should be exposed.
// MoveNode should be hidden (implementation details)
typedef unsigned char Nag;

class MoveNode;
class MoveTree;

struct MoveInfo {
    // info acquirable from PGN
    Move mv {};
    std::set<Nag> nags {};
    std::string comment {};
};

class MoveNode {
    // No public interface; give only its wrapper permission to manipulate.
    friend class MoveTree;
    public:
    // TODO: Proper destructor for MoveNode; should iterate over its children
    ~MoveNode() = default;
    
    private:
    MoveNode() = default;
    MoveNode(Move mv)
        : move{mv}
    { }
    MoveNode(MoveInfo mvi)
        : move{mvi.mv}
        , nags{mvi.nags}
        , comment{mvi.comment}
    { }
    // MoveNode not simply copyable due to unique pointers.
    MoveNode(const MoveNode& other) = delete;
    MoveNode& operator=(const MoveNode& other) = delete;
    
    // TODO: should include FEN?
    // data members
    Move move {}; // The move that was just played to reach the node.
    // Principal variation is the 0th element (variations[0], if any).
    // Pointers in variations should never be nullptr.
    std::deque<std::unique_ptr<MoveNode> > variations {};
    MoveNode* prevMove {nullptr}; // non-owning
    std::set<Nag> nags {}; // keep sorted?
    std::string comment {};
    
    // methods
    bool isLeaf() const {
        return variations.size() == 0;
    }
    
    bool isRoot() const {
        return prevMove == nullptr;
    }
    
    auto next() {
        return isLeaf() ? this : variations[0].get();
    }
    
    auto previous() {
        return isRoot() ? this : prevMove;
    }
    
    auto getVariation(int varNum) const {
        return 0 <= varNum && varNum < variations.size()
            ? variations[varNum].get()
            : this;
    }
    
    // TODO: check to see if these variations already exist
    void addMove(MoveInfo mvi) {
        auto ptr {std::unique_ptr<MoveNode>(new MoveNode(mvi))};
        variations.push_back(std::move(ptr));
        return;
    }
    
    void addMove(Move mv) {
        auto ptr {std::unique_ptr<MoveNode>(new MoveNode(mv))};
        variations.push_back(std::move(ptr));
        return;
    }
    
    void promoteVariation(int varNum) {
        // Pushes the selected variation up in the list of variations.
        // TODO: check for valid varNum
        variations[varNum].swap(variations[varNum - 1]);
        return;
    }
    
    void promoteToMainline(int varNum) {
        // Makes the selected variation the principal variation (mainline).
        // TODO: check for valid varNum
        auto ptr {std::move(variations[varNum])};
        variations.erase(variations.begin() + varNum);
        variations.push_front(std::move(ptr));
        return;
    }
    
    // TODO: define Visitor accept() method.
};

class MoveTree {
    // Wrapper class for a root MoveNode, representing a move tree.
    public:
    // TODO: constructors and destructor (impt!) for actual MoveTree.
    // Null movetree (zero-length game) needs to have 1 node.
    // Root node of movetree is always present and contains no last move
    // information.
    MoveTree() {
        auto ptr {std::unique_ptr<MoveNode>(new MoveNode())};
        root = std::move(ptr);
        currentNode = root.get();
    }
    ~MoveTree() = default;
    MoveTree(const MoveTree& other) = delete;
    MoveTree& operator=(const MoveTree& other) = delete;
    
    private:
    // data members -- should never be nullptr if instantiated correctly
    std::unique_ptr<MoveNode> root {nullptr};
    MoveNode* currentNode {nullptr}; // Non-owning, used to traverse the tree.
    
    // public interface
    public:
    // query current node
    Move getMove() const {
        return currentNode->move;
    }
    
    std::string getComment() const {
        return currentNode->comment;
    }
    
    auto getNags() const {
        return currentNode->nags;
    }
    
    // traverse tree
    void stepForward() {
        currentNode = currentNode->next();
        return;
    }
    
    void stepBack() {
        currentNode = currentNode->previous();
        return;
    }
    
    void stepToVariation(int varNum) {
        currentNode = currentNode->getVariation(varNum);
        return;
    }
    
    void goToStart() {
        currentNode = root.get();
        return;
    }
    
    void goToEnd() {
        while (!(currentNode->isLeaf())) {
            currentNode = currentNode->next();
        }
        return;
    }
    
    void goToVariationStart() {
        // Backtracks to latest point where the branch we are on is not the main
        // variation.
        while (!(currentNode->isRoot())) {
            Move mv {getMove()};
            stepBack();
            if (mv != currentNode->variations[0]->move) {
                break;
            }
        }
        return;
    }
    
    void addMove(Move mv) {
        // Appends a (NEW!) move to the current node and goes to it.
        // ASSERT(mv not in currentNode->variations)
        currentNode->addMove(mv);
        currentNode = currentNode->variations.back();
        return;
    }
    
    void deleteMove() {
        // Deletes the current movenode and all child nodes.
        if (currentNode == root.get()) {
            // reset root node
            currentNode->variations.clear();
            return;
        }
        Move move {currentNode->move};
        currentNode = currentNode->previous();
        auto variations = currentNode->variations;
        for (auto it = variations.begin(); it != variations.end();) {
            if (move == variations[i]->move) {
                it = variations.erase(it);
                break; // assume at most one match
            } else {
                ++it;
            }
        }
        return;
    }
    
    /*
    // traverse mvtree
    stepForward(Move mv); // advances if exists, adds move if not
    GoToPly(int plyNum); // necessary or not?
    
    // change mvtree (most of these call functions in mvtree) (on current node)
    AddMove(Move mv);
    DeleteMove(); //truncates move tree
    PromoteVariation();
    PromoteToMainVariation();
    
    // utility
    getFen(); // calls from Position class!
    */
    
};

#endif //#MOVETREE_INCLUDED