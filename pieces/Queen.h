#pragma once
#include "notation/Notation.h"
#include "pieces/Bishop.h"
#include "pieces/ChessPiece.h"
#include "pieces/Rook.h"
#include "Side.h"

class Queen : public ChessPiece {
public:
    Queen(const Notation& startPos, Side color);

    virtual bool threatens(const Notation& newPos) const override;

private:
    virtual bool canReach(const Notation& newPos) const override;
};