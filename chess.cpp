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
    char file; // For columns (a-h)
    int rank; // For rows (1-8)
};

enum class Side{
    White,
    Black
};

bool onBoard(const Notation& position) {
    return position.getFile() >= 'a' && position.getFile() <= 'h' && position.getRank() >= 1 && position.getRank() <= 8;
}

class ChessPiece {
public:
    ChessPiece(const Notation& startPos, Side color) : m_currentPos{startPos}, m_color{color} {}

    virtual bool isValidMove(const Notation& newPos) const {
        assert(false && "Derived class must override isValidMove()");
        return false;
    }

    bool validCapture(const Notation& newPos) const {
        for(const auto& piece : ((m_color == Side::White) ? ChessPiece::blackPieces : ChessPiece::whitePieces)) {
            if(isValidMove(newPos) && piece->getPosition() == newPos && piece->getColor() != m_color) {
                return true;
            }
        }

        return false;
    }
    
    virtual bool threatens(const Notation& newPos) const {
        assert(false && "Derived class must override threat()");
        return false;
    }

    void takes(const Notation& newPos) const {
        for(auto& piece : ((m_color == Side::White) ? ChessPiece::whitePieces : ChessPiece::blackPieces)) {
            if(piece->getPosition() == newPos && piece->getColor() != m_color) {
                Notation capturedPos{piece->getPosition()};
                piece.reset();
                isOccupied[capturedPos.getRank() - 1][capturedPos.getFile() - 'a'] = false;
            }
        }
    }

    virtual void moveTo(const Notation& newPos) {
        if(isValidMove(newPos)) {
            isOccupied[m_currentPos.getRank() - 1][m_currentPos.getFile() - 'a'] = false;
            m_currentPos = newPos;
            isOccupied[m_currentPos.getRank() - 1][m_currentPos.getFile() - 'a'] = true;
        }
    }

    Notation getPosition() const { return m_currentPos; }
    Side getColor() const { return m_color; }

    static std::vector<std::unique_ptr<ChessPiece>> whitePieces;
    static std::vector<std::unique_ptr<ChessPiece>> blackPieces;
protected:
    Notation m_currentPos;
    Side m_color;

    static std::array<std::array<bool, 8>, 8> isOccupied;
};

std::array<std::array<bool, 8>, 8> ChessPiece::isOccupied = { false };

class Pawn : public ChessPiece {
public:
    Pawn(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}

    virtual bool isValidMove(const Notation& newPos) const override {
        if(!onBoard(newPos)) { return false; }

        Notation moves{
            m_currentPos.getFile() + m_validMoves.first,
            m_currentPos.getRank() + m_validMoves.second
        };

        if(moves == newPos) {
            return true;
        }
        if(m_firstMove) {
            Notation doubleMove{
                m_currentPos.getFile() + m_validMoves.first * 2,
                m_currentPos.getRank() + m_validMoves.second * 2
            };
            if(doubleMove == newPos) {
                return true;
            }
        }
        
        return false;
    }

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position)) { return false; }

        for(const auto& [fileOffset, rankOffset] : m_captureMoves) {
            Notation threatened{ m_currentPos.getFile() + fileOffset, m_currentPos.getRank() + rankOffset };
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

        ChessPiece::moveTo(newPos);
    }
private:
    const std::array<std::pair<int, int>, 2> m_captureMoves {
        (m_color == Side::White)
            ? std::array<std::pair<int, int>, 2>{{{-1, 1}, {1, 1}}}
            : std::array<std::pair<int, int>, 2>{{{-1, -1}, {1, -1}}}
    };

    const std::pair<int, int> m_validMoves {
        (m_color == Side::White)
            ? std::pair<int, int>{0, 1}
            : std::pair<int, int>{0, -1}
    };

    bool m_firstMove { true };
};

class Rook : public ChessPiece {
public:
    Rook(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}
    
    virtual bool isValidMove(const Notation& newPos) const override {
        if(!onBoard(newPos) || m_currentPos == newPos) { return false; }

        for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
            Notation moves{m_currentPos.getFile() + fileOffsets, m_currentPos.getRank() + rankOffsets};

            while(moves != newPos && !isOccupied[moves.getRank() - 1][moves.getFile() - 'a'] && onBoard(moves)) {
                moves = {static_cast<char>(moves.getFile() + fileOffsets), moves.getRank() + rankOffsets};
            }
            if(moves == newPos) {
                return true;
            }
        }
        
        return false;
    }

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position) || this->getColor() == m_color) { return false; }
        
        for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
            Notation threatened{m_currentPos.getFile() + fileOffsets, m_currentPos.getRank() + rankOffsets};

            while(threatened != position && !isOccupied[threatened.getRank() - 1][threatened.getFile() - 'a'] && onBoard(threatened)) {
                threatened = {static_cast<char>(threatened.getFile() + fileOffsets), threatened.getRank() + rankOffsets};
            }
            if(threatened == position) {
                return true;
            }
        }
        return false;
    }
private:
    const std::array<std::pair<int, int>, 4> m_validMoves {{{1, 0}, {0, 1}, {-1, 0}, {0, -1}}};
};

class Knight : public ChessPiece {
public:
    Knight(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}

    virtual bool isValidMove(const Notation& newPos) const override {
        if(!onBoard(newPos)) { return false; }

        for(const auto& [fileOffset, rankOffset] : m_validMoves) {
            Notation moves{m_currentPos.getFile() + fileOffset, m_currentPos.getRank() + rankOffset};

            if(moves == newPos) {
                return true;
            }
        }
    }

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position)) { return false; }

        for(const auto& [fileOffset, rankOffset] : m_validMoves) {
            Notation threatened{m_currentPos.getFile() + fileOffset, m_currentPos.getRank() + rankOffset};

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
};

class Bishop : public ChessPiece {
public:
    Bishop(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}

    virtual bool isValidMove(const Notation& newPos) const override {
        if(!onBoard(newPos)) { return false; }

        for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
            Notation moves{m_currentPos.getFile() + fileOffsets, m_currentPos.getRank() + rankOffsets};
            
            while(moves != newPos && !isOccupied[moves.getRank() - 1][moves.getFile() - 'a'] && onBoard(moves)) {
                moves = {static_cast<char>(moves.getFile() + fileOffsets), moves.getRank() + rankOffsets};
            }
            if(moves == newPos) {
                return true;
            }
        }
    }

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position)) { return false; }
        
        for(const auto& [fileOffsets, rankOffsets] : m_validMoves) {
            Notation threatened{m_currentPos.getFile() + fileOffsets, m_currentPos.getRank() + rankOffsets};

            while(threatened != position && !isOccupied[threatened.getRank() - 1][threatened.getFile() - 'a'] && onBoard(threatened)) {
                threatened = {static_cast<char>(threatened.getFile() + fileOffsets), threatened.getRank() + rankOffsets};
            }
            if(threatened == position) {
                return true;
            }
        }
        return false;
    }
private:
    const std::array<std::pair<int, int>, 4> m_validMoves {{{1, 1}, {-1, 1}, {-1, -1}, {1, -1}}};
};

class Queen : public ChessPiece {
public:
    Queen(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}

    virtual bool isValidMove(const Notation& newPos) const override {
        return Rook{m_currentPos, m_color}.isValidMove(newPos) || Bishop{m_currentPos, m_color}.isValidMove(newPos);
    }

    virtual bool threatens(const Notation& newPos) const override {
        return Rook{m_currentPos, m_color}.threatens(newPos) || Bishop{m_currentPos, m_color}.threatens(newPos);
    }
};

class King : public ChessPiece {
public:
    King(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}

    virtual bool isValidMove(const Notation& newPos) const override {
        if(!onBoard(newPos)) { return false; }

        for(const auto& [fileOffset, rankOffset] : m_validMoves) {
            Notation moves{m_currentPos.getFile() + fileOffset, m_currentPos.getRank() + rankOffset};

            if(moves == newPos) {
                return true;
            }
        }
    }

    virtual bool threatens(const Notation& position) const override {
        if(!onBoard(position)) { return false; }

        for(const auto& [fileOffset, rankOffset] : m_validMoves) {
            Notation threatened{m_currentPos.getFile() + fileOffset, m_currentPos.getRank() + rankOffset};

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

bool kingSafe(Side color) {
    Notation kingPosition{'e', (color == Side::White) ? 1 : 8};

    auto kingPtr{ std::find_if(
        (color == Side::White) ? ChessPiece::whitePieces.begin() : ChessPiece::blackPieces.begin(),
        (color == Side::White) ? ChessPiece::whitePieces.end() : ChessPiece::blackPieces.end(),
        [&](const std::unique_ptr<ChessPiece>& piece) {
            return dynamic_cast<King*>(piece.get()) != nullptr;
        }
    )};

    if(kingPtr != ((color == Side::White) ? ChessPiece::whitePieces.end() : ChessPiece::blackPieces.end())) {
        kingPosition = (*kingPtr)->getPosition();
    }
    else {
        std::cerr << "Error: King not found on the board.\n";
        std::exit(EXIT_FAILURE);
    }
    
    for(const auto& piece : ((color == Side::White) ? ChessPiece::whitePieces : ChessPiece::blackPieces)) {
        if(piece->threatens(kingPosition)) {
            return false;
        }
    }

    return true;
}

void makeMove(const Notation& fromPosition, const Notation& toPosition, Side color, int turnNum) {
    for(const auto& movingPiece : ((color == Side::White) ? ChessPiece::whitePieces : ChessPiece::blackPieces)) {
        if(movingPiece->getPosition() == fromPosition) {
            if(!kingSafe(color)) {
                std::cout << "Move would put or leave your king in check. Please try again.\n";
                return;
            }
            if(movingPiece->validCapture(toPosition)) {
                movingPiece->takes(toPosition);
                movingPiece->moveTo(toPosition);
                return;
            }
            if(movingPiece->isValidMove(toPosition)) {
                movingPiece->moveTo(toPosition);
                return;
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
}

int main() {
    std::cout << "Welcome to Chess!\n\n"
              << "Enter moves in standard algebraic notation in this format:"
              << "e2 (initial position) [space] e4 (new position).\n"
              << "To indicate capturing a piece, use an 'x' between the positions, e.g., exf5.\n"
              << "To check, append + after the move (e.g. e5 e6+).\n"
              << "To castle short, type O-O, to castle long, type O-O-O.\n"
              << "Type \"resign\" to resign from the game.\n\n";
    std::string from, to;
    int turnNum{ 1 };

    while(true) {
        if(turnNum % 2 == 0) {
            std::cout << "Turn " << turnNum << "\nBlack's turn. Enter your move: ";
        } else {
            std::cout << "Turn " << turnNum << "\nWhite's turn. Enter your move: ";
        }
        
        std::cin >> from;
        if(from == "resign") {
            std::cout << "You have resigned. Thanks for playing!\n";
            break;
        }
        std::cin >> to;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        Notation fromPosition{from[0], from[1]};
        Notation toPosition{to[0], to[1]};
        
        if(std::isalpha(from[0]) && std::isdigit(from[1]) && std::isalpha(to[0]) && std::isdigit(to[1]) && turnNum % 2 == 0) {
            makeMove(fromPosition, toPosition, Side::Black, turnNum);
        } else if(std::isalpha(from[0]) && std::isdigit(from[1]) && std::isalpha(to[0]) && std::isdigit(to[1]) && turnNum % 2 == 1) {
            makeMove(fromPosition, toPosition, Side::White, turnNum);
        } else {
            std::cout << "Invalid move format. Please use standard algebraic notation.\n";
        }
        
        from = "";
        to = "";
    }

    return 0;
}