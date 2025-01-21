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

#include <array>
#include <iostream>

#include "bitboard.h"

namespace stoat {
    class Position {
    public:
        Position();

        Position(const Position&) = default;
        Position(Position&&) = default;

        [[nodiscard]] inline Bitboard occupancy() const {
            return m_colors[0] | m_colors[1];
        }

        [[nodiscard]] inline Bitboard colorBb(Color color) const {
            assert(color);
            return m_colors[color.idx()];
        }

        [[nodiscard]] inline Bitboard pieceTypeBb(PieceType pt) const {
            assert(pt);
            return m_pieces[pt.idx()];
        }

        [[nodiscard]] inline Bitboard pieceBb(Piece piece) const {
            assert(piece);
            return m_colors[piece.color().idx()] & m_pieces[piece.type().idx()];
        }

        [[nodiscard]] inline Piece pieceOn(Square square) const {
            assert(square);
            return m_mailbox[square.idx()];
        }

        [[nodiscard]] inline Color stm() const {
            return m_stm;
        }

        [[nodiscard]] inline u32 halfmoves() const {
            return m_halfmoves;
        }

        [[nodiscard]] inline u32 fullmoves() const {
            return m_fullmoves;
        }

        [[nodiscard]] bool operator==(const Position&) const = default;

        Position& operator=(const Position&) = default;
        Position& operator=(Position&&) = default;

        [[nodiscard]] static Position startpos();

        friend std::ostream& operator<<(std::ostream& stream, const Position& pos);

    private:
        std::array<Bitboard, Colors::kCount> m_colors{};
        std::array<Bitboard, PieceTypes::kCount> m_pieces{};

        std::array<Piece, Squares::kCount> m_mailbox{};

        Color m_stm{Colors::kBlack};

        u8 m_halfmoves{};
        u16 m_fullmoves{1};

        void regen();
    };
} // namespace stoat
