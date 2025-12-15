#pragma once
#include <iosfwd>

class Notation {
public:
    Notation(char file, int rank);

    int getFile() const { return file; }
    int getRank() const { return rank; }

    bool operator==(const Notation& other) const;
    bool operator!=(const Notation& other) const;
    
    friend std::ostream& operator<<(std::ostream& out, const Notation& notation);

    bool onBoard() const;

private:
    char file;
    int rank;
};