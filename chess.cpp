#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

class Notation {
public:
    Notation(char file, int rank) : file(file), rank(rank) {}

    int getFile() const { return file; }
    int getRank() const { return rank; }

    bool operator==(const Notation& other) const {
        return file == other.file && rank == other.rank;
    }
    bool operator!=(const Notation& other) const {
        return file != other.file && rank != other.rank;
    }
    friend std::ostream& operator<<(std::ostream& out, const Notation& notation) {
        return out << notation.getFile() << notation.getRank();
    }

private:
    char file;
    int rank;
};

enum class Side{
    White,
    Black
};

bool onBoard(const Notation& position) {
    return position.getFile() >= 'a' &&
           position.getFile() <= 'h' &&
           position.getRank() >= 1 &&
           position.getRank() <= 8;
}

class ChessPiece {
public:
    ChessPiece(const Notation& startPos, Side color)
        : m_currentPos{startPos}, m_color{color}
    {}

    bool validMove(const Notation& newPos) const {
        if(!onBoard(newPos) || !canReach(newPos)) { return false; }

        for(auto& piece : ((m_color == Side::White) ?
            ChessPiece::whitePieces : ChessPiece::blackPieces)) {
            if(piece->getPosition() == newPos && piece->getColor() == m_color) {
                return false;
            }
        }

        return true;
    }
    
    virtual bool threatens(const Notation& newPos) const {
        assert(false && "Derived class must override threat()");
        std::exit(EXIT_FAILURE);
        return false;
    }

    bool validCapture(const Notation& newPos) const {
        for(const auto& piece : ((m_color == Side::White) ?
            ChessPiece::blackPieces : ChessPiece::whitePieces)) {
            if(piece->getPosition() == newPos && piece->getColor() != m_color) {
                return true;
            }
        }
        return false;
    }
    
    void takes(const Notation& newPos) const {
        for(auto& piece : ((m_color == Side::White) ?
            ChessPiece::whitePieces : ChessPiece::blackPieces)) {
            if(piece->getPosition() == newPos && piece->getColor() != m_color) {
                Notation capturedPos{piece->getPosition()};
                piece.reset();
                isOccupied[capturedPos.getRank() - 1][capturedPos.getFile() - 'a'] = false;
            }
        }
    }

    virtual void moveTo(const Notation& newPos) {
        isOccupied[m_currentPos.getRank() - 1][m_currentPos.getFile() - 'a'] = false;
        m_currentPos = newPos;
        isOccupied[m_currentPos.getRank() - 1][m_currentPos.getFile() - 'a'] = true;
    }

    Notation getPosition() const { return m_currentPos; }
    Side getColor() const { return m_color; }

    static std::vector<std::unique_ptr<ChessPiece>> whitePieces;
    static std::vector<std::unique_ptr<ChessPiece>> blackPieces;

    static std::array<std::array<bool, 8>, 8> isOccupied;

protected:
    virtual bool canReach(const Notation& newPos) const {
        assert(false && "Derived class must override validMove()");
        std::exit(EXIT_FAILURE);
        return false;
    }

    Notation m_currentPos;
    Side m_color;
};

std::array<std::array<bool, 8>, 8> ChessPiece::isOccupied = { false };
using PieceIter = std::vector<std::unique_ptr<ChessPiece>>::iterator;

class Pawn : public ChessPiece {
public:
    Pawn(const Notation& startPos, Side color)
        : ChessPiece{startPos, color}
    {}

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position)) { return false; }

        for(const auto& [fileOffset, rankOffset] : m_captureMoves) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffset;
            Notation threatened{ static_cast<char>(fileInt),
                                 m_currentPos.getRank() + rankOffset };
            if(threatened == position) {
                return true;
            }
        }
        return false;
    }

    virtual void moveTo(const Notation& newPos) override {
        if(m_firstMove) {
            m_firstMove = false;
        }
        if(validCapture(newPos)) {
            fiftyMoveCounter = 0;
        }

        ChessPiece::moveTo(newPos);
    }

    PieceIter getAdjacentPawnPtr() const {
        return std::find_if(
            (m_color == Side::White) ?
            ChessPiece::blackPieces.begin() : ChessPiece::whitePieces.begin(),
            (m_color == Side::White) ?
            ChessPiece::blackPieces.end() : ChessPiece::whitePieces.end(),
            [this](const std::unique_ptr<ChessPiece>& piece) {
                return dynamic_cast<Pawn*>(piece.get()) != nullptr &&
                       std::abs(piece->getPosition().getFile() - this->getPosition().getFile()) == 1 &&
                       piece->getPosition().getRank() == this->getPosition().getRank();
            }
        );
    }

    bool canEnPassant(PieceIter pawnPtr) const {
        if(this->getPosition().getRank() != ((m_color == Side::White) ? 5 : 4)) {
            std::cout << "En passant not possible. Please try again.\n";
            return false;
        }
        if(pawnPtr != ((m_color == Side::White) ?
           ChessPiece::blackPieces.end() : ChessPiece::whitePieces.end())) {
            Pawn* pawn = dynamic_cast<Pawn*>(pawnPtr->get());
            if(pawn->getFirstMove()) {
                std::cout << "En passant not possible. Please try again.\n";
                return false;
            }
        }
        return true;
    }

    void enPassant(PieceIter pawnPtr) {
        if(this->getPosition().getRank() != ((m_color == Side::White) ? 5 : 4)) {
            std::cout << "En passant not possible. Please try again.\n";
            return;
        }

        if(pawnPtr != ((m_color == Side::White) ?
           ChessPiece::blackPieces.end() : ChessPiece::whitePieces.end())) {
            Pawn* pawn = dynamic_cast<Pawn*>(pawnPtr->get());
            Notation capturedPos{(*pawnPtr)->getPosition()};
            pawnPtr->reset();
            isOccupied[capturedPos.getRank() - 1][capturedPos.getFile() - 'a'] = false;
            
            this->moveTo(Notation(this->getPosition().getFile(), (m_color == Side::White) ? 6 : 3));
        }
    }

    bool getFirstMove() const { return m_firstMove; }

    static int fiftyMoveCounter;

private:
    const std::pair<int, int> m_validMoves {
        (m_color == Side::White)
            ? std::pair<int, int>{0, 1}
            : std::pair<int, int>{0, -1}
    };
    bool m_firstMove { true };

    virtual bool canReach(const Notation& newPos) const override {
        int fileInt = static_cast<int>(m_currentPos.getFile()) + m_validMoves.first;
        Notation moves{
            static_cast<char>(fileInt),
            m_currentPos.getRank() + m_validMoves.second
        };

        if(moves == newPos) {
            return true;
        }
        if(m_firstMove) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + m_validMoves.first * 2;
            Notation doubleMove{
                static_cast<char>(fileInt),
                m_currentPos.getRank() + m_validMoves.second * 2
            };
            if(doubleMove == newPos) {
                return true;
            }
        }
        return false;
    }

    const std::array<std::pair<int, int>, 2> m_captureMoves {
        (m_color == Side::White)
            ? std::array<std::pair<int, int>, 2>{{{-1, 1}, {1, 1}}}
            : std::array<std::pair<int, int>, 2>{{{-1, -1}, {1, -1}}}
    };
};

int Pawn::fiftyMoveCounter = {0};

class Rook : public ChessPiece {
public:
    Rook(const Notation& startPos, Side color)
        : ChessPiece{startPos, color}
    {}

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position)) { return false; }
        
        for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffsets;
            Notation threatened{static_cast<char>(fileInt),
                                m_currentPos.getRank() + rankOffsets};

            while(threatened != position &&
                  !isOccupied[threatened.getRank() - 1][threatened.getFile() - 'a'] &&
                  onBoard(threatened)) {
                int fileInt = static_cast<int>(threatened.getFile()) + fileOffsets;
                threatened = {static_cast<char>(fileInt), threatened.getRank() + rankOffsets};
            }
            if(threatened == position) {
                return true;
            }
        }
        return false;
    }

private:
    const std::array<std::pair<int, int>, 4> m_validMoves {
        {{1, 0}, {0, 1}, {-1, 0}, {0, -1}}
    };

    virtual bool canReach(const Notation& newPos) const override {
        for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffsets;
            Notation moves{static_cast<char>(fileInt),
                           m_currentPos.getRank() + rankOffsets};

            while(moves != newPos &&
                  !isOccupied[moves.getRank() - 1][moves.getFile() - 'a'] &&
                  onBoard(moves)) {
                int fileInt = static_cast<int>(moves.getFile()) + fileOffsets;
                moves = { static_cast<char>(fileInt), moves.getRank() + rankOffsets};
            }
            if(moves == newPos) {
                return true;
            }
        }
        return false;
    }
};

class Knight : public ChessPiece {
public:
    Knight(const Notation& startPos, Side color)
        : ChessPiece{startPos, color}
    {}

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position)) { return false; }

        for(const auto& [fileOffset, rankOffset] : m_validMoves) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffset;
            Notation threatened{static_cast<char>(fileInt),
                                m_currentPos.getRank() + rankOffset};

            if(threatened == position) {
                return true;
            }
        }
        return false;
    }

private:
    const std::array<std::pair<int, int>, 8> m_validMoves {
        {{1, 2}, {2, 1}, {1, -2}, {2, -1},
        {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}}
    };

    virtual bool canReach(const Notation& newPos) const override {
        for(const auto& [fileOffset, rankOffset] : m_validMoves) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffset;
            Notation moves{static_cast<char>(fileInt),
                           m_currentPos.getRank() + rankOffset};

            if(moves == newPos) {
                return true;
            }
        }
        return false;
    }
};

class Bishop : public ChessPiece {
public:
    Bishop(const Notation& startPos, Side color)
        : ChessPiece{startPos, color}
    {}

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position)) { return false; }
        
        for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffsets;
            Notation threatened{static_cast<char>(fileInt),
                                m_currentPos.getRank() + rankOffsets};

            while(threatened != position &&
                  !isOccupied[threatened.getRank() - 1][threatened.getFile() - 'a'] &&
                  onBoard(threatened)) {
                int fileInt = static_cast<int>(threatened.getFile()) + fileOffsets;
                threatened = {static_cast<char>(fileInt), threatened.getRank() + rankOffsets};
            }
            if(threatened == position) {
                return true;
            }
        }
        return false;
    }

private:
    const std::array<std::pair<int, int>, 4> m_validMoves {
        {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}}
    };

    virtual bool canReach(const Notation& newPos) const override {
        for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffsets;
            Notation moves{static_cast<char>(fileInt), m_currentPos.getRank() + rankOffsets};

            while(moves != newPos &&
                  !isOccupied[moves.getRank() - 1][moves.getFile() - 'a'] &&
                  onBoard(moves)) {
                    int fileInt = static_cast<int>(moves.getFile()) + fileOffsets;
                    moves = {static_cast<char>(fileInt), moves.getRank() + rankOffsets};
            }
            if(moves == newPos) {
                return true;
            }
        }
        return false;
    }
};

class Queen : public ChessPiece {
public:
    Queen(const Notation& startPos, Side color)
        : ChessPiece{startPos, color}
    {}

    virtual bool threatens(const Notation& newPos) const override {
        return Rook{m_currentPos, m_color}.threatens(newPos) ||
               Bishop{m_currentPos, m_color}.threatens(newPos);
    }

private:
    virtual bool canReach(const Notation& newPos) const override {
        return Rook{m_currentPos, m_color}.validMove(newPos) ||
               Bishop{m_currentPos, m_color}.validMove(newPos);
    }
};

class King : public ChessPiece {
public:
    King(const Notation& startPos, Side color)
        : ChessPiece{startPos, color}
    {}

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position)) { return false; }

        for(const auto& [fileOffset, rankOffset] : m_validMoves) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffset;
            Notation threatened{static_cast<char>(fileInt),
                                m_currentPos.getRank() + rankOffset};

            if(threatened == position) {
                return true;
            }
        }
        return false;
    }

private:
    const std::array<std::pair<int, int>, 8> m_validMoves {
        {{1, 0}, {0, 1}, {-1, 0}, {0, -1},
        {1, 1}, {-1, 1}, {-1, -1}, {1, -1}}
    };

    virtual bool canReach(const Notation& newPos) const override {
        for(const auto& [fileOffset, rankOffset] : m_validMoves) {
            int fileInt = static_cast<int>(m_currentPos.getFile()) + fileOffset;
            Notation moves{static_cast<char>(fileInt),
                           m_currentPos.getRank() + rankOffset};

            if(moves == newPos) {
                return true;
            }
        }
        return false;
    }
};

std::vector<std::unique_ptr<ChessPiece>> makeWhitePieces() {
    std::vector<std::unique_ptr<ChessPiece>> v;
    v.emplace_back(std::make_unique<Rook>(Notation('a', 1), Side::White));
    v.emplace_back(std::make_unique<Knight>(Notation('b', 1), Side::White));
    v.emplace_back(std::make_unique<Bishop>(Notation('c', 1), Side::White));
    v.emplace_back(std::make_unique<Queen>(Notation('d', 1), Side::White));
    v.emplace_back(std::make_unique<King>(Notation('e', 1), Side::White));
    v.emplace_back(std::make_unique<Bishop>(Notation('f', 1), Side::White));
    v.emplace_back(std::make_unique<Knight>(Notation('g', 1), Side::White));
    v.emplace_back(std::make_unique<Rook>(Notation('h', 1), Side::White));
    v.emplace_back(std::make_unique<Pawn>(Notation('a', 2), Side::White));
    v.emplace_back(std::make_unique<Pawn>(Notation('b', 2), Side::White));
    v.emplace_back(std::make_unique<Pawn>(Notation('c', 2), Side::White));
    v.emplace_back(std::make_unique<Pawn>(Notation('d', 2), Side::White));
    v.emplace_back(std::make_unique<Pawn>(Notation('e', 2), Side::White));
    v.emplace_back(std::make_unique<Pawn>(Notation('f', 2), Side::White));
    v.emplace_back(std::make_unique<Pawn>(Notation('g', 2), Side::White));
    v.emplace_back(std::make_unique<Pawn>(Notation('h', 2), Side::White));
    return v;
}

std::vector<std::unique_ptr<ChessPiece>> makeBlackPieces() {
    std::vector<std::unique_ptr<ChessPiece>> v;
    v.emplace_back(std::make_unique<Rook>(Notation('a', 8), Side::Black));
    v.emplace_back(std::make_unique<Knight>(Notation('b', 8), Side::Black));
    v.emplace_back(std::make_unique<Bishop>(Notation('c', 8), Side::Black));
    v.emplace_back(std::make_unique<Queen>(Notation('d', 8), Side::Black));
    v.emplace_back(std::make_unique<King>(Notation('e', 8), Side::Black));
    v.emplace_back(std::make_unique<Bishop>(Notation('f', 8), Side::Black));
    v.emplace_back(std::make_unique<Knight>(Notation('g', 8), Side::Black));
    v.emplace_back(std::make_unique<Pawn>(Notation('a', 7), Side::Black));
    v.emplace_back(std::make_unique<Pawn>(Notation('b', 7), Side::Black));
    v.emplace_back(std::make_unique<Pawn>(Notation('c', 7), Side::Black));
    v.emplace_back(std::make_unique<Pawn>(Notation('d', 7), Side::Black));
    v.emplace_back(std::make_unique<Pawn>(Notation('e', 7), Side::Black));
    v.emplace_back(std::make_unique<Pawn>(Notation('f', 7), Side::Black));
    v.emplace_back(std::make_unique<Pawn>(Notation('g', 7), Side::Black));
    v.emplace_back(std::make_unique<Pawn>(Notation('h', 7), Side::Black));
    return v;
}

std::vector<std::unique_ptr<ChessPiece>> ChessPiece::whitePieces{ makeWhitePieces() };
std::vector<std::unique_ptr<ChessPiece>> ChessPiece::blackPieces{ makeBlackPieces() }; 

PieceIter getKingPtr(Side color) {
    return std::find_if(
        (color == Side::White) ?
        ChessPiece::whitePieces.begin() : ChessPiece::blackPieces.begin(),
        (color == Side::White) ?
        ChessPiece::whitePieces.end() : ChessPiece::blackPieces.end(),
        [color](const std::unique_ptr<ChessPiece>& piece) {
            return dynamic_cast<King*>(piece.get()) != nullptr;
        }
    );
}

bool kingSafe(Side color) {
    Notation kingPosition{'e', (color == Side::White) ? 1 : 8};

    auto kingPtr{ getKingPtr(color) };

    if(kingPtr != ((color == Side::White) ?
       ChessPiece::whitePieces.end() : ChessPiece::blackPieces.end())) {
        kingPosition = (*kingPtr)->getPosition();
    }
    else {
        std::cerr << "Error: King not found on the board.\n";
        std::exit(EXIT_FAILURE);
    }
    
    for(const auto& piece : ((color == Side::White) ?
        ChessPiece::whitePieces : ChessPiece::blackPieces)) {
        if(piece->threatens(kingPosition)) {
            return false;
        }
    }

    return true;
}

bool canEscapeCheckmate(const std::unique_ptr<ChessPiece>& piece, Side color) {
    for(char file = 'a'; file <= 'h'; ++file) {
        for(int rank = 1; rank <= 8; ++rank) {
            Notation newPos{file, rank};
            if(piece->validMove(newPos)) {
                Notation originalPos{piece->getPosition()};
                piece->moveTo(newPos);
                bool safe = kingSafe(color);
                piece->moveTo(originalPos);
                if(safe) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool checkmate(Side color) {
     if(!std::any_of(
         (color == Side::White) ?
         ChessPiece::whitePieces.begin() : ChessPiece::blackPieces.begin(),
         (color == Side::White) ?
         ChessPiece::whitePieces.end() : ChessPiece::blackPieces.end(),
         [color](const std::unique_ptr<ChessPiece>& piece){ return canEscapeCheckmate(piece, color); })) {
        return true;
    }

    return false;
}

bool moveExists(const std::unique_ptr<ChessPiece>& piece, Side color) {
    for(char file = 'a'; file <= 'h'; ++file) {
        for(int rank = 1; rank <= 8; ++rank) {
            Notation newPos{file, rank};
            if(piece->validMove(newPos)) {
                return true;
            }
        }
    }
    return false;
}

bool stalemate(Side color) {
     if(!std::any_of(
         (color == Side::White) ?
         ChessPiece::whitePieces.begin() : ChessPiece::blackPieces.begin(),
         (color == Side::White) ?
         ChessPiece::whitePieces.end() : ChessPiece::blackPieces.end(),
         [color](const std::unique_ptr<ChessPiece>& piece){ return moveExists(piece, color); })) {
        return true;
    }

    return false;
}

PieceIter getRookPtr(Side color, const Notation& rookPosition) {
    return std::find_if(
        (color == Side::White) ?
        ChessPiece::whitePieces.begin() : ChessPiece::blackPieces.begin(),
        (color == Side::White) ?
        ChessPiece::whitePieces.end() : ChessPiece::blackPieces.end(),
        [rookPosition](const std::unique_ptr<ChessPiece>& piece) {
            return dynamic_cast<Rook*>(piece.get()) != nullptr &&
                   piece->getPosition() == rookPosition;
        }
    );
}

void shortCastle(PieceIter kingPtr, PieceIter rookPtr, Side color) {
    Notation originalPos{(*kingPtr)->getPosition()};

    for(int i = 0; i <= 1; ++i) {
        (*kingPtr)->moveTo(
            (color == Side::White) ?
            Notation(static_cast<char>((*kingPtr)->getPosition().getFile() + 1), 1) :
            Notation(static_cast<char>((*kingPtr)->getPosition().getFile() + 1), 8)
        );
        if(!kingSafe(color) ||
        ChessPiece::isOccupied[(*kingPtr)->getPosition().getRank() - 1]
                              [(*kingPtr)->getPosition().getFile() - 'a']) {
            std::cout << "Cannot castle through or into check. Please try again.\n";
            (*kingPtr)->moveTo(originalPos);
            return;
        }
    }

    (*rookPtr)->moveTo((color == Side::White) ? Notation('f', 1) : Notation('f', 8));
}

void longCastle(PieceIter kingPtr, PieceIter rookPtr, Side color) {
    Notation originalPos{(*kingPtr)->getPosition()};

    for(int i = 0; i <= 2; ++i) {
        (*kingPtr)->moveTo(
            (color == Side::White) ?
            Notation(static_cast<char>((*kingPtr)->getPosition().getFile() - 1), 1) :
            Notation(static_cast<char>((*kingPtr)->getPosition().getFile() - 1), 8)
        );
        if(!kingSafe(color) ||
            ChessPiece::isOccupied[(*kingPtr)->getPosition().getRank() - 1]
                                  [(*kingPtr)->getPosition().getFile() - 'a']) {
            std::cout << "Cannot castle through or into check. Please try again.\n";
            (*kingPtr)->moveTo(originalPos);
            return;
        }
    }

    (*rookPtr)->moveTo((color == Side::White) ? Notation('d', 1) : Notation('d', 8));
}

void castling(std::string_view castleType, Side color) {
    if(!kingSafe(color)) {
        std::cout << "Cannot castle while in check. Please try again.\n";
        return;
    }

    auto rookPtr {
        getRookPtr(color, (castleType == "O-O") ?
        Notation('h', (color == Side::White) ? 1 : 8) :
        Notation('a', (color == Side::White) ? 1 : 8))
    };
    auto kingPtr{ getKingPtr(color) };

    if(rookPtr != ((color == Side::White) ?
       ChessPiece::whitePieces.end() : ChessPiece::blackPieces.end())) {
        std::cout << "Rook has been moved or was taken. Cannot castle.\n";
        return;
    }

    if(kingPtr != ((color == Side::White) ?
       ChessPiece::whitePieces.end() : ChessPiece::blackPieces.end()) &&
       (*kingPtr)->getPosition() != Notation('e', (color == Side::White) ? 1 : 8)) {
        std::cout << "King has been moved or was taken. Cannot castle.\n";
        return;
    }

    if(castleType == "O-O" && ((color == Side::White) ?
       !ChessPiece::isOccupied[0][5 - 'a'] && !ChessPiece::isOccupied[0][6 - 'a'] :
       !ChessPiece::isOccupied[7][5 - 'a'] && !ChessPiece::isOccupied[7][6 - 'a'])) {
        shortCastle(kingPtr, rookPtr, color);
    }
    else if(castleType == "O-O-O" && ((color == Side::White) ?
            !ChessPiece::isOccupied[0][1 - 'a'] &&
            !ChessPiece::isOccupied[0][2 - 'a'] &&
            !ChessPiece::isOccupied[0][3 - 'a'] :
            !ChessPiece::isOccupied[7][1 - 'a'] &&
            !ChessPiece::isOccupied[7][2 - 'a'] &&
            !ChessPiece::isOccupied[7][3 - 'a'])) {
        longCastle(kingPtr, rookPtr, color);
    }
}

void didGameEnd(Side color) {
    if(!kingSafe(color) && checkmate(color)) {
        std::cout << "Checkmate! "
                  << ((color == Side::White) ? "White" : "Black")
                  << " wins the game!\n";
        std::exit(EXIT_SUCCESS);
    }
    else if(!kingSafe(color)) {
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

void makeMove(const Notation& fromPosition, const Notation& toPosition, Side color, int turnNum) {
    if(color == Side::White) {
        ++Pawn::fiftyMoveCounter;
    }

    for(const auto& movingPiece : ((color == Side::White) ?
        ChessPiece::whitePieces : ChessPiece::blackPieces)) {
        if(movingPiece->getPosition() == fromPosition) {
            if(!kingSafe(color)) {
                std::cout << "Move would put or leave your king in check. Please try again.\n";
                return;
            }
            if(movingPiece->validMove(toPosition)) {
                if(dynamic_cast<Pawn*>(movingPiece.get()) != nullptr) {
                    Pawn* pawn{ dynamic_cast<Pawn*>(movingPiece.get()) };
                    auto pawnPtr{ pawn->getAdjacentPawnPtr() };
                    if(pawn->canEnPassant(pawnPtr)) {
                        pawn->enPassant(pawnPtr);
                    }
                    return;
                }
                movingPiece->moveTo(toPosition);
                if(movingPiece->validCapture(toPosition)) {
                    movingPiece->takes(toPosition);
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
    
    didGameEnd(color);
}

int main() {
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
            castling(notation1, (turnNum % 2 == 0) ? Side::Black : Side::White);
            ++turnNum;
            continue;
        }

        std::cin >> notation2;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        bool validNotation { notation1[0] >= 'a' && notation1[0] <= 'h' &&
                             notation1[1] - '0' >= 1 && notation1[1] - '0' <= 8 &&
                             notation2[0] >= 'a' && notation2[0] <= 'h' &&
                             notation2[1] - '0' >= 1 && notation2[1] - '0' <= 8 };

        if(validNotation && turnNum % 2 == 0) {
            Notation fromPosition{notation1[0], notation1[1]};
            Notation toPosition{notation2[0], notation2[1] - '0'};
            makeMove(fromPosition, toPosition, Side::Black, turnNum);

        } else if(validNotation && turnNum % 2 == 1) {
            Notation fromPosition{notation1[0], notation1[1]};
            Notation toPosition{notation2[0], notation2[1] - '0'};
            makeMove(fromPosition, toPosition, Side::White, turnNum);

        } else {
            std::cout << "Invalid move format. Please use standard algebraic notation.\n";
        }
        
        notation1 = "";
        notation2 = "";
    }

    return 0;
}