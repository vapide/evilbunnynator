#include "move.hpp"

std::string square_name(int square) {
    std::string s;
    s += FILES[square % 8];
    s += static_cast<char>('1' + square / 8);
    return s;
}

int square_index(const std::string& name) {
    if (name.size() != 2)
        return -1;

    int file = (name[0] >= 'A' && name[0] <= 'Z' ? name[0] + 32 : name[0]) - 'a';
    int rank = name[1] - '1';

    if (file < 0 || file >= 8 || rank < 0 || rank >= 8)
        return -1;

    return rank * 8 + file;
}

Move from_uci(const std::string& uci) {
    if (uci.size() != 4 && uci.size() != 5)
        return MOVE_NONE;

    int from_sq = square_index(uci.substr(0, 2));
    int to_sq = square_index(uci.substr(2, 2));
    if (from_sq < 0 || to_sq < 0)
        return MOVE_NONE;

    int promotion = PROMO_NONE;
    if (uci.size() == 5) {
        switch (uci[4] >= 'A' && uci[4] <= 'Z' ? uci[4] + 32 : uci[4]) {
            case 'q': promotion = PROMO_QUEEN;  break;
            case 'r': promotion = PROMO_ROOK;   break;
            case 'b': promotion = PROMO_BISHOP; break;
            case 'n': promotion = PROMO_KNIGHT; break;
            default:  return MOVE_NONE;
        }
    }

    return encode_move(from_sq, to_sq, 0, promotion);
}

std::string to_uci(Move m) {
    std::string s = square_name(get_from(m)) + square_name(get_to(m));
    switch (get_promotion(m)) {
        case PROMO_QUEEN:  s += 'q'; break;
        case PROMO_ROOK:   s += 'r'; break;
        case PROMO_BISHOP: s += 'b'; break;
        case PROMO_KNIGHT: s += 'n'; break;
        default: break;
    }
    return s;
}