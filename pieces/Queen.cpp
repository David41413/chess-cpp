#include "pieces/Queen.h"

#include "notation/Notation.h"
#include "pieces/Bishop.h"
#include "pieces/ChessPiece.h"
#include "pieces/Rook.h"
#include "Side.h"

Queen::Queen(const Notation& startPos, Side color)
    : ChessPiece{startPos, color}
{}

bool Queen::threatens(const Notation& newPos) const {
    return Rook{m_currentPos, m_color}.threatens(newPos) ||
           Bishop{m_currentPos, m_color}.threatens(newPos);
}

bool Queen::canReach(const Notation& newPos) const {
    return Rook{m_currentPos, m_color}.canReach(newPos) ||
           Bishop{m_currentPos, m_color}.canReach(newPos);
}