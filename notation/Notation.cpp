#include <iostream>
#include "notation/Notation.h"

Notation::Notation(char file, int rank)
    : file{ file }, rank{ rank }
{}

bool Notation::operator==(const Notation& other) const {
    return file == other.file && rank == other.rank;
}

bool Notation::operator!=(const Notation& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& out, const Notation& notation) {
    return out << notation.getFile() << notation.getRank();
}

bool Notation::onBoard() const {
    return getFile() >= 'a' && getFile() <= 'h' &&
           getRank() >= 1 && getRank() <= 8;
}