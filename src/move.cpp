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

#include "move.h"

namespace stoat {
    std::ostream& operator<<(std::ostream& stream, const Move& move) {
        if (move.isDrop()) {
            const auto square = move.to();
            const auto piece = move.dropPiece();

            stream << piece.str()[0] << '*' << square;
            return stream;
        }

        const auto to = move.to();
        const auto from = move.from();

        stream << from << to;

        if (move.isPromo()) {
            stream << '+';
        }

        return stream;
    }
} // namespace stoat
