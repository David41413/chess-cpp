#include "pieces/ChessPiece.h"

#include "notation/Notation.h"
#include "Side.h"

ChessPiece::ChessPiece(const Notation& startPos, Side color)
    : m_currentPos{startPos}, m_color{color}
{}

Notation ChessPiece::getPosition() const { return m_currentPos; }
Side ChessPiece::getColor() const { return m_color; }