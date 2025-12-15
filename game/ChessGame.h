#pragma once
#include <iostream>
#include <limits>
#include <string>

#include "game/Board.h"
#include "game/Castling.h"
#include "game/GameEnd.h"
#include "notation/Notation.h"

class ChessGame {
public:
    void run();

private:
    Board m_board;
    MoveHandler mv{ m_board };
    Castling m_castling{ m_board };

    void executeMove(std::string_view notation1, std::string_view notation2, int turnNum);
};