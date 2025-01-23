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

#include "../bitboard.h"
#include "../core.h"
#include "../util/multi_array.h"

namespace stoat::attacks {
    namespace internal {
        [[nodiscard]] constexpr Bitboard edges(i32 dir) {
            switch (dir) {
                case offsets::kNorth:
                    return Bitboards::kRankA;
                case offsets::kSouth:
                    return Bitboards::kRankI;
                case offsets::kWest:
                    return Bitboards::kFile9;
                case offsets::kEast:
                    return Bitboards::kFile1;
                case offsets::kNorthWest:
                    return Bitboards::kRankA | Bitboards::kFile9;
                case offsets::kNorthEast:
                    return Bitboards::kRankA | Bitboards::kFile1;
                case offsets::kSouthWest:
                    return Bitboards::kRankI | Bitboards::kFile9;
                case offsets::kSouthEast:
                    return Bitboards::kRankI | Bitboards::kFile1;
                default:
                    assert(false);
                    return Bitboards::kEmpty;
            }
        }

        [[nodiscard]] constexpr Bitboard generateSlidingAttacks(Square src, i32 dir, Bitboard occ) {
            assert(src);

            auto blockers = edges(dir);
            auto bit = Bitboard::fromSquare(src);

            if (!(blockers & bit).empty()) {
                return Bitboards::kEmpty;
            }

            blockers |= occ;

            const bool right = dir < 0;
            const auto shift = dir < 0 ? -dir : dir;

            Bitboard dst{};

            do {
                if (right) {
                    bit >>= shift;
                } else {
                    bit <<= shift;
                }

                dst |= bit;
            } while ((bit & blockers).empty());

            return dst;
        }

        template <i32... Dirs>
        constexpr Bitboard generateMultiSlidingAttacks(Square src, Bitboard occ) {
            assert(src);

            Bitboard attacks{};

            for (const auto dir : {Dirs...}) {
                attacks |= generateSlidingAttacks(src, dir, occ);
            }

            return attacks;
        }

        template <i32... Dirs>
        consteval std::array<Bitboard, Squares::kCount> generateEmptyBoardAttacks() {
            std::array<Bitboard, Squares::kCount> dst{};

            for (i32 sqIdx = 0; sqIdx < Squares::kCount; ++sqIdx) {
                const auto sq = Square::fromRaw(sqIdx);

                for (const auto dir : {Dirs...}) {
                    const auto attacks = generateSlidingAttacks(sq, dir, Bitboards::kEmpty);
                    dst[sq.idx()] |= attacks;
                }
            }

            return dst;
        }
    } // namespace internal

    constexpr util::MultiArray<Bitboard, Colors::kCount, Squares::kCount> kEmptyBoardLanceAttacks = {
        internal::generateEmptyBoardAttacks<offsets::kNorth>(), // black
        internal::generateEmptyBoardAttacks<offsets::kSouth>(), // white
    };

    constexpr auto kEmptyBoardBishopAttacks = internal::
        generateEmptyBoardAttacks<offsets::kNorthWest, offsets::kNorthEast, offsets::kSouthWest, offsets::kSouthEast>();
    constexpr auto kEmptyBoardRookAttacks =
        internal::generateEmptyBoardAttacks<offsets::kNorth, offsets::kSouth, offsets::kWest, offsets::kEast>();
} // namespace stoat::attacks
