#include "ChessGame.h"

#include <iostream>
#include <limits>
#include <string>
#include <string_view>

#include "game/Castling.h"
#include "notation/Notation.h"
#include "Side.h"

void ChessGame::run() {
    std::cout << "Welcome to Chess!\n\n"
                << "Enter moves in standard algebraic notation in this format:"
                << "e2 (initial position) [space] e4 (new position).\n"
                << "To castle short, type O-O, to castle long, type O-O-O.\n"
                << "Type \"resign\" to resign from the game.\n\n";
    std::string notation1, notation2;
    int turnNum{ 1 };

    while(true) {
        if(turnNum % 2 == 0) {
            std::cout << "Turn " << turnNum << "\nBlack's turn. Enter your move: ";
        } else {
            std::cout << "Turn " << turnNum << "\nWhite's turn. Enter your move: ";
        }
        
        std::cin >> notation1;

        if(notation1 == "resign") {
            std::cout << "You have resigned. Thanks for playing!\n";
            break;
        }
        else if(notation1 == "O-O" || notation1 == "O-O-O") {
            m_castling.castling(notation1, (turnNum % 2 == 0) ? Side::Black : Side::White);
            ++turnNum;
            continue;
        }

        std::cin >> notation2;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        executeMove(notation1, notation2, turnNum);
    }
}

void ChessGame::executeMove(std::string_view notation1, std::string_view notation2, int turnNum) {
    bool validNotation {
        notation1[0] >= 'a' && notation1[0] <= 'h' &&
        notation1[1] - '0' >= 1 && notation1[1] - '0' <= 8 &&
        notation2[0] >= 'a' && notation2[0] <= 'h' &&
        notation2[1] - '0' >= 1 && notation2[1] - '0' <= 8
    };

    if(validNotation && turnNum % 2 == 0) {
        Notation fromPosition{notation1[0], notation1[1]};
        Notation toPosition{notation2[0], notation2[1] - '0'};
        mv.makeMove(fromPosition, toPosition, Side::Black, turnNum);
    } else if(validNotation && turnNum % 2 == 1) {
        Notation fromPosition{notation1[0], notation1[1]};
        Notation toPosition{notation2[0], notation2[1] - '0'};
        mv.makeMove(fromPosition, toPosition, Side::White, turnNum);
    } else {
        std::cout << "Invalid move format. Please use standard algebraic notation.\n";
    }
    
    notation1 = "";
    notation2 = "";
}