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

#include <array>

#include "../bitboard.h"
#include "../core.h"
#include "../util/multi_array.h"

#if ST_HAS_FAST_PEXT
    #include "bmi2/bmi2.h"
#else
    #error non-BMI2 attack generator not yet implemented
#endif

namespace stoat::attacks {
    namespace tables {
        consteval std::array<Bitboard, Squares::kCount> generateAttacks(auto func) {
            std::array<Bitboard, Squares::kCount> attacks{};

            for (i32 idx = 0; idx < Squares::kCount; ++idx) {
                func(attacks[idx], Square::fromRaw(idx));
                attacks[idx] &= Bitboards::kAll;
            }

            return attacks;
        }

        consteval std::array<std::array<Bitboard, Squares::kCount>, 2> generateSidedAttacks(auto func) {
            util::MultiArray<Bitboard, 2, Squares::kCount> attacks{};

            attacks[Colors::kBlack.idx()] =
                generateAttacks([&func](Bitboard& attacks, Square sq) { return func(Colors::kBlack, attacks, sq); });
            attacks[Colors::kWhite.idx()] =
                generateAttacks([&func](Bitboard& attacks, Square sq) { return func(Colors::kWhite, attacks, sq); });

            return attacks;
        }

        constexpr auto kPawnAttacks = generateSidedAttacks([](Color c, Bitboard& attacks, Square sq) {
            const auto bit = Bitboard::fromSquare(sq);
            attacks |= bit.shiftNorthRelative(c);
        });

        constexpr auto kKnightAttacks = generateSidedAttacks([](Color c, Bitboard& attacks, Square sq) {
            const auto bit = Bitboard::fromSquare(sq);

            attacks |= bit.shiftNorthRelative(c).shiftNorthWestRelative(c);
            attacks |= bit.shiftNorthRelative(c).shiftNorthEastRelative(c);
        });

        constexpr auto kSilverAttacks = generateSidedAttacks([](Color c, Bitboard& attacks, Square sq) {
            const auto bit = Bitboard::fromSquare(sq);

            attacks |= bit.shiftNorthWest();
            attacks |= bit.shiftNorthEast();
            attacks |= bit.shiftSouthWest();
            attacks |= bit.shiftSouthEast();

            attacks |= bit.shiftNorthRelative(c);
        });

        constexpr auto kGoldAttacks = generateSidedAttacks([](Color c, Bitboard& attacks, Square sq) {
            const auto bit = Bitboard::fromSquare(sq);

            attacks |= bit.shiftNorth();
            attacks |= bit.shiftSouth();
            attacks |= bit.shiftWest();
            attacks |= bit.shiftEast();

            attacks |= bit.shiftNorthWestRelative(c);
            attacks |= bit.shiftNorthEastRelative(c);
        });

        constexpr auto kKingAttacks = generateAttacks([](Bitboard& attacks, Square sq) {
            const auto bit = Bitboard::fromSquare(sq);

            attacks |= bit.shiftNorth();
            attacks |= bit.shiftSouth();
            attacks |= bit.shiftWest();
            attacks |= bit.shiftEast();

            attacks |= bit.shiftNorthWest();
            attacks |= bit.shiftNorthEast();
            attacks |= bit.shiftSouthWest();
            attacks |= bit.shiftSouthEast();
        });
    } // namespace tables

    [[nodiscard]] inline Bitboard pawnAttacks(Color c, Square sq) {
        assert(c);
        assert(sq);
        return tables::kPawnAttacks[c.idx()][sq.idx()];
    }

    [[nodiscard]] inline Bitboard knightAttacks(Color c, Square sq) {
        assert(c);
        assert(sq);
        return tables::kKnightAttacks[c.idx()][sq.idx()];
    }

    [[nodiscard]] inline Bitboard silverAttacks(Color c, Square sq) {
        assert(c);
        assert(sq);
        return tables::kSilverAttacks[c.idx()][sq.idx()];
    }

    [[nodiscard]] inline Bitboard goldAttacks(Color c, Square sq) {
        assert(c);
        assert(sq);
        return tables::kGoldAttacks[c.idx()][sq.idx()];
    }

    [[nodiscard]] inline Bitboard kingAttacks(Square sq) {
        assert(sq);
        return tables::kKingAttacks[sq.idx()];
    }
} // namespace stoat::attacks
