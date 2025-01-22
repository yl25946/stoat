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

#include "movegen.h"

#include "attacks/attacks.h"

namespace stoat::movegen {
    namespace {
        void serializeNormals(MoveList& dst, i32 offset, Bitboard attacks) {
            while (!attacks.empty()) {
                const auto to = attacks.popLsb();
                const auto from = to.offset(-offset);

                dst.push(Move::makeNormal(from, to));
            }
        }

        void serializeNormals(MoveList& dst, Square from, Bitboard attacks) {
            while (!attacks.empty()) {
                const auto to = attacks.popLsb();
                dst.push(Move::makeNormal(from, to));
            }
        }

        void serializePromotions(MoveList& dst, i32 offset, Bitboard attacks) {
            while (!attacks.empty()) {
                const auto to = attacks.popLsb();
                const auto from = to.offset(-offset);

                dst.push(Move::makePromotion(from, to));
            }
        }

        void serializePromotions(MoveList& dst, Square from, Bitboard attacks) {
            while (!attacks.empty()) {
                const auto to = attacks.popLsb();
                dst.push(Move::makePromotion(from, to));
            }
        }

        void serializeDrops(MoveList& dst, PieceType pt, Bitboard targets) {
            while (!targets.empty()) {
                const auto to = targets.popLsb();
                dst.push(Move::makeDrop(pt, to));
            }
        }

        template <bool CanPromote>
        void generatePrecalculated(
            MoveList& dst,
            const Position& pos,
            Bitboard pieces,
            auto attackGetter,
            Bitboard nonPromoMask = Bitboards::kAll
        ) {
            const auto stm = pos.stm();
            const auto mask = ~pos.colorBb(stm);

            if constexpr (CanPromote) {
                const auto promoArea = Bitboards::promoArea(stm);

                auto promotable = pieces;
                while (!promotable.empty()) {
                    const auto piece = promotable.popLsb();
                    const auto attacks = attackGetter(pos.stm(), piece) & mask & promoArea;

                    serializePromotions(dst, piece, attacks);
                }

                promotable = pieces & promoArea;
                while (!promotable.empty()) {
                    const auto piece = promotable.popLsb();
                    const auto attacks = attackGetter(pos.stm(), piece) & mask & ~promoArea;

                    serializePromotions(dst, piece, attacks);
                }
            }

            auto movable = pieces;
            while (!movable.empty()) {
                const auto piece = movable.popLsb();
                const auto attacks = attackGetter(pos.stm(), piece) & mask & nonPromoMask;

                serializeNormals(dst, piece, attacks);
            }
        }

        void generatePawns(MoveList& dst, const Position& pos) {
            const auto stm = pos.stm();
            const auto pawns = pos.pieceBb(PieceTypes::kPawn, stm);

            const auto shifted = pawns.shiftNorthRelative(stm) & ~pos.colorBb(stm);

            const auto promos = shifted & Bitboards::promoArea(stm);
            const auto nonPromos = shifted & ~Bitboards::relativeRank(stm, 8);

            const auto offset = offsets::relativeOffset(stm, offsets::kNorth);

            serializePromotions(dst, offset, promos);
            serializeNormals(dst, offset, nonPromos);
        }

        void generateLances(MoveList& dst, const Position& pos) {
            //TODO
        }

        void generateKnights(MoveList& dst, const Position& pos) {
            const auto stm = pos.stm();
            const auto knights = pos.pieceBb(PieceTypes::kKnight, stm);
            generatePrecalculated<true>(
                dst,
                pos,
                knights,
                attacks::knightAttacks,
                ~(Bitboards::relativeRank(stm, 8) | Bitboards::relativeRank(stm, 7))
            );
        }

        void generateSilvers(MoveList& dst, const Position& pos) {
            const auto silvers = pos.pieceBb(PieceTypes::kSilver, pos.stm());
            generatePrecalculated<true>(dst, pos, silvers, attacks::silverAttacks);
        }

        void generateGolds(MoveList& dst, const Position& pos) {
            const auto stm = pos.stm();
            const auto golds = pos.pieceBb(PieceTypes::kGold, stm) | pos.pieceBb(PieceTypes::kPromotedPawn, stm)
                             | pos.pieceBb(PieceTypes::kPromotedLance, stm)
                             | pos.pieceBb(PieceTypes::kPromotedKnight, stm)
                             | pos.pieceBb(PieceTypes::kPromotedSilver, stm);
            generatePrecalculated<false>(dst, pos, golds, attacks::goldAttacks);
        }

        void generateBishops(MoveList& dst, const Position& pos) {
            //TODO
        }

        void generateRooks(MoveList& dst, const Position& pos) {
            //TODO
        }

        void generatePromotedBishops(MoveList& dst, const Position& pos) {
            //TODO
        }

        void generatePromotedRooks(MoveList& dst, const Position& pos) {
            //TODO
        }

        void generateKings(MoveList& dst, const Position& pos) {
            const auto kings = pos.pieceBb(PieceTypes::kKing, pos.stm());
            generatePrecalculated<false>(dst, pos, kings, [](Color c, Square sq) { return attacks::kingAttacks(sq); });
        }

        void generateDrops(MoveList& dst, const Position& pos) {
            const auto stm = pos.stm();
            const auto& hand = pos.hand(stm);

            const auto empty = ~pos.occupancy() & Bitboards::kAll;

            const auto generate = [&](PieceType pt, Bitboard restriction = Bitboards::kAll) {
                if (hand.count(pt) > 0) {
                    const auto targets = empty & restriction;
                    serializeDrops(dst, pt, targets);
                }
            };

            generate(
                PieceTypes::kPawn,
                ~Bitboards::relativeRank(stm, 8) & ~pos.pieceBb(PieceTypes::kPawn, stm).fillFile()
            );
            generate(PieceTypes::kLance, ~Bitboards::relativeRank(stm, 8));
            generate(PieceTypes::kKnight, ~(Bitboards::relativeRank(stm, 8) | Bitboards::relativeRank(stm, 7)));
            generate(PieceTypes::kSilver);
            generate(PieceTypes::kGold);
            generate(PieceTypes::kBishop);
            generate(PieceTypes::kRook);
        }
    } // namespace

    void generateAll(MoveList& dst, const Position& pos) {
        generatePawns(dst, pos);
        generateLances(dst, pos);
        generateKnights(dst, pos);
        generateSilvers(dst, pos);
        generateGolds(dst, pos);
        generateKings(dst, pos);

        generateDrops(dst, pos);
    }
} // namespace stoat::movegen
