#include "pieces/King.h"

#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "Side.h"

King::King(const Notation& startPos, Side color)
    : ChessPiece{startPos, color}
{}

bool King::threatens(const Notation& position) const {
    if(!position.onBoard()) { return false; }

    for(const auto& [fileOffset, rankOffset] : m_validMoves) {
        int fileInt{ static_cast<int>(m_currentPos.getFile()) + fileOffset};
        Notation threatened{ static_cast<char>(fileInt),
                             m_currentPos.getRank() + rankOffset };

        if(threatened == position) {
            return true;
        }
    }
    return false;
}

bool King::canReach(const Notation& newPos) const {
    for(const auto& [fileOffset, rankOffset] : m_validMoves) {
        int fileInt{ static_cast<int>(m_currentPos.getFile()) + fileOffset};
        Notation moves{ static_cast<char>(fileInt),
                        m_currentPos.getRank() + rankOffset };

        if(moves == newPos) {
            return true;
        }
    }
    return false;
}