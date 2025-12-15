#include "game/MoveHandler.h"

#include <iostream>

#include "game/Board.h"
#include "game/GameEnd.h"
#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "pieces/Pawn.h"
#include "Side.h"

MoveHandler::MoveHandler(Board& board) : m_board{ board } {}

void MoveHandler::moveTo(const Notation& newPos, const ChessPiece& chessPiece) {
    int rank{ chessPiece.getNotation().getRank() - 1 };
    char file{ static_cast<char>(chessPiece.getNotation().getFile() - 'a') };

    ChessPiece::isOccupied[rank][file] = false;
    chessPiece.getNotation() = newPos;

    rank = chessPiece.getNotation().getRank() - 1;
    file = chessPiece.getNotation().getFile() - 'a';

    ChessPiece::isOccupied[rank][file] = true;
}

void MoveHandler::takes(const Notation& newPos, ChessPiece& chessPiece, Board& pieces) {
    for(auto& piece : pieces.getPieces(chessPiece.getColor())) {
        if(piece->getNotation() == newPos && piece->getColor() != chessPiece.getColor()) {
            Notation capturedPos{ piece->getNotation() };
            piece.reset();
            ChessPiece::isOccupied[capturedPos.getRank() - 1][capturedPos.getFile() - 'a'] = false;
        }
    }
}

bool MoveHandler::validMove(const Notation& newPos, const ChessPiece& chessPiece, const Board& pieces) {
    if(!newPos.onBoard() || !chessPiece.canReach(newPos)) { return false; }

    for(auto& piece : pieces.getPieces(chessPiece.getColor())) {
        if(piece->getNotation() == newPos && piece->getColor() == chessPiece.getColor()) {
            return false;
        }
    }

    return true;
}

bool MoveHandler::validCapture(const Notation& newPos, const ChessPiece& chessPiece, const Board& pieces) {
    for(const auto& piece : pieces.getPieces(chessPiece.getColor())) {
        if(piece->getNotation() == newPos && piece->getColor() != chessPiece.getColor()) {
            return true;
        }
    }
    return false;
}

void MoveHandler::makeMove(const Notation& fromNotation, const Notation& toNotation, Side color, int turnNum) {
    if(color == Side::White) { ++Pawn::fiftyMoveCounter; }

    for(const auto& movingPiece : m_board.getPieces(color)) {
        if(movingPiece->getNotation() == fromNotation) {
            if(!m_board.kingSafe(color)) {
                std::cout << "Move would put or leave your king in check. Please try again.\n";
                return;
            }
            if(validMove(toNotation, *movingPiece, m_board)) {
                if(auto* pawn = dynamic_cast<Pawn*>(movingPiece.get())) {
                    auto pawnPtr{ pawn->getAdjacentPawnPtr(m_board) };
                    if(pawn->canEnPassant(pawnPtr, m_board)) {
                        pawn->enPassant(pawnPtr, *this, m_board);
                    }
                    else {
                        pawn->moveTo(toNotation, *this, m_board);
                    }
                    return;
                }
                else {
                    moveTo(toNotation, *movingPiece);
                }
                if(validCapture(toNotation, *movingPiece, m_board)) {
                    takes(toNotation, *movingPiece, m_board);
                    Pawn::fiftyMoveCounter = 0;
                }
            }
            else {
                std::cout << "Invalid move for the selected piece. Please try again.\n";
                return;
            }
        }
    }
    
    if(color == Side::White) {
        ++turnNum;
    }
    
    GameEnd{ m_board }.didGameEnd(color);
}