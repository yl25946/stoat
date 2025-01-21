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
    namespace {
        constexpr i32 PawnHandBits = 5;
        constexpr i32 LanceHandBits = 3;
        constexpr i32 KnightHandBits = 3;
        constexpr i32 SilverHandBits = 3;
        constexpr i32 GoldHandBits = 3;
        constexpr i32 BishopHandBits = 2;
        constexpr i32 RookHandBits = 2;

        constexpr i32 PawnHandOffset = 0;
        constexpr i32 LanceHandOffset = PawnHandOffset + PawnHandBits;
        constexpr i32 KnightHandOffset = LanceHandOffset + LanceHandBits;
        constexpr i32 SilverHandOffset = KnightHandOffset + KnightHandBits;
        constexpr i32 GoldHandOffset = SilverHandOffset + SilverHandBits;
        constexpr i32 BishopHandOffset = GoldHandOffset + GoldHandBits;
        constexpr i32 RookHandOffset = BishopHandOffset + BishopHandBits;

        static_assert(RookHandOffset + RookHandBits <= 32);

        constexpr auto kHandOffsets = [] {
            std::array<i32, PieceTypes::kCount> offsets{};
            offsets.fill(-1);

            offsets[PieceTypes::kPawn.idx()] = PawnHandOffset;
            offsets[PieceTypes::kLance.idx()] = LanceHandOffset;
            offsets[PieceTypes::kKnight.idx()] = KnightHandOffset;
            offsets[PieceTypes::kSilver.idx()] = SilverHandOffset;
            offsets[PieceTypes::kGold.idx()] = GoldHandOffset;
            offsets[PieceTypes::kBishop.idx()] = BishopHandOffset;
            offsets[PieceTypes::kRook.idx()] = RookHandOffset;

            return offsets;
        }();

        constexpr auto kHandMasks = [] {
            std::array<u32, PieceTypes::kCount> masks{};

            masks[PieceTypes::kPawn.idx()] = ((1 << PawnHandBits) - 1) << PawnHandOffset;
            masks[PieceTypes::kLance.idx()] = ((1 << LanceHandBits) - 1) << LanceHandOffset;
            masks[PieceTypes::kKnight.idx()] = ((1 << KnightHandBits) - 1) << KnightHandOffset;
            masks[PieceTypes::kSilver.idx()] = ((1 << SilverHandBits) - 1) << SilverHandOffset;
            masks[PieceTypes::kGold.idx()] = ((1 << GoldHandBits) - 1) << GoldHandOffset;
            masks[PieceTypes::kBishop.idx()] = ((1 << BishopHandBits) - 1) << BishopHandOffset;
            masks[PieceTypes::kRook.idx()] = ((1 << RookHandBits) - 1) << RookHandOffset;

            return masks;
        }();
    } // namespace

    u32 Hand::count(PieceType pt) const {
        assert(pt);

        const auto offset = kHandOffsets[pt.idx()];
        const auto mask = kHandMasks[pt.idx()];

        assert(offset != -1);

        return (m_hand & mask) >> offset;
    }

    void Hand::increment(PieceType pt) {
        assert(pt);
        const auto curr = count(pt);
        assert(curr < (kHandMasks[pt.idx()] >> kHandOffsets[pt.idx()]));
        set(pt, curr + 1);
    }

    void Hand::decrement(PieceType pt) {
        assert(pt);
        const auto curr = count(pt);
        assert(curr > 0);
        set(pt, curr - 1);
    }

    void Hand::set(PieceType pt, u32 count) {
        assert(pt);

        const auto offset = kHandOffsets[pt.idx()];
        const auto mask = kHandMasks[pt.idx()];

        assert(offset != -1);
        assert(count <= (mask >> offset));

        m_hand = (m_hand & ~mask) | (count << offset);
    }

    std::ostream& operator<<(std::ostream& stream, const Hand& hand) {
        const auto print = [&stream, &hand](PieceType pt) {
            const auto count = hand.count(pt);

            if (count == 0) {
                return;
            }

            if (count > 1) {
                stream << count;
            }

            stream << pt;
        };

        print(PieceTypes::kRook);
        print(PieceTypes::kBishop);
        print(PieceTypes::kGold);
        print(PieceTypes::kSilver);
        print(PieceTypes::kKnight);
        print(PieceTypes::kLance);
        print(PieceTypes::kPawn);

        return stream;
    }

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
                    stream << " |" << (!piece.type().isPromoted() ? " " : "") << piece;
                } else {
                    stream << " |  ";
                }
            }

            stream << " | " << static_cast<char>('a' + 8 - rank);
            stream << "\n +---+---+---+---+---+---+---+---+---+\n";
        }

        stream << "\nBlack pieces in hand: " << pos.hand(Colors::kBlack);
        stream << "\nWhite pieces in hand: " << pos.hand(Colors::kWhite);

        stream << "\n\n" << (pos.stm() == Colors::kBlack ? "Black" : "White") << " to move";

        return stream;
    }
} // namespace stoat
