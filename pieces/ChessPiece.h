#pragma once
#include <array>

#include "notation/Notation.h"
#include "Side.h"

class ChessPiece {
public:
    ChessPiece(const Notation& startPos, Side color);
    virtual ~ChessPiece() = default;
    
    virtual bool threatens(const Notation& newPos) const = 0;
    virtual bool canReach(const Notation& newPos) const = 0;

    Notation getNotation() const;
    Side getColor() const;

    inline static std::array<std::array<bool, 8>, 8> isOccupied = { false };

protected:
    Notation m_currentPos;
    Side m_color;
};