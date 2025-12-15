#pragma once
#include <array>
#include <iostream>
#include <utility>

#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "Side.h"

class Rook : public ChessPiece {
public:
    Rook(const Notation& startPos, Side color);

    virtual bool threatens(const Notation& position) const override;
    virtual bool canReach(const Notation& newPos) const override;

private:
    const std::array<std::pair<int, int>, 4> m_validMoves {
        {{1, 0}, {0, 1}, {-1, 0}, {0, -1}}
    };
};