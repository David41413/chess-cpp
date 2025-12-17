#pragma once
#include <memory>
#include <string_view>
#include <vector>

#include "game/Board.h"
#include "game/MoveHandler.h"
#include "notation/Notation.h"
#include "Side.h"

using ConstPieceIter = std::vector<std::unique_ptr<ChessPiece>>::const_iterator;

class Castling {
public:
    Castling::Castling(Board& board) : m_board(board) {}

    ConstPieceIter getRookPtr(const Notation& rookPosition, const Board& board, Side color);
    void shortCastle(ConstPieceIter kingPtr, ConstPieceIter rookPtr, Side color);
    void longCastle(ConstPieceIter kingPtr, ConstPieceIter rookPtr, Side color);
    void castling(std::string_view castleType, Side color);

private:
    Board& m_board;
    MoveHandler mh;
};