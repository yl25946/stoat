/*
 * Stoat, a USI shogi engine
 * Copyright (C) 2025 Ciekce
 *
 * Stoat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stoat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stoat. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "../types.h"

namespace stoat::util {
    [[nodiscard]] inline i32 ctz(u128 v) {
        const u64 low = v;
        const u64 high = v >> 64;

        if (low == 0) {
            return 64 + __builtin_ctzll(high);
        } else {
            return __builtin_ctzll(low);
        }
    }

    [[nodiscard]] inline i32 popcount(u128 v) {
        const u64 low = v;
        const u64 high = v >> 64;

        return __builtin_popcountll(low) + __builtin_popcountll(high);
    }
} // namespace stoat::util
