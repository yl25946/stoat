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

#include "../../types.h"

#include <span>

#include "../../bitboard.h"
#include "../../core.h"
#include "util.h"

#if !ST_HAS_FAST_PEXT
    #include "magics.h"
#endif

namespace stoat::attacks::sliders {
    namespace internal {
        struct SquareData {
            u128 mask;
            u32 offset;
#if ST_HAS_FAST_PEXT
            i32 shift;
#endif
        };

        struct PieceData {
            std::array<SquareData, Squares::kCount> squares;
            u32 tableSize;
        };

        template <i32... kDirs>
        consteval PieceData generatePieceData(
#if !ST_HAS_FAST_PEXT
            std::span<const i32, Squares::kCount> shifts
#endif
        ) {
            PieceData dst{};

            for (i32 sqIdx = 0; sqIdx < Squares::kCount; ++sqIdx) {
                const auto sq = Square::fromRaw(sqIdx);
                auto& sqData = dst.squares[sq.idx()];

#if !ST_HAS_FAST_PEXT
                // lances
                if (shifts[sq.idx()] == 0) {
                    sqData.offset = dst.tableSize;
                    ++dst.tableSize;
                    continue;
                }
#endif

                Bitboard mask{};

                for (const auto dir : {kDirs...}) {
                    const auto attacks = internal::generateSlidingAttacks(sq, dir, Bitboards::kEmpty);

                    mask |= attacks & ~internal::edges(dir);
                }

                sqData.offset = dst.tableSize;

#if ST_HAS_FAST_PEXT
                sqData.mask = mask.raw();
                sqData.shift = std::popcount(static_cast<u64>(mask.raw()));
                dst.tableSize += 1 << mask.popcount();
#else
                sqData.mask = ~mask.raw();
                dst.tableSize += 1 << (128 - shifts[sq.idx()]);
#endif
            }

            return dst;
        }
    } // namespace internal

    constexpr std::array<internal::PieceData, Colors::kCount> kLanceData = {
        internal::generatePieceData<offsets::kNorth>(
#if !ST_HAS_FAST_PEXT
            black_magic::lanceShifts(Colors::kBlack)
#endif
        ),
        internal::generatePieceData<offsets::kSouth>(
#if !ST_HAS_FAST_PEXT
            black_magic::lanceShifts(Colors::kWhite)
#endif
        ),
    };

    static_assert(kLanceData[0].tableSize == kLanceData[1].tableSize);

    [[nodiscard]] constexpr const internal::PieceData& lanceData(Color c) {
        assert(c);
        return kLanceData[c.idx()];
    }

    constexpr usize kLanceDataTableSize = kLanceData[0].tableSize;

    constexpr auto kBishopData =
        internal::generatePieceData<offsets::kNorthWest, offsets::kNorthEast, offsets::kSouthWest, offsets::kSouthEast>(
#if !ST_HAS_FAST_PEXT
            black_magic::kBishopShifts
#endif
        );

    constexpr auto kRookData =
        internal::generatePieceData<offsets::kNorth, offsets::kSouth, offsets::kWest, offsets::kEast>(
#if !ST_HAS_FAST_PEXT
            black_magic::kRookShifts
#endif
        );
} // namespace stoat::attacks::sliders
