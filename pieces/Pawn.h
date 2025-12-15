#pragma once
#include <memory>
#include <utility>
#include <vector>

#include "game/Board.h"
#include "game/MoveHandler.h"
#include "notation/Notation.h"
#include "pieces/ChessPiece.h"
#include "Side.h"

using PieceIter = std::vector<std::unique_ptr<ChessPiece>>::iterator;
using ConstPieceIter = std::vector<std::unique_ptr<ChessPiece>>::const_iterator;

class Pawn : public ChessPiece {
public:
    Pawn(const Notation& startPos, Side color);

    virtual bool threatens(const Notation& position) const override;
    void moveTo(const Notation& newPos, MoveHandler& mv, const Board& pieces);
    PieceIter getAdjacentPawnPtr(Board& board) const;
    bool canEnPassant(PieceIter pawnPtr, const Board& board);
    void enPassant(PieceIter pawnPtr, MoveHandler& mv, const Board& board);

    bool getFirstMove() const { return m_firstMove; }
    inline static int fiftyMoveCounter = { 0 };

private:
    const std::pair<int, int> m_validMoves {
        (m_color == Side::White)
            ? std::pair<int, int>{0, 1}
            : std::pair<int, int>{0, -1}
    };

    const std::array<std::pair<int, int>, 2> m_captureMoves {
        (m_color == Side::White)
            ? std::array<std::pair<int, int>, 2>{{{-1, 1}, {1, 1}}}
            : std::array<std::pair<int, int>, 2>{{{-1, -1}, {1, -1}}}
    };

    bool m_firstMove { true };
    bool m_enPassantChoice { true };

    virtual bool canReach(const Notation& newPos) const override;
};