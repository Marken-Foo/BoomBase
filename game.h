#ifndef GAME_INCLUDED
#define GAME_INCLUDED

#include "movetree.h"

#include <memory>
#include <string>


class Game {
    // separate headers data into separate struct?
    public:
    // Seven Tag Roster
    std::string whitePlayer;
    std::string blackPlayer;
    std::string event;
    std::string site;
    std::string round;
    // Date date;
    int result;
    // Common tags
    int whiteRating {};
    int blackRating {};
    std::string annotator;
    
    std::unique_ptr<Position> pos;
    MoveTree mvtree;
    
    // What functions do we want here?
    /*
    // query mvtree current node
    getComment();
    getMove();
    getNags();
    
    // traverse mvtree
    AdvancePosition();
    AdvancePosition(Move mv); // advances if exists, adds move if not
    RetractPosition();
    AdvanceToVariation(int varNum);
    GoToPly(int plyNum);
    GoToEnd();
    GoToStart();
    
    // change mvtree (most of these call functions in mvtree) (on current node)
    AddMove(Move mv);
    DeleteMove(); //truncates move tree
    PromoteVariation();
    PromoteToMainVariation();
    
    // utility
    getFen(); // calls from Position class!
    */
};


#endif //#ifndef GAME_INCLUDED