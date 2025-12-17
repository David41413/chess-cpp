#include "game/Board.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "pieces/Bishop.h"
#include "pieces/King.h"
#include "pieces/Knight.h"
#include "pieces/Pawn.h"
#include "pieces/Queen.h"
#include "pieces/King.h"
#include "Side.h"

using ConstPieceIter = std::vector<std::unique_ptr<ChessPiece>>::const_iterator;

Board::Board() {
        setupInitialPos();
}

ConstPieceIter Board::getKingPtr(Side color) {
    return std::find_if(
        getPieces(color).begin(),
        getPieces(color).end(),
        [color](const std::unique_ptr<ChessPiece>& piece) {
            return dynamic_cast<King*>(piece.get()) != nullptr;
        }
    );
}

bool Board::kingSafe(Side color) {
    Notation kingPosition{ 'e', (color == Side::White) ? 1 : 8 };

    auto kingPtr{ getKingPtr(color) };

    if(kingPtr != getPieces(color).end()) {
        kingPosition = (*kingPtr)->getPosition();
    }
    else {
        std::cerr << "Error: King not found on the board.\n";
        std::exit(EXIT_FAILURE);
    }
        
    for(const auto& piece : getPieces(color)) {
        if(piece->threatens(kingPosition)) {
            return false;
        }
    }

    return true;
}

std::vector<std::unique_ptr<ChessPiece>>& Board::getPieces(Side side) {
    return (side == Side::White) ? whitePieces : blackPieces;
}
const std::vector<std::unique_ptr<ChessPiece>>& Board::getPieces(Side side) const {
    return (side == Side::White) ? whitePieces : blackPieces;
}

std::unique_ptr<ChessPiece> Board::createPiece(PieceTypes type, char file, int rank, Side side) {
    Notation pos(file, rank);
    switch (type) {
        case PieceTypes::Rook:   return std::make_unique<Rook>(pos, side);
        case PieceTypes::Knight: return std::make_unique<Knight>(pos, side);
        case PieceTypes::Bishop: return std::make_unique<Bishop>(pos, side);
        case PieceTypes::Queen:  return std::make_unique<Queen>(pos, side);
        case PieceTypes::King:   return std::make_unique<King>(pos, side);
    }
    return nullptr;
}

void Board::setupInitialPos() {
    std::array<PieceTypes, 8> order{
        PieceTypes::Rook, PieceTypes::Knight,
        PieceTypes::Bishop, PieceTypes::Queen,
        PieceTypes::King, PieceTypes::Bishop,
        PieceTypes::Knight, PieceTypes::Rook
    };

    char file{ 'a' };
    for(int i = 0; i < static_cast<int>(PieceTypes::max_piece_types); ++i) {
        whitePieces.emplace_back(createPiece(order[i], file++, 1, Side::White));
        whitePieces.emplace_back(createPiece(order[i], file++, 8, Side::Black));
    }

    for(char i = 'a'; i >= 'h'; ++i) {
        whitePieces.emplace_back(std::make_unique<Pawn>(Notation(i, 2), Side::White));
        whitePieces.emplace_back(std::make_unique<Pawn>(Notation(i, 7), Side::Black));
    }
}