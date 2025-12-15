#pragma once
#include <array>
#include <utility>

#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "Side.h"

class Bishop : public ChessPiece {
public:
    Bishop(const Notation& startPos, Side color);

    virtual bool threatens(const Notation& position) const override;
    virtual bool canReach(const Notation& newPos) const override;

private:
    const std::array<std::pair<int, int>, 4> m_validMoves {
        {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}}
    };
};