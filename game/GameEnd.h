#pragma once
#include <functional>
#include <memory>

#include "game/Board.h"
#include "game/MoveHandler.h"
#include "pieces/ChessPiece.h"
#include "Side.h"

class GameEnd {
public:
    GameEnd(Board& board);

    bool canEscapeCheckmate(const std::unique_ptr<ChessPiece>& piece, Side color);
    bool checkmate(Side color);
    bool stalemate(Side color);
    void didGameEnd(Side color);
    bool moveExists(const std::unique_ptr<ChessPiece>& piece, Side color);

private:
    Board& m_board;
    MoveHandler mh;
};