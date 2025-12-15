#pragma once
#include <memory>
#include <vector>

#include "pieces/ChessPiece.h"
#include "Side.h"

using ConstPieceIter = std::vector<std::unique_ptr<ChessPiece>>::const_iterator;
enum class BoardInit { Empty, WithPieces };

class Board {
public:
    Board();

    ConstPieceIter getKingPtr(Side color);
    bool kingSafe(Side color);

    std::vector<std::unique_ptr<ChessPiece>>& getPieces(Side side);
    const std::vector<std::unique_ptr<ChessPiece>>& getPieces(Side side) const;

private:
    std::vector<std::unique_ptr<ChessPiece>> whitePieces;
    std::vector<std::unique_ptr<ChessPiece>> blackPieces;

    enum class PieceTypes {
        Rook,
        Knight,
        Bishop,
        Queen,
        King,
        Pawn,
        max_piece_types,
    };

    std::unique_ptr<ChessPiece> createPiece(PieceTypes type, char file, int rank, Side side);
    void setupInitialPos();
};