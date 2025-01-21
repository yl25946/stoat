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

#include "position.h"

namespace stoat {
    Position::Position() {
        m_mailbox.fill(Pieces::kNone);
    }

    void Position::regen() {
        m_mailbox.fill(Pieces::kNone);

        for (u8 pieceIdx = 0; pieceIdx < Pieces::kCount; ++pieceIdx) {
            const auto piece = Piece::fromRaw(pieceIdx);

            auto bb = pieceBb(piece);
            while (!bb.empty()) {
                const auto sq = bb.popLsb();
                assert(!m_mailbox[sq.idx()]);
                m_mailbox[sq.idx()] = piece;
            }
        }
    }

    Position Position::startpos() {
        Position pos{};

        pos.m_colors[Colors::kBlack.idx()] = Bitboard{u128{UINT64_C(0x7fd05ff)}};
        pos.m_colors[Colors::kWhite.idx()] =
            Bitboard{(u128{UINT64_C(0x1ff41)} << 64) | u128{UINT64_C(0x7fc0000000000000)}};

        pos.m_pieces[PieceTypes::kPawn.idx()] = Bitboard{u128{UINT64_C(0x7fc0000007fc0000)}};
        pos.m_pieces[PieceTypes::kLance.idx()] = Bitboard{(u128{UINT64_C(0x10100)} << 64) | u128{UINT64_C(0x101)}};
        pos.m_pieces[PieceTypes::kKnight.idx()] = Bitboard{(u128{UINT64_C(0x8200)} << 64) | u128{UINT64_C(0x82)}};
        pos.m_pieces[PieceTypes::kSilver.idx()] = Bitboard{(u128{UINT64_C(0x4400)} << 64) | u128{UINT64_C(0x44)}};
        pos.m_pieces[PieceTypes::kGold.idx()] = Bitboard{(u128{UINT64_C(0x2800)} << 64) | u128{UINT64_C(0x28)}};
        pos.m_pieces[PieceTypes::kKing.idx()] = Bitboard{(u128{UINT64_C(0x1000)} << 64) | u128{UINT64_C(0x10)}};
        pos.m_pieces[PieceTypes::kBishop.idx()] = Bitboard{(u128{UINT64_C(0x40)} << 64) | u128{UINT64_C(0x400)}};
        pos.m_pieces[PieceTypes::kRook.idx()] = Bitboard{(u128{UINT64_C(0x1)} << 64) | u128{UINT64_C(0x10000)}};

        pos.regen();

        return pos;
    }

    std::ostream& operator<<(std::ostream& stream, const Position& pos) {
        stream << "   9   8   7   6   5   4   3   2   1\n";
        stream << " +---+---+---+---+---+---+---+---+---+\n";

        for (i32 rank = 8; rank >= 0; --rank) {
            for (i32 file = 0; file < 9; ++file) {
                const auto piece = pos.pieceOn(Square::fromFileRank(file, rank));

                if (piece) {
                    stream << " |" << piece;
                } else {
                    stream << " |  ";
                }
            }

            stream << " | " << static_cast<char>('a' + 8 - rank);
            stream << "\n +---+---+---+---+---+---+---+---+---+\n";
        }

        stream << "\n" << (pos.stm() == Colors::kBlack ? "Black" : "White") << " to move\n";

        return stream;
    }
} // namespace stoat
