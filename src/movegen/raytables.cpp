#include "raytables.hpp"

namespace Ray {

U64 BETWEEN[64][64];
U64 LINE[64][64];
U64 FULL_LINE[64][64];

// ported from tables.py generate ray tables
void init() {
    constexpr int directions[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {-1, -1}, {1, -1}, {-1, 1},
    };

    for (int start = 0; start < 64; ++start) {
        const int sf = start % 8;
        const int sr = start / 8;
        for (const auto& dir : directions) {
            const int df = dir[0], dr = dir[1];

            int ray[8];
            int ray_len = 0;
            int f = sf + df, r = sr + dr;
            while (f >= 0 && f < 8 && r >= 0 && r < 8) {
                ray[ray_len++] = r * 8 + f;
                f += df;
                r += dr;
            }

            U64 full = 1ULL << start;
            for (int i = 0; i < ray_len; ++i)
                full |= 1ULL << ray[i];
            f = sf - df;
            r = sr - dr;
            while (f >= 0 && f < 8 && r >= 0 && r < 8) {
                full |= 1ULL << (r * 8 + f);
                f -= df;
                r -= dr;
            }

            for (int end_index = 0; end_index < ray_len; ++end_index) {
                FULL_LINE[start][ray[end_index]] = full;
                const int end = ray[end_index];

                U64 line = 1ULL << start;
                for (int i = 0; i <= end_index; ++i)
                    line |= 1ULL << ray[i];
                LINE[start][end] = line;

                U64 between = 0;
                for (int i = 0; i < end_index; ++i)
                    between |= 1ULL << ray[i];
                BETWEEN[start][end] = between;
            }
        }
    }
}

}
