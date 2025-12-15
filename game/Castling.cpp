#include "game/Castling.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <memory>

#include "game/Board.h"
#include "game/MoveHandler.h"
#include "pieces/ChessPiece.h"
#include "pieces/Rook.h"
#include "notation/Notation.h"
#include "Side.h"

using ConstPieceIter = std::vector<std::unique_ptr<ChessPiece>>::const_iterator;

ConstPieceIter Castling::getRookPtr(const Notation& rookNotation, const Board& m_board, Side color) {
    return std::find_if(
        m_board.getPieces(color).begin(),
        m_board.getPieces(color).end(),
        [rookNotation](const std::unique_ptr<ChessPiece>& piece) {
            return dynamic_cast<Rook*>(piece.get()) != nullptr &&
                    piece->getNotation() == rookNotation;
        }
    );
}

void Castling::shortCastle(ConstPieceIter kingPtr, ConstPieceIter rookPtr, Side color) {
    Notation originalPos{ (*kingPtr)->getNotation() };

    for(int i = 0; i <= 1; ++i) {
        mh.moveTo(
            (color == Side::White) ?
            Notation(static_cast<char>((*kingPtr)->getNotation().getFile() + 1), 1) :
            Notation(static_cast<char>((*kingPtr)->getNotation().getFile() + 1), 8),
            **kingPtr
        );
        if(!m_board.kingSafe(color) ||
        ChessPiece::isOccupied[(*kingPtr)->getNotation().getRank() - 1]
                                [(*kingPtr)->getNotation().getFile() - 'a']) {
            std::cout << "Cannot castle through or into check. Please try again.\n";
            mh.moveTo(originalPos, **kingPtr);
            return;
        }
    }

    mh.moveTo((color == Side::White) ? Notation('f', 1) :
                        Notation('f', 8), **rookPtr);
}

void Castling::longCastle(ConstPieceIter kingPtr, ConstPieceIter rookPtr, Side color) {
    Notation originalPos{ (*kingPtr)->getNotation() };

    for(int i = 0; i <= 2; ++i) {
        mh.moveTo(
            (color == Side::White) ?
            Notation(static_cast<char>((*kingPtr)->getNotation().getFile() - 1), 1) :
            Notation(static_cast<char>((*kingPtr)->getNotation().getFile() - 1), 8),
            **kingPtr
        );
        if(!m_board.kingSafe(color) ||
            ChessPiece::isOccupied[(*kingPtr)->getNotation().getRank() - 1]
                                [(*kingPtr)->getNotation().getFile() - 'a']) {
            std::cout << "Cannot castle through or into check. Please try again.\n";
            mh.moveTo(originalPos, **kingPtr);
            return;
        }
    }

    mh.moveTo((color == Side::White) ? Notation('d', 1) :
                        Notation('d', 8), **rookPtr);
}

void Castling::castling(std::string_view castleType, Side color) {
    if(!m_board.kingSafe(color)) {
        std::cout << "Cannot castle while in check. Please try again.\n";
        return;
    }

    auto rookPtr {
        getRookPtr((castleType == "O-O") ?
        Notation('h', (color == Side::White) ? 1 : 8) :
        Notation('a', (color == Side::White) ? 1 : 8),
        m_board,
        color)
    };

    auto kingPtr{ m_board.getKingPtr(color) };

    if(rookPtr != m_board.getPieces(color).end()) {
        std::cout << "Rook has been moved or was taken. Cannot castle.\n";
        return;
    }

    if(kingPtr != m_board.getPieces(color).end() &&
    (*kingPtr)->getNotation() != Notation('e', (color == Side::White) ? 1 : 8)) {
        std::cout << "King has been moved or was taken. Cannot castle.\n";
        return;
    }

    if(castleType == "O-O" && ((color == Side::White) ?
    !ChessPiece::isOccupied[0][5 - 'a'] && !ChessPiece::isOccupied[0][6 - 'a'] :
    !ChessPiece::isOccupied[7][5 - 'a'] && !ChessPiece::isOccupied[7][6 - 'a'])) {
        shortCastle(kingPtr, rookPtr, color);
    }
    else if(castleType == "O-O-O" && ((color == Side::White) ?
            !ChessPiece::isOccupied[0][1 - 'a'] &&
            !ChessPiece::isOccupied[0][2 - 'a'] &&
            !ChessPiece::isOccupied[0][3 - 'a'] :
            !ChessPiece::isOccupied[7][1 - 'a'] &&
            !ChessPiece::isOccupied[7][2 - 'a'] &&
            !ChessPiece::isOccupied[7][3 - 'a'])) {
        longCastle(kingPtr, rookPtr, color);
    }
}