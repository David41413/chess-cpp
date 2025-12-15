#include "pieces/Pawn.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include "game/Board.h"
#include "game/MoveHandler.h"
#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "Side.h"

using PieceIter = std::vector<std::unique_ptr<ChessPiece>>::iterator;
using ConstPieceIter = std::vector<std::unique_ptr<ChessPiece>>::const_iterator;

Pawn::Pawn(const Notation& startPos, Side color)
    : ChessPiece{startPos, color}
{}

bool Pawn::threatens(const Notation& position) const {
    if(!position.onBoard()) { return false; }

    for(const auto& [fileOffset, rankOffset] : m_captureMoves) {
        int fileInt{ static_cast<int>(m_currentPos.getFile()) + fileOffset };
        Notation threatened{ static_cast<char>(fileInt),
                             m_currentPos.getRank() + rankOffset };
        if(threatened == position) {
            return true;
        }
    }
    return false;
}

void Pawn::moveTo(const Notation& newPos, MoveHandler& mv, const Board& pieces) {
    if(m_firstMove) {
        m_firstMove = false;
    }
    if(mv.validCapture(newPos, *this, pieces)) {
        fiftyMoveCounter = 0;
    }

    isOccupied[m_currentPos.getRank() - 1][m_currentPos.getFile() - 'a'] = false;
    m_currentPos = newPos;
    isOccupied[m_currentPos.getRank() - 1][m_currentPos.getFile() - 'a'] = true;
}

PieceIter Pawn::getAdjacentPawnPtr(Board& board) const {
    return std::find_if(
        board.getPieces(m_color).begin(),
        board.getPieces(m_color).end(),
        [this](const std::unique_ptr<ChessPiece>& piece) {
            return dynamic_cast<Pawn*>(piece.get()) != nullptr &&
                   std::abs(piece->getNotation().getFile() - this->getNotation().getFile()) == 1 &&
                   piece->getNotation().getRank() == this->getNotation().getRank();
        }
    );
}

bool Pawn::canEnPassant(PieceIter pawnPtr, const Board& board) {
    if(!this->getNotation().onBoard() || !pawnPtr->get()->getNotation().onBoard()) {
        return false;
    }
    if(m_enPassantChoice) {
        std::cout << "En passant not possible. Please try again.\n";
        return false;
    }
    else {
        m_enPassantChoice = false;
    }
    if(this->getNotation().getRank() != ((m_color == Side::White) ? 5 : 4)) {
        std::cout << "En passant not possible. Please try again.\n";
        return false;
    }
    if(pawnPtr != board.getPieces(m_color).end()) {
        Pawn* pawn{ dynamic_cast<Pawn*>(pawnPtr->get()) };
        if(pawn->getFirstMove()) {
            std::cout << "En passant not possible. Please try again.\n";
            return false;
        }
    }

    return true;
}

void Pawn::enPassant(PieceIter pawnPtr, MoveHandler& mv, const Board& board) {
    if(pawnPtr != board.getPieces(m_color).end()) {
        Pawn* pawn{ dynamic_cast<Pawn*>(pawnPtr->get()) };
        Notation capturedPos{ (*pawnPtr)->getNotation() };
        pawnPtr->reset();
        isOccupied[capturedPos.getRank() - 1][capturedPos.getFile() - 'a'] = false;
            
        this->moveTo(Notation(this->getNotation().getFile(), 
                             (m_color == Side::White) ? 6 : 3),
                     mv,
                     board
                    );
    }
}

bool Pawn::canReach(const Notation& newPos) const {
    int fileInt{ static_cast<int>(m_currentPos.getFile()) + m_validMoves.first };
    Notation moves{
        static_cast<char>(fileInt),
        m_currentPos.getRank() + m_validMoves.second
    };

    if(moves == newPos) {
        return true;
    }
    if(m_firstMove) {
        int fileInt{ static_cast<int>(m_currentPos.getFile()) + m_validMoves.first * 2 };
        Notation doubleMove{
            static_cast<char>(fileInt),
            m_currentPos.getRank() + m_validMoves.second * 2
        };
        if(doubleMove == newPos) {
            return true;
        }
    }
    return false;
}