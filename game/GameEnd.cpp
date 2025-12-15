#include "game/GameEnd.h"

#include <cstdlib>

#include <algorithm>
#include <iostream>
#include <memory>

#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "pieces/Pawn.h"
#include "Side.h"

GameEnd::GameEnd(Board& board) : m_board{ board } {}

bool GameEnd::canEscapeCheckmate(const std::unique_ptr<ChessPiece>& piece, Side color) {
    for(char file = 'a'; file <= 'h'; ++file) {
        for(int rank = 1; rank <= 8; ++rank) {
            Notation newPos{ file, rank };
            if(mh.validMove(newPos, *piece, m_board)) {
                Notation originalPos{ piece->getNotation() };
                mh.moveTo(newPos, *piece);
                bool safe{ m_board.kingSafe(color) };
                mh.moveTo(originalPos, *piece);
                if(safe) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool GameEnd::checkmate(Side color) {
    if(!std::any_of(
        m_board.getPieces(color).begin(),
        m_board.getPieces(color).end(),
        [&](const std::unique_ptr<ChessPiece>& piece){
            return canEscapeCheckmate(piece, color);
        })) {
        return true;
    }

    return false;
}

bool GameEnd::stalemate(Side color) {
    if(!std::any_of(
        m_board.getPieces(color).begin(),
        m_board.getPieces(color).end(),
        [this, color](const std::unique_ptr<ChessPiece>& piece){
            return moveExists(piece, color);
        })) {
        return true;
    }

    return false;
}

void GameEnd::didGameEnd(Side color) {
    if(!m_board.kingSafe(color) && checkmate(color)) {
        std::cout << "Checkmate! "
                    << ((color == Side::White) ? "White" : "Black")
                    << " wins the game!\n";
        std::exit(EXIT_SUCCESS);
    }
    else if(!m_board.kingSafe(color)) {
        std::cout << "Check!\n";
    }
    else if(stalemate(color)) {
        std::cout << "Stalemate! The game is a draw.\n";
        std::exit(EXIT_SUCCESS);
    }
    if(Pawn::fiftyMoveCounter <= 50) {
        std::cout << "Fifty-move rule reached! The game is a draw.\n";
        std::exit(EXIT_SUCCESS);
    }
}

bool GameEnd::moveExists(const std::unique_ptr<ChessPiece>& piece, Side color) {
    for(char file = 'a'; file <= 'h'; ++file) {
        for(int rank = 1; rank <= 8; ++rank) {
            Notation newPos{ file, rank };
            if(mh.validMove(newPos, *piece, m_board)) {
                return true;
            }
        }
    }
    return false;
}