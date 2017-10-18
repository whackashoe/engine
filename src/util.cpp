#include "util.hpp"

float map_val(
    const float v,
    const float ax,
    const float ay,
    const float bx,
    const float by
) {
      return bx + (by - bx) * ((v - ax) / (ay - ax));
}

float dist(
    const float x1,
    const float y1,
    const float x2,
    const float y2
) {
    return std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
}

// converts [-1, 0, 1] to angle in radians
float angle_from_input(const int x, const int y)
{
    int m = 0;

    if(x) m = 4 + ((x + 1) << 1);
    if(y) m = 6 + ((y + 1) << 1);
    if(x && y) m += x == y ? -1 : 1;

    return TWO_PI / 8 * (m & 0x7);
}

std::vector<std::string> split_on(const std::string & str, const char n)
{
    std::vector<std::string> pieces;
    std::stringstream ss(str);
    while(ss.good()) {
        std::string sub;
        std::getline(ss, sub, n);
        pieces.emplace_back(sub);
    }

    return pieces;
}

std::vector<polygon> make_square_poly(
    const std::uint32_t x,
    const std::uint32_t y
) {
    return {
        polygon(x, y, x+1, y, x+1, y+1),
        polygon(x, y, x, y+1, x+1, y+1)
    };
};

