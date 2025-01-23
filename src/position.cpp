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

#include <algorithm>
#include <sstream>

#include "attacks/attacks.h"
#include "movegen.h"
#include "rays.h"
#include "util/parse.h"
#include "util/split.h"

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

    std::string Hand::sfen(bool uppercase) const {
        std::ostringstream sfen{};

        const auto print = [&](PieceType pt) {
            const auto count = this->count(pt);

            if (count == 0) {
                return;
            }

            if (count > 1) {
                sfen << count;
            }

            const auto c = pt.str()[0];
            assert(c != '+' && c != '?');

            sfen << (uppercase ? c : static_cast<char>(std::tolower(c)));
        };

        print(PieceTypes::kRook);
        print(PieceTypes::kBishop);
        print(PieceTypes::kGold);
        print(PieceTypes::kSilver);
        print(PieceTypes::kKnight);
        print(PieceTypes::kLance);
        print(PieceTypes::kPawn);

        return sfen.str();
    }

    std::ostream& operator<<(std::ostream& stream, const Hand& hand) {
        bool first = true;

        const auto print = [&](PieceType pt) {
            const auto count = hand.count(pt);

            if (count == 0) {
                return;
            }

            if (!first) {
                stream << ' ';
            } else {
                first = false;
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

    Position Position::applyMove(Move move) const {
        auto newPos = *this;

        const auto stm = this->stm();

        if (move.isDrop()) {
            const auto square = move.to();
            const auto piece = move.dropPiece();

            auto& hand = newPos.m_hands[stm.idx()];

            assert(!newPos.pieceOn(square));
            assert(hand.count(piece) > 0);

            newPos.addPiece(square, piece.withColor(stm));
            hand.decrement(piece);
        } else {
            const auto to = move.to();
            const auto from = move.from();

            const auto piece = newPos.pieceOn(from);

            if (move.isPromo()) {
                newPos.promotePiece(from, to, piece);
            } else {
                newPos.movePiece(from, to, piece);
            }
        }

        ++newPos.m_moveCount;
        newPos.m_stm = newPos.m_stm.flip();

        newPos.updateAttacks();

        return newPos;
    }

    bool Position::isLegal(Move move) const {
        assert(!move.isNull());

        const auto stm = this->stm();
        const auto nstm = this->stm().flip();

        const auto stmKing = king(stm);

        if (move.isDrop()) {
            if (isInCheck()) {
                // multiple checks can only be evaded with a king move
                if (m_checkers.multiple()) {
                    return false;
                }

                const auto checker = m_checkers.lsb();
                const auto checkRay = rayBetween(stmKing, checker);

                // a drop must block the check
                if (!checkRay.getSquare(move.to())) {
                    return false;
                }
            }

            // pawn drop mate rule (delivering mate by dropping a pawn is illegal)
            if (move.dropPiece() == PieceTypes::kPawn) {
                const auto dropBb = Bitboard::fromSquare(move.to());
                if (!(dropBb.shiftNorthRelative(stm) & pieceBb(PieceTypes::kKing, nstm)).empty()) {
                    // this pawn drop gives check - ensure it's not mate
                    // slow and cursed, but rare
                    const auto newPos = applyMove(move);

                    assert(newPos.checkers().one());
                    assert(newPos.checkers().getSquare(move.to()));

                    movegen::MoveList newMoves{};
                    movegen::generateAll(newMoves, newPos);

                    return std::ranges::any_of(newMoves, [&](Move newMove) { return newPos.isLegal(newMove); });
                }
            }

            // impossible to put yourself in check by dropping a piece, no cannons here
            return true;
        }

        if (pieceOn(move.from()).type() == PieceTypes::kKing) {
            // remove the king to account for moving away from the checker
            const auto kinglessOcc = occupancy() ^ pieceBb(PieceTypes::kKing, stm);
            return !isAttacked(move.to(), nstm, kinglessOcc);
        } else if (m_checkers.multiple()) {
            // multiple checks can only be evaded with a king move
            return false;
        }

        if (m_pinned.getSquare(move.from())) {
            const auto pinRay = rayIntersecting(move.from(), stmKing);
            if (!pinRay.getSquare(move.to())) {
                return false;
            }
        }

        if (isInCheck()) {
            const auto checker = m_checkers.lsb();
            // includes the checker
            const auto checkRay = rayBetween(stmKing, checker) | checker.bit();

            // must either block the check or capture the checker
            if (!checkRay.getSquare(move.to())) {
                return false;
            }
        }

        // :3
        return true;
    }

    bool Position::isAttacked(Square sq, Color attacker, Bitboard occ) const {
        assert(sq);
        assert(attacker);

        const auto c = attacker.flip();

        const auto horses = pieceBb(PieceTypes::kPromotedBishop, attacker);
        const auto dragons = pieceBb(PieceTypes::kPromotedRook, attacker);

        const auto rooks = dragons | pieceBb(PieceTypes::kRook, attacker);

        if (const auto pawns = pieceBb(PieceTypes::kPawn, attacker); !(pawns & attacks::pawnAttacks(sq, c)).empty()) {
            return true;
        }

        if (const auto knights = pieceBb(PieceTypes::kKnight, attacker);
            !(knights & attacks::knightAttacks(sq, c)).empty())
        {
            return true;
        }

        if (const auto silvers = pieceBb(PieceTypes::kSilver, attacker);
            !(silvers & attacks::silverAttacks(sq, c)).empty())
        {
            return true;
        }

        if (const auto golds = pieceBb(PieceTypes::kGold, attacker) | pieceBb(PieceTypes::kPromotedPawn, attacker)
                             | pieceBb(PieceTypes::kPromotedLance, attacker)
                             | pieceBb(PieceTypes::kPromotedKnight, attacker)
                             | pieceBb(PieceTypes::kPromotedSilver, attacker);
            !(golds & attacks::goldAttacks(sq, c)).empty())
        {
            return true;
        }

        if (const auto kings = horses | dragons | pieceBb(PieceTypes::kKing, attacker);
            !(kings & attacks::kingAttacks(sq)).empty())
        {
            return true;
        }

        if (const auto lances = rooks | pieceBb(PieceTypes::kLance, attacker);
            !(lances & attacks::lanceAttacks(sq, c, occ)).empty())
        {
            return true;
        }

        if (const auto bishops = horses | pieceBb(PieceTypes::kBishop, attacker);
            !(bishops & attacks::bishopAttacks(sq, occ)).empty())
        {
            return true;
        }

        if (!(rooks & attacks::rookAttacks(sq, occ)).empty()) {
            return true;
        }

        return false;
    }

    Bitboard Position::attackersTo(Square sq, Color attacker) const {
        assert(sq);
        assert(attacker);

        const auto defender = attacker.flip();
        const auto occ = occupancy();

        Bitboard attackers{};

        const auto horses = pieceBb(PieceTypes::kPromotedBishop, attacker);
        const auto dragons = pieceBb(PieceTypes::kPromotedRook, attacker);

        const auto pawns = pieceBb(PieceTypes::kPawn, attacker);
        attackers |= pawns & attacks::pawnAttacks(sq, defender);

        const auto lances = pieceBb(PieceTypes::kLance, attacker);
        attackers |= lances & attacks::lanceAttacks(sq, defender, occ);

        const auto knights = pieceBb(PieceTypes::kKnight, attacker);
        attackers |= knights & attacks::knightAttacks(sq, defender);

        const auto silvers = pieceBb(PieceTypes::kSilver, attacker);
        attackers |= silvers & attacks::silverAttacks(sq, defender);

        const auto golds = pieceBb(PieceTypes::kGold, attacker) | pieceBb(PieceTypes::kPromotedPawn, attacker)
                         | pieceBb(PieceTypes::kPromotedLance, attacker)
                         | pieceBb(PieceTypes::kPromotedKnight, attacker)
                         | pieceBb(PieceTypes::kPromotedSilver, attacker);
        attackers |= golds & attacks::goldAttacks(sq, defender);

        const auto bishops = horses | pieceBb(PieceTypes::kBishop, attacker);
        attackers |= bishops & attacks::bishopAttacks(sq, occ);

        const auto rooks = dragons | pieceBb(PieceTypes::kRook, attacker);
        attackers |= rooks & attacks::rookAttacks(sq, occ);

        const auto kings = horses | dragons | pieceBb(PieceTypes::kKing, attacker);
        attackers |= kings & attacks::kingAttacks(sq);

        return attackers;
    }

    std::string Position::sfen() const {
        std::ostringstream sfen{};

        for (i32 rank = 8; rank >= 0; --rank) {
            for (i32 file = 0; file < 9; ++file) {
                if (const auto piece = pieceOn(Square::fromFileRank(file, rank)); piece == Pieces::kNone) {
                    u32 emptySquares = 1;
                    for (; file < 8 && pieceOn(Square::fromFileRank(file + 1, rank)) == Pieces::kNone;
                         ++file, ++emptySquares)
                    {}

                    sfen << emptySquares;
                } else {
                    sfen << piece;
                }
            }

            if (rank > 0) {
                sfen << '/';
            }
        }

        sfen << (stm() == Colors::kBlack ? " b " : " w ");

        const auto& blackHand = hand(Colors::kBlack);
        const auto& whiteHand = hand(Colors::kWhite);

        if (blackHand.empty() && whiteHand.empty()) {
            sfen << '-';
        } else {
            sfen << blackHand.sfen(true);
            sfen << whiteHand.sfen(false);
        }

        sfen << ' ';

        sfen << moveCount();

        return sfen.str();
    }

    void Position::addPiece(Square square, Piece piece) {
        assert(square);
        assert(piece);

        assert(!pieceOn(square));

        m_colors[piece.color().idx()] |= square.bit();
        m_pieces[piece.type().idx()] |= square.bit();

        m_mailbox[square.idx()] = piece;
    }

    void Position::movePiece(Square from, Square to, Piece piece) {
        assert(from);
        assert(to);
        assert(from != to);

        assert(piece);

        const auto captured = pieceOn(to);
        assert(!captured || captured.color() != piece.color());
        assert(!captured || captured.type() != PieceTypes::kKing);

        if (captured) {
            m_colors[captured.color().idx()] ^= to.bit();
            m_pieces[captured.type().idx()] ^= to.bit();

            m_hands[captured.color().flip().idx()].increment(captured.type().unpromoted());
        }

        m_colors[piece.color().idx()] ^= from.bit() ^ to.bit();
        m_pieces[piece.type().idx()] ^= from.bit() ^ to.bit();

        m_mailbox[from.idx()] = Pieces::kNone;
        m_mailbox[to.idx()] = piece;
    }

    void Position::promotePiece(Square from, Square to, Piece piece) {
        assert(from);
        assert(to);
        assert(from != to);

        assert(piece);
        assert(!piece.isPromoted());

        const auto captured = pieceOn(to);
        assert(!captured || captured.color() != piece.color());
        assert(!captured || captured.type() != PieceTypes::kKing);

        if (captured) {
            m_colors[captured.color().idx()] ^= to.bit();
            m_pieces[captured.type().idx()] ^= to.bit();

            m_hands[captured.color().flip().idx()].increment(captured.type().unpromoted());
        }

        const auto promoted = piece.promoted();

        m_colors[piece.color().idx()] ^= from.bit() ^ to.bit();

        m_pieces[piece.type().idx()] ^= from.bit();
        m_pieces[promoted.type().idx()] ^= to.bit();

        m_mailbox[from.idx()] = Pieces::kNone;
        m_mailbox[to.idx()] = promoted;
    }

    void Position::updateAttacks() {
        const auto stm = this->stm();
        const auto nstm = this->stm().flip();

        m_checkers = attackersTo(king(stm), nstm);
        m_pinned = Bitboards::kEmpty;

        const auto stmKing = king(stm);

        const auto stmOcc = colorBb(stm);
        const auto nstmOcc = colorBb(nstm);

        const auto nstmLances = pieceBb(PieceTypes::kLance, nstm);
        const auto nstmBishops = pieceBb(PieceTypes::kBishop, nstm) | pieceBb(PieceTypes::kPromotedBishop, nstm);
        const auto nstmRooks = pieceBb(PieceTypes::kRook, nstm) | pieceBb(PieceTypes::kPromotedRook, nstm);

        auto potentialAttackers = (attacks::lanceAttacks(stmKing, stm, nstmOcc) & nstmLances)
                                | (attacks::bishopAttacks(stmKing, nstmOcc) & nstmBishops)
                                | (attacks::rookAttacks(stmKing, nstmOcc) & nstmRooks);
        while (!potentialAttackers.empty()) {
            const auto potentialAttacker = potentialAttackers.popLsb();
            const auto maybePinned = stmOcc & rayBetween(potentialAttacker, stmKing);

            if (maybePinned.one()) {
                m_pinned |= maybePinned;
            }
        }
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

        updateAttacks();
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

    util::Result<Position, SfenError> Position::fromSfenParts(std::span<std::string_view> sfen) {
        if (sfen.size() < 3 || sfen.size() > 4) {
            return util::err<SfenError>("wrong number of SFEN parts");
        }

        Position pos{};

        std::vector<std::string_view> ranks{};
        ranks.reserve(9);

        util::split(ranks, sfen[0], '/');

        if (ranks.size() != 9) {
            return util::err<SfenError>("wrong number of ranks in SFEN");
        }

        for (i32 rankIdx = 0; rankIdx < ranks.size(); ++rankIdx) {
            const auto rank = ranks[rankIdx];

            i32 fileIdx = 0;

            for (i32 curr = 0; curr < rank.size(); ++curr) {
                const auto c = rank[curr];
                if (const auto emptySquares = util::tryParseDigit<i32>(c)) {
                    fileIdx += *emptySquares;
                } else if (c == '+') {
                    if (curr == rank.size() - 1) {
                        return util::err<SfenError>("+ found at end of rank with no matching piece");
                    }

                    const auto pieceStr = rank.substr(curr, 2);

                    if (const auto piece = Piece::fromStr(pieceStr)) {
                        pos.addPiece(Square::fromFileRank(fileIdx, 8 - rankIdx), piece);
                        ++fileIdx;
                        ++curr;
                    } else {
                        return util::err<SfenError>("invalid promoted piece " + std::string{pieceStr});
                    }
                } else if (const auto piece = Piece::fromStr(rank.substr(curr, 1))) {
                    pos.addPiece(Square::fromFileRank(fileIdx, 8 - rankIdx), piece);
                    ++fileIdx;
                } else {
                    return util::err<SfenError>("invalid piece char " + std::string{c});
                }
            }

            if (fileIdx != 9) {
                return util::err<SfenError>("wrong number of files in rank");
            }
        }

        if (const auto blackKingCount = pos.pieceBb(Pieces::kBlackKing).popcount(); blackKingCount != 1) {
            return util::err<SfenError>("black must have exactly 1 king");
        }

        if (const auto whiteKingCount = pos.pieceBb(Pieces::kWhiteKing).popcount(); whiteKingCount != 1) {
            return util::err<SfenError>("white must have exactly 1 king");
        }

        const auto stm = sfen[1];

        if (stm.size() != 1 || (stm[0] != 'b' && stm[0] != 'w')) {
            return util::err<SfenError>("invalid side to move");
        }

        pos.m_stm = stm[0] == 'b' ? Colors::kBlack : Colors::kWhite;

        //TODO ensure opponent is not in check

        const auto hand = sfen[2];

        if (hand != "-") {
            u32 nextCount = 1;

            for (i32 curr = 0; curr < hand.size(); ++curr) {
                const auto c = hand[curr];
                if (std::isdigit(c)) {
                    if (curr == hand.size() - 1) {
                        return util::err<SfenError>("digit found at end of hand with no matching piece");
                    }

                    nextCount = *util::tryParseDigit(c);

                    if (nextCount == 0) {
                        return util::err<SfenError>("0 found in hand");
                    }
                } else if (const auto piece = Piece::unpromotedFromChar(hand[curr])) {
                    pos.m_hands[piece.color().idx()].set(piece.type(), nextCount);
                    nextCount = 1;
                } else {
                    return util::err<SfenError>("invalid piece " + std::string{c} + " found in hand");
                }
            }
        }

        if (sfen.size() == 4 && !util::tryParse(pos.m_moveCount, sfen[3])) {
            return util::err<SfenError>("invalid move count " + std::string{sfen[3]});
        }

        pos.updateAttacks();

        return util::ok(pos);
    }

    util::Result<Position, SfenError> Position::fromSfen(std::string_view sfen) {
        std::vector<std::string_view> parts{};
        parts.reserve(4);

        util::split(parts, sfen);

        return fromSfenParts(parts);
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
