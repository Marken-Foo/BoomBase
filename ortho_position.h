#ifndef ORTHO_POSITION_INCLUDED
#define ORTHO_POSITION_INCLUDED

#include "position.h"
#include "move.h"

class OrthoPosition : public Position {
    public:
    void makeMove(Move mv) override;
    void unmakeMove(Move mv) override;
    void reset() override;
};

#endif //#ifndef ORTHO_POSITION_INCLUDED