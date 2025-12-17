#pragma once
#include "pieces/ChessPiece.h"
#include "game/Board.h"
#include "notation/Notation.h"

class MoveHandler {
public:
    MoveHandler(Board& board);
    MoveHandler() = default;

    void moveTo(const Notation& newPos, const ChessPiece& chessPiece);
    void takes(const Notation& newPos, ChessPiece& chessPiece, Board& pieces);
    bool validMove(const Notation& newPos, const ChessPiece& chessPiece, const Board& pieces);
    bool validCapture(const Notation& newPos, const ChessPiece& chessPiece, const Board& pieces);
    void makeMove(const Notation& fromPosition, const Notation& toPosition, Side color, int turnNum);

private:
    Board& m_board;
};