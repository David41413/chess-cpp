#pragma once
#include <array>
#include <utility>

#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "Side.h"

class King : public ChessPiece {
public:
    King(const Notation& startPos, Side color);

    virtual bool threatens(const Notation& position) const override;

private:
    const std::array<std::pair<int, int>, 8> m_validMoves {
        {{1, 0}, {0, 1}, {-1, 0}, {0, -1},
        {1, 1}, {-1, 1}, {-1, -1}, {1, -1}}
    };

    virtual bool canReach(const Notation& newPos) const override;
};