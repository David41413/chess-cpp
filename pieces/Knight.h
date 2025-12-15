#pragma once
#include <array>
#include <utility>

#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "Side.h"

class Knight : public ChessPiece {
public:
    Knight(const Notation& startPos, Side color);

    virtual bool threatens(const Notation& position) const override;

private:
    const std::array<std::pair<int, int>, 8> m_validMoves {
        {{1, 2}, {2, 1}, {1, -2}, {2, -1},
        {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}}
    };

    virtual bool canReach(const Notation& newPos) const override;
};