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

#include "../../arch.h"

#if ST_HAS_FAST_PEXT
    #include "bmi2.h"

namespace stoat::attacks::sliders::bmi2 {
    namespace {
        template <const internal::PieceData& kData, i32... kDirs>
        std::array<Bitboard, kData.tableSize> generateAttacks() {
            std::array<Bitboard, kData.tableSize> dst{};

            for (i32 sqIdx = 0; sqIdx < Squares::kCount; ++sqIdx) {
                const auto sq = Square::fromRaw(sqIdx);
                const auto& sqData = kData.squares[sq.idx()];

                const auto entries = 1 << util::popcount(sqData.mask);

                for (i32 i = 0; i < entries; ++i) {
                    const auto occ = Bitboard{util::pdep(i, sqData.mask)};

                    auto& attacks = dst[sqData.offset + i];

                    for (const auto dir : {kDirs...}) {
                        attacks |= internal::generateSlidingAttacks(sq, dir, occ);
                    }
                }
            }

            return dst;
        }
    } // namespace

    const util::MultiArray<Bitboard, Colors::kCount, kLanceDataTableSize> g_lanceAttacks = {
        generateAttacks<lanceData(Colors::kBlack), offsets::kNorth>(),
        generateAttacks<lanceData(Colors::kWhite), offsets::kSouth>(),
    };

    const std::array<Bitboard, kBishopData.tableSize> g_bishopAttacks = generateAttacks<
        kBishopData,
        offsets::kNorthWest,
        offsets::kNorthEast,
        offsets::kSouthWest,
        offsets::kSouthEast>();

    const std::array<Bitboard, kRookData.tableSize> g_rookAttacks =
        generateAttacks<kRookData, offsets::kNorth, offsets::kSouth, offsets::kWest, offsets::kEast>();
} // namespace stoat::attacks::sliders::bmi2

#endif
