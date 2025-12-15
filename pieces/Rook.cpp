#include "pieces/Rook.h"

#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "Side.h"

Rook::Rook(const Notation& startPos, Side color)
    : ChessPiece{startPos, color}
{}

bool Rook::threatens(const Notation& position) const {
    if(!position.onBoard()) { return false; }
    
    for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
        int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffsets;
        Notation threatened{ static_cast<char>(fileInt),
                             m_currentPos.getRank() + rankOffsets };
    
        while(threatened != position &&
              !isOccupied[threatened.getRank() - 1][threatened.getFile() - 'a'] &&
              threatened.onBoard()) {
            int fileInt{ static_cast<int>(threatened.getFile()) + fileOffsets };
            threatened = {static_cast<char>(fileInt), threatened.getRank() + rankOffsets};
        }
        if(threatened == position) {
            return true;
        }
    }
    return false;
}

bool Rook::canReach(const Notation& newPos) const {
    for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
        int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffsets;
        Notation moves{ static_cast<char>(fileInt),
                        m_currentPos.getRank() + rankOffsets };

        while(moves != newPos &&
              !isOccupied[moves.getRank() - 1][moves.getFile() - 'a'] &&
              moves.onBoard()) {
            int fileInt{ static_cast<int>(moves.getFile()) + fileOffsets };
            moves = { static_cast<char>(fileInt), moves.getRank() + rankOffsets};
        }
        if(moves == newPos) {
            return true;
        }
    }
    return false;
}