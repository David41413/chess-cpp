#include <algorithm>
#include <array>
#include <cmath>
#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
// #include "Random.h" // For eventual AI opponent
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

class ChessPiece {
public:
    ChessPiece(const Notation& startPos, Side color) : m_currentPos{startPos}, m_color{color} {}

    virtual bool isValidMove(const Notation& newPos) const {
        assert(false && "Derived class must override isValidMove()");
        return false;
    }

    bool isValidCapture(const Notation& newPos) const {
        if(m_color == Side::White && isOccupied[newPos.getRank() - 1][newPos.getFile() - 1]) {
            auto currentPiecePtr{std::find_if(blackPieces.begin(), blackPieces.end(),
                [&](const std::unique_ptr<ChessPiece>& piece){ return piece->getPosition() == newPos; })};
            if(currentPiecePtr != blackPieces.end()) {
                Side capturedColor{(*currentPiecePtr)->getColor()};
                return capturedColor == Side::Black;
            }
        } else if(m_color == Side::Black && isOccupied[newPos.getRank() - 1][newPos.getFile() - 1]) {
            auto currentPiecePtr{std::find_if(whitePieces.begin(), whitePieces.end(),
                [&](const std::unique_ptr<ChessPiece>& piece){ return piece->getPosition() == newPos; })};
            if(currentPiecePtr != whitePieces.end()) {
                Side capturedColor{(*currentPiecePtr)->getColor()};
                return capturedColor == Side::White;
            }
        }
    }
    
    virtual bool threatens(const Notation& newPos) const {
        assert(false && "Derived class must override threat()");
        return false;
    }

    void takes(const Notation& newPos) const {
        if(m_color == Side::White) {
            auto currentPiecePtr{std::find_if(blackPieces.begin(), blackPieces.end(),
                [&](const std::unique_ptr<ChessPiece>& piece){ return piece->getPosition() == newPos; })};
            if(currentPiecePtr != blackPieces.end()) {
                Notation capturedPos{(*currentPiecePtr)->getPosition()};
                currentPiecePtr->reset();
                isOccupied[capturedPos.getRank() - 1][capturedPos.getFile() - 'a'] = false;
            }
        } else if(m_color == Side::Black) {
            auto currentPiecePtr{std::find_if(whitePieces.begin(), whitePieces.end(),
                [&](const std::unique_ptr<ChessPiece>& piece){ return piece->getPosition() == newPos; })};
            if(currentPiecePtr != whitePieces.end()) {
                Notation capturedPos{(*currentPiecePtr)->getPosition()};
                currentPiecePtr->reset();
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
        if (!onBoard(newPos)) return false;

        int fileDiff = std::abs(newPos.getFile() - m_currentPos.getFile());
        int rankDiff = std::abs(newPos.getRank() - m_currentPos.getRank());

        if(fileDiff == 0 && rankDiff == 0)
            return false;
        if(isOccupied[newPos.getRank() - 1][newPos.getFile() - 1] && (fileDiff == 0 && rankDiff == 1))
            return false;
        if(isValidCapture(newPos) && (fileDiff == 1 && rankDiff == 1)) {
            takes(newPos);
            return true;
        } if(fileDiff == 0 && rankDiff == 2)
            return firstMove;

        return fileDiff == 0 && rankDiff == 1;
    }

    virtual bool threatens(const Notation& position) const override {
        if (!onBoard(position)) return false;
        
        const std::array<std::pair<int, int>, 2> offsets {
            (m_color == Side::White)
                ? std::array<std::pair<int, int>, 2>{{{-1, 1}, {1, 1}}}
                : std::array<std::pair<int, int>, 2>{{{-1, -1}, {1, -1}}}
        };

        for(const auto& [fileOffset, rankOffset] : offsets) {
            Notation threatened{m_currentPos.getFile() + fileOffset, m_currentPos.getRank() + rankOffset};
            if(threatened == position)
                return true;
        }

        return false;
    }

    virtual void moveTo(const Notation& newPos) override {
        if(isValidMove(newPos) && firstMove)
            firstMove = false;

        ChessPiece::moveTo(newPos);
    }
private:
    bool firstMove = true;
};

class Rook : public ChessPiece {
public:
    Rook(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}
    
    virtual bool isValidMove(const Notation& newPos) const override {
        if (!onBoard(newPos)) return false;

        int fileDiff = std::abs(newPos.getFile() - m_currentPos.getFile());
        int rankDiff = std::abs(newPos.getRank() - m_currentPos.getRank());

        if(fileDiff == 0 && rankDiff == 0)
            return false;
        
        if(fileDiff && rankDiff == 0) {
            for(char i = std::min(m_currentPos.getFile(), newPos.getFile());
            i < std::max(m_currentPos.getFile(), newPos.getFile()) - 1; ++i) {
                if(isOccupied[m_currentPos.getRank() - 1][i])
                    return false;
            }
            if(isValidCapture(newPos)) {
                takes(newPos);
                return true;
            }
        } else if(rankDiff && fileDiff == 0) {
            for(int i = std::min(m_currentPos.getRank(), newPos.getRank());
                i < std::max(m_currentPos.getRank(), newPos.getRank()) - 1; ++i) {
                if(isOccupied[i - 1][m_currentPos.getFile() - 'a'])
                    return false;
            }
            if(isValidCapture(newPos)) {
                takes(newPos);
                return true;
            }
        }

        return true;
    }

    virtual bool threatens(const Notation& position) const override {
        if (!onBoard(position)) return false;
        
        const std::array<std::pair<int, int>, 4> offsets {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
        for(const auto& [fileOffsets, rankOffsets] : offsets) {
            Notation threatened{m_currentPos.getFile() + fileOffsets, m_currentPos.getRank() + rankOffsets};
            while((threatened != position) && isOccupied[threatened.getRank() - 1][threatened.getFile() - 'a'] && onBoard(threatened)) {
                    threatened = {static_cast<char>(threatened.getFile() + fileOffsets), threatened.getRank() + rankOffsets};
                }
            if(threatened == position)
                return true;
        }
        return false;
    }
};

class Knight : public ChessPiece {
public:
    Knight(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}

    virtual bool isValidMove(const Notation& newPos) const override {
        if (!onBoard(newPos)) return false;

        int fileDiff = std::abs(newPos.getFile() - m_currentPos.getFile());
        int rankDiff = std::abs(newPos.getRank() - m_currentPos.getRank());

        if((fileDiff == 2 && rankDiff == 1) || (fileDiff == 1 && rankDiff == 2)) {
            if(isValidCapture(newPos))
                takes(newPos);
            return true;
        }
        
        return false;
    }
};

class Bishop : public ChessPiece {
public:
    Bishop(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}

    virtual bool isValidMove(const Notation& newPos) const override {
        if (!onBoard(newPos)) return false;

        int fileDiff = std::abs(newPos.getFile() - m_currentPos.getFile());
        int rankDiff = std::abs(newPos.getRank() - m_currentPos.getRank());

        if((fileDiff == 0 && rankDiff == 0) && (fileDiff != rankDiff))
            return false;

        int fileStep = (m_currentPos.getFile() < newPos.getFile()) ? 1 : -1;
        int rankStep = (m_currentPos.getRank() < newPos.getRank()) ? 1 : -1;

        Notation position{static_cast<char>(m_currentPos.getFile() + fileStep), m_currentPos.getRank() + rankStep};
        while(position != newPos) {
            if(isOccupied[position.getRank() - 1][position.getFile() - 'a']) {
                return false;
            }
            position = {static_cast<char>(position.getFile() + fileStep), position.getRank() + rankStep};
        }

        if(isValidCapture(newPos))
            takes(newPos);

        return true;
    }

    virtual bool threatens(const Notation& position) const override {
        if (!onBoard(position)) return false;
        
        const std::array<std::pair<int, int>, 4> offsets {{{1, 1}, {-1, 1}, {1, -1}, {-1, -1}}};

        // Not done

        return false;
    }
};

class Queen : public ChessPiece {
public:
    Queen(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}

    virtual bool isValidMove(const Notation& newPos) const override {
        return Rook{m_currentPos, m_color}.isValidMove(newPos) || Bishop{m_currentPos, m_color}.isValidMove(newPos);
    }
};

class King : public ChessPiece {
public:
    King(const Notation& startPos, Side color) : ChessPiece{startPos, color} {}

    virtual bool isValidMove(const Notation& newPos) const override {
        if (!onBoard(newPos)) return false;

        int fileDiff = std::abs(newPos.getFile() - m_currentPos.getFile());
        int rankDiff = std::abs(newPos.getRank() - m_currentPos.getRank());

        if((fileDiff == 1 && rankDiff == 0) || (fileDiff == 0 && rankDiff == 1)) {
            if(isValidCapture(newPos))
                takes(newPos);
            return true;
        } if(fileDiff == 1 && rankDiff == 1) {
            if(isValidCapture(newPos))
                takes(newPos);
            return true;
        }

        return false;
    }
};

std::vector<std::unique_ptr<ChessPiece>> makeWhitePieces() {
    std::vector<std::unique_ptr<ChessPiece>> v;
    v.emplace_back(std::make_unique<Rook>(Notation('a', 1)), Side::White);
    v.emplace_back(std::make_unique<Knight>(Notation('b', 1)), Side::White);
    v.emplace_back(std::make_unique<Bishop>(Notation('c', 1)), Side::White);
    v.emplace_back(std::make_unique<Queen>(Notation('d', 1)), Side::White);
    v.emplace_back(std::make_unique<King>(Notation('e', 1)), Side::White);
    v.emplace_back(std::make_unique<Bishop>(Notation('f', 1)), Side::White);
    v.emplace_back(std::make_unique<Knight>(Notation('g', 1)), Side::White);
    v.emplace_back(std::make_unique<Rook>(Notation('h', 1)), Side::White);
    v.emplace_back(std::make_unique<Pawn>(Notation('a', 2)), Side::White);
    v.emplace_back(std::make_unique<Pawn>(Notation('b', 2)), Side::White);
    v.emplace_back(std::make_unique<Pawn>(Notation('c', 2)), Side::White);
    v.emplace_back(std::make_unique<Pawn>(Notation('d', 2)), Side::White);
    v.emplace_back(std::make_unique<Pawn>(Notation('e', 2)), Side::White);
    v.emplace_back(std::make_unique<Pawn>(Notation('f', 2)), Side::White);
    v.emplace_back(std::make_unique<Pawn>(Notation('g', 2)), Side::White);
    v.emplace_back(std::make_unique<Pawn>(Notation('h', 2)), Side::White);
    return v;
}

std::vector<std::unique_ptr<ChessPiece>> makeBlackPieces() {
    std::vector<std::unique_ptr<ChessPiece>> v;
    v.emplace_back(std::make_unique<Rook>(Notation('a', 8)), Side::Black);
    v.emplace_back(std::make_unique<Knight>(Notation('b', 8)), Side::Black);
    v.emplace_back(std::make_unique<Bishop>(Notation('c', 8)), Side::Black);
    v.emplace_back(std::make_unique<Queen>(Notation('d', 8)), Side::Black);
    v.emplace_back(std::make_unique<Queen>(Notation('e', 8)), Side::Black);
    v.emplace_back(std::make_unique<Bishop>(Notation('f', 8)), Side::Black);
    v.emplace_back(std::make_unique<Knight>(Notation('g', 8)), Side::Black);
    v.emplace_back(std::make_unique<Pawn>(Notation('a', 7)), Side::Black);
    v.emplace_back(std::make_unique<Pawn>(Notation('b', 7)), Side::Black);
    v.emplace_back(std::make_unique<Pawn>(Notation('c', 7)), Side::Black);
    v.emplace_back(std::make_unique<Pawn>(Notation('d', 7)), Side::Black);
    v.emplace_back(std::make_unique<Pawn>(Notation('e', 7)), Side::Black);
    v.emplace_back(std::make_unique<Pawn>(Notation('f', 7), Side::Black));
    v.emplace_back(std::make_unique<Pawn>(Notation('g', 7)), Side::Black);
    v.emplace_back(std::make_unique<Pawn>(Notation('h', 7)), Side::Black);
    return v;
}

std::vector<std::unique_ptr<ChessPiece>> ChessPiece::whitePieces{ makeWhitePieces() };
std::vector<std::unique_ptr<ChessPiece>> ChessPiece::blackPieces{ makeBlackPieces() };

bool onBoard(const Notation& position) {
    return position.getFile() >= 'a' && position.getFile() <= 'h' && position.getRank() >= 1 && position.getRank() <= 8;
}

void makeMove(const std::string& from, const std::string& to, Side color, int turnNum) {
    Notation fromPosition{from[0], from[1]};
    Notation newPosition{to[0], to[1]};

    if(color == Side::White) {
        auto currentPiecePtr{std::find_if(ChessPiece::whitePieces.begin(), ChessPiece::whitePieces.end(),
            [&](const std::unique_ptr<ChessPiece>& piece){ return piece->getPosition() == fromPosition; })};
        if(currentPiecePtr != ChessPiece::whitePieces.end()) {
            ChessPiece* movingPiece{currentPiecePtr->get()};
            movingPiece->moveTo(newPosition);
            ++turnNum;
        } else
            std::cout << "There is no piece at " << fromPosition << ". Please try again.";
            
    } else if(color == Side::Black) {
        auto currentPiecePtr{std::find_if(ChessPiece::blackPieces.begin(), ChessPiece::blackPieces.end(),
            [&](const std::unique_ptr<ChessPiece>& piece){ return piece->getPosition() == fromPosition; })};
        if(currentPiecePtr != ChessPiece::blackPieces.end()) {
            ChessPiece* movingPiece{currentPiecePtr->get()};
            movingPiece->moveTo(newPosition);
            ++turnNum;
        } else
            std::cout << "There is no piece at " << fromPosition << ". Please try again.";
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
        if(turnNum % 2 == 0)
            std::cout << "Turn " << turnNum << "\nBlack's turn. Enter your move: ";
        else
            std::cout << "Turn " << turnNum << "\nWhite's turn. Enter your move: ";
        
        std::cin >> from;
        if(from == "resign") {
            std::cout << "You have resigned. Thanks for playing!\n";
            break;
        }
        std::cin >> to;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if(std::isalpha(from[0]) && std::isdigit(from[1]) && std::isalpha(to[0]) && std::isdigit(to[1]) && turnNum % 2 == 0)
            makeMove(from, to, Side::Black, turnNum);
        else if(std::isalpha(from[0]) && std::isdigit(from[1]) && std::isalpha(to[0]) && std::isdigit(to[1]) && turnNum % 2 == 1)
            makeMove(from, to, Side::White, turnNum);
        else
            std::cout << "Invalid move format. Please use standard algebraic notation.\n";
        
        from = "";
        to = "";
    }

    return 0;
}