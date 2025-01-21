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

#include "types.h"

#include <iostream>

#include "core.h"
#include "util/bits.h"

namespace stoat {
    namespace offsets {
        constexpr i32 kNorth = 9;
        constexpr i32 kSouth = -9;
        constexpr i32 kWest = -1;
        constexpr i32 kEast = 1;

        constexpr i32 kNorthWest = 8;
        constexpr i32 kNorthEast = 10;
        constexpr i32 kSouthWest = -10;
        constexpr i32 kSouthEast = -8;
    } // namespace offsets

    class Bitboard {
    public:
        constexpr Bitboard() = default;

        explicit constexpr Bitboard(u128 bb) :
                m_bb{bb} {}

        constexpr Bitboard(const Bitboard&) = default;
        constexpr Bitboard(Bitboard&&) = default;

        [[nodiscard]] constexpr Bitboard operator&(Bitboard rhs) const {
            return Bitboard{m_bb & rhs.raw()};
        }

        [[nodiscard]] constexpr Bitboard operator|(Bitboard rhs) const {
            return Bitboard{m_bb | rhs.raw()};
        }

        [[nodiscard]] constexpr Bitboard operator^(Bitboard rhs) const {
            return Bitboard{m_bb ^ rhs.raw()};
        }

        constexpr Bitboard& operator&=(Bitboard rhs) {
            m_bb &= rhs.raw();
            return *this;
        }

        constexpr Bitboard& operator|=(Bitboard rhs) {
            m_bb |= rhs.raw();
            return *this;
        }

        constexpr Bitboard& operator^=(Bitboard rhs) {
            m_bb ^= rhs.raw();
            return *this;
        }

        [[nodiscard]] constexpr Bitboard operator&(u128 rhs) const {
            return Bitboard{m_bb & rhs};
        }

        [[nodiscard]] constexpr Bitboard operator|(u128 rhs) const {
            return Bitboard{m_bb | rhs};
        }

        [[nodiscard]] constexpr Bitboard operator^(u128 rhs) const {
            return Bitboard{m_bb ^ rhs};
        }

        constexpr Bitboard& operator&=(u128 rhs) {
            m_bb &= rhs;
            return *this;
        }

        constexpr Bitboard& operator|=(u128 rhs) {
            m_bb |= rhs;
            return *this;
        }

        constexpr Bitboard& operator^=(u128 rhs) {
            m_bb ^= rhs;
            return *this;
        }

        [[nodiscard]] constexpr Bitboard operator&(i32 rhs) const {
            return Bitboard{m_bb & static_cast<u128>(rhs)};
        }

        [[nodiscard]] constexpr Bitboard operator|(i32 rhs) const {
            return Bitboard{m_bb | static_cast<u128>(rhs)};
        }

        [[nodiscard]] constexpr Bitboard operator^(i32 rhs) const {
            return Bitboard{m_bb ^ static_cast<u128>(rhs)};
        }

        constexpr Bitboard& operator&=(i32 rhs) {
            m_bb &= static_cast<u128>(rhs);
            return *this;
        }

        constexpr Bitboard& operator|=(i32 rhs) {
            m_bb |= static_cast<u128>(rhs);
            return *this;
        }

        constexpr Bitboard& operator^=(i32 rhs) {
            m_bb ^= static_cast<u128>(rhs);
            return *this;
        }

        [[nodiscard]] constexpr Bitboard operator~() const {
            return Bitboard{~m_bb};
        }

        [[nodiscard]] constexpr Bitboard operator<<(i32 rhs) const {
            return Bitboard{m_bb << rhs};
        }

        [[nodiscard]] constexpr Bitboard operator>>(i32 rhs) const {
            return Bitboard{m_bb >> rhs};
        }

        constexpr Bitboard& operator<<=(i32 rhs) {
            m_bb <<= rhs;
            return *this;
        }

        constexpr Bitboard& operator>>=(i32 rhs) {
            m_bb >>= rhs;
            return *this;
        }

        [[nodiscard]] constexpr bool getSquare(Square square) const {
            return (m_bb & square.bit()) != 0;
        }

        constexpr Bitboard& setSquare(Square square) {
            m_bb |= square.bit();
            return *this;
        }

        constexpr Bitboard& clearSquare(Square square) {
            m_bb &= ~square.bit();
            return *this;
        }

        constexpr Bitboard& toggleSquare(Square square) {
            m_bb ^= square.bit();
            return *this;
        }

        constexpr Bitboard& setSquare(Square sq, bool set) {
            if (set) {
                return setSquare(sq);
            } else {
                return clearSquare(sq);
            }
        }

        constexpr Bitboard& clear() {
            m_bb = 0;
            return *this;
        }

        [[nodiscard]] constexpr i32 popcount() const {
            return util::popcount(m_bb);
        }

        [[nodiscard]] constexpr bool empty() const {
            return m_bb == 0;
        }

        [[nodiscard]] constexpr Square lsb() const {
            const auto idx = util::ctz(m_bb);
            return Square::fromRaw(idx);
        }

        constexpr Square popLsb() {
            const auto square = lsb();
            m_bb &= m_bb - 1;
            return square;
        }

        [[nodiscard]] constexpr Bitboard shiftNorth() const {
            return Bitboard{(m_bb & ~kRankA) << offsets::kNorth};
        }

        [[nodiscard]] constexpr Bitboard shiftSouth() const {
            return Bitboard{m_bb >> -offsets::kSouth};
        }

        [[nodiscard]] constexpr Bitboard shiftWest() const {
            return Bitboard{(m_bb & ~kFile9) >> -offsets::kWest};
        }

        [[nodiscard]] constexpr Bitboard shiftEast() const {
            return Bitboard{(m_bb & ~kFile1) << offsets::kEast};
        }

        [[nodiscard]] constexpr Bitboard shiftNorthWest() const {
            return Bitboard{(m_bb & ~(kRankA | kFile9)) << offsets::kNorthWest};
        }

        [[nodiscard]] constexpr Bitboard shiftNorthEast() const {
            return Bitboard{(m_bb & ~(kRankA | kFile1)) << offsets::kNorthEast};
        }

        [[nodiscard]] constexpr Bitboard shiftSouthWest() const {
            return Bitboard{(m_bb & ~kFile9) >> -offsets::kSouthWest};
        }

        [[nodiscard]] constexpr Bitboard shiftSouthEast() const {
            return Bitboard{(m_bb & ~kFile1) >> -offsets::kSouthEast};
        }

        [[nodiscard]] constexpr u128 raw() const {
            return m_bb;
        }

        [[nodiscard]] constexpr bool operator==(const Bitboard&) const = default;

        constexpr Bitboard& operator=(const Bitboard&) = default;
        constexpr Bitboard& operator=(Bitboard&&) = default;

        [[nodiscard]] static constexpr Bitboard fromSquare(Square square) {
            return Bitboard{square.bit()};
        }

        [[nodiscard]] static constexpr Bitboard fromSquareOrZero(Square square) {
            if (square == Squares::kNone)
                return Bitboard{};
            return Bitboard{square.bit()};
        }

    private:
        u128 m_bb{};

        static constexpr u128 kAll = (u128{UINT64_C(0x1ffff)} << 64) | u128{UINT64_C(0xffffffffffffffff)};
        static constexpr u128 kEmpty = 0;

        static constexpr u128 kRankI = u128{UINT64_C(0x1ff)};
        static constexpr u128 kRankH = u128{UINT64_C(0x3fe00)};
        static constexpr u128 kRankG = u128{UINT64_C(0x7fc0000)};
        static constexpr u128 kRankF = u128{UINT64_C(0xff8000000)};
        static constexpr u128 kRankE = u128{UINT64_C(0x1ff000000000)};
        static constexpr u128 kRankD = u128{UINT64_C(0x3fe00000000000)};
        static constexpr u128 kRankC = u128{UINT64_C(0x7fc0000000000000)};
        static constexpr u128 kRankB = (u128{UINT64_C(0xff)} << 64) | u128{UINT64_C(0x8000000000000000)};
        static constexpr u128 kRankA = u128{UINT64_C(0x1ff00)} << 64;

        static constexpr u128 kFile9 = (u128{UINT64_C(0x100)} << 64) | u128{UINT64_C(0x8040201008040201)};
        static constexpr u128 kFile8 = (u128{UINT64_C(0x201)} << 64) | u128{UINT64_C(0x80402010080402)};
        static constexpr u128 kFile7 = (u128{UINT64_C(0x402)} << 64) | u128{UINT64_C(0x100804020100804)};
        static constexpr u128 kFile6 = (u128{UINT64_C(0x804)} << 64) | u128{UINT64_C(0x201008040201008)};
        static constexpr u128 kFile5 = (u128{UINT64_C(0x1008)} << 64) | u128{UINT64_C(0x402010080402010)};
        static constexpr u128 kFile4 = (u128{UINT64_C(0x2010)} << 64) | u128{UINT64_C(0x804020100804020)};
        static constexpr u128 kFile3 = (u128{UINT64_C(0x4020)} << 64) | u128{UINT64_C(0x1008040201008040)};
        static constexpr u128 kFile2 = (u128{UINT64_C(0x8040)} << 64) | u128{UINT64_C(0x2010080402010080)};
        static constexpr u128 kFile1 = (u128{UINT64_C(0x10080)} << 64) | u128{UINT64_C(0x4020100804020100)};

        friend struct Bitboards;

        friend inline std::ostream& operator<<(std::ostream& stream, Bitboard bb) {
            for (i32 rank = 8; rank >= 0; --rank) {
                for (i32 file = 0; file < 9; ++file) {
                    if (file > 0) {
                        stream << ' ';
                    }

                    stream << (bb.getSquare(Square::fromFileRank(file, rank)) ? '1' : '.');
                }

                stream << '\n';
            }

            return stream;
        }
    };

    struct Bitboards {
        Bitboards() = delete;

        static constexpr Bitboard kAll = Bitboard{Bitboard::kAll};
        static constexpr Bitboard kEmpty = Bitboard{Bitboard::kEmpty};

        static constexpr Bitboard kRankI = Bitboard{Bitboard::kRankI};
        static constexpr Bitboard kRankH = Bitboard{Bitboard::kRankH};
        static constexpr Bitboard kRankG = Bitboard{Bitboard::kRankG};
        static constexpr Bitboard kRankF = Bitboard{Bitboard::kRankF};
        static constexpr Bitboard kRankE = Bitboard{Bitboard::kRankE};
        static constexpr Bitboard kRankD = Bitboard{Bitboard::kRankD};
        static constexpr Bitboard kRankC = Bitboard{Bitboard::kRankC};
        static constexpr Bitboard kRankB = Bitboard{Bitboard::kRankB};
        static constexpr Bitboard kRankA = Bitboard{Bitboard::kRankA};

        static constexpr Bitboard kFile9 = Bitboard{Bitboard::kFile9};
        static constexpr Bitboard kFile8 = Bitboard{Bitboard::kFile8};
        static constexpr Bitboard kFile7 = Bitboard{Bitboard::kFile7};
        static constexpr Bitboard kFile6 = Bitboard{Bitboard::kFile6};
        static constexpr Bitboard kFile5 = Bitboard{Bitboard::kFile5};
        static constexpr Bitboard kFile4 = Bitboard{Bitboard::kFile4};
        static constexpr Bitboard kFile3 = Bitboard{Bitboard::kFile3};
        static constexpr Bitboard kFile2 = Bitboard{Bitboard::kFile2};
        static constexpr Bitboard kFile1 = Bitboard{Bitboard::kFile1};
    };
} // namespace stoat
