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

#include <cassert>
#include <iostream>

namespace stoat {
    class Color {
    public:
        constexpr Color() = default;

        constexpr Color(const Color&) = default;
        constexpr Color(Color&&) = default;

        [[nodiscard]] constexpr u8 raw() const {
            return m_id;
        }

        [[nodiscard]] constexpr usize idx() const {
            return static_cast<usize>(m_id);
        }

        [[nodiscard]] constexpr Color flip() const {
            assert(m_id != kNoneId);
            return Color{static_cast<u8>(m_id ^ 1)};
        }

        [[nodiscard]] static constexpr Color fromRaw(u8 id) {
            assert(id <= kNoneId);
            return Color{id};
        }

        [[nodiscard]] constexpr explicit operator bool() const {
            return m_id != kNoneId;
        }

        [[nodiscard]] constexpr bool operator==(const Color&) const = default;

        constexpr Color& operator=(const Color&) = default;
        constexpr Color& operator=(Color&&) = default;

    private:
        explicit constexpr Color(u8 id) :
                m_id{id} {}

        u8 m_id{};

        enum : u8 {
            kWhiteId = 0,
            kBlackId,
            kNoneId,
        };

        friend struct Colors;
    };

    struct Colors {
        Colors() = delete;

        static constexpr Color kWhite{Color::kWhiteId};
        static constexpr Color kBlack{Color::kBlackId};
        static constexpr Color kNone{Color::kNoneId};

        static constexpr usize kCount = kNone.idx();
    };

    class Piece;

    class PieceType {
    public:
        constexpr PieceType() = default;

        constexpr PieceType(const PieceType&) = default;
        constexpr PieceType(PieceType&&) = default;

        [[nodiscard]] constexpr u8 raw() const {
            return m_id;
        }

        [[nodiscard]] constexpr usize idx() const {
            return static_cast<usize>(m_id);
        }

        [[nodiscard]] constexpr Piece withColor(Color c) const;

        [[nodiscard]] static constexpr PieceType fromRaw(u8 id) {
            assert(id <= kNoneId);
            return PieceType{id};
        }

        [[nodiscard]] constexpr explicit operator bool() const {
            return m_id != kNoneId;
        }

        [[nodiscard]] constexpr bool operator==(const PieceType&) const = default;

        constexpr PieceType& operator=(const PieceType&) = default;
        constexpr PieceType& operator=(PieceType&&) = default;

    private:
        explicit constexpr PieceType(u8 id) :
                m_id{id} {}

        u8 m_id{};

        enum : u8 {
            kPawnId = 0,
            kPromotedPawnId,
            kLanceId,
            kKnightId,
            kPromotedLanceId,
            kPromotedKnightId,
            kSilverId,
            kPromotedSilverId,
            kGoldId,
            kBishopId,
            kRookId,
            kPromotedBishopId,
            kPromotedRookId,
            kKingId,
            kNoneId,
        };

        friend struct PieceTypes;

        friend inline std::ostream& operator<<(std::ostream& stream, PieceType pt) {
            switch (pt.raw()) {
                case kPawnId:
                    stream << " P";
                    break;
                case kPromotedPawnId:
                    stream << "+P";
                    break;
                case kLanceId:
                    stream << " L";
                    break;
                case kKnightId:
                    stream << " N";
                    break;
                case kPromotedLanceId:
                    stream << "+L";
                    break;
                case kPromotedKnightId:
                    stream << "+N";
                    break;
                case kSilverId:
                    stream << " S";
                    break;
                case kPromotedSilverId:
                    stream << "+S";
                    break;
                case kGoldId:
                    stream << " G";
                    break;
                case kBishopId:
                    stream << " B";
                    break;
                case kRookId:
                    stream << " R";
                    break;
                case kPromotedBishopId:
                    stream << "+B";
                    break;
                case kPromotedRookId:
                    stream << "+R";
                    break;
                case kKingId:
                    stream << " K";
                    break;
                default:
                    stream << " ?";
                    break;
            }

            return stream;
        }
    };

    struct PieceTypes {
        PieceTypes() = delete;

        static constexpr PieceType kPawn{PieceType::kPawnId};
        static constexpr PieceType kPromotedPawn{PieceType::kPromotedPawnId};
        static constexpr PieceType kLance{PieceType::kLanceId};
        static constexpr PieceType kKnight{PieceType::kKnightId};
        static constexpr PieceType kPromotedLance{PieceType::kPromotedLanceId};
        static constexpr PieceType kPromotedKnight{PieceType::kPromotedKnightId};
        static constexpr PieceType kSilver{PieceType::kSilverId};
        static constexpr PieceType kPromotedSilver{PieceType::kPromotedSilverId};
        static constexpr PieceType kGold{PieceType::kGoldId};
        static constexpr PieceType kBishop{PieceType::kBishopId};
        static constexpr PieceType kRook{PieceType::kRookId};
        static constexpr PieceType kPromotedBishop{PieceType::kPromotedBishopId};
        static constexpr PieceType kPromotedRook{PieceType::kPromotedRookId};
        static constexpr PieceType kKing{PieceType::kKingId};
        static constexpr PieceType kNone{PieceType::kNoneId};

        static constexpr usize kCount = kNone.idx();
    };

    class Piece {
    public:
        constexpr Piece() = default;

        constexpr Piece(const Piece&) = default;
        constexpr Piece(Piece&&) = default;

        [[nodiscard]] constexpr u8 raw() const {
            return m_id;
        }

        [[nodiscard]] constexpr usize idx() const {
            return static_cast<usize>(m_id);
        }

        [[nodiscard]] constexpr PieceType type() const {
            assert(m_id != kNoneId);
            return PieceType::fromRaw(m_id >> 1);
        }

        [[nodiscard]] constexpr Color color() const {
            assert(m_id != kNoneId);
            return Color::fromRaw(m_id & 0b1);
        }

        [[nodiscard]] static constexpr Piece fromRaw(u8 id) {
            assert(id <= kNoneId);
            return Piece{id};
        }

        [[nodiscard]] static constexpr Piece fromStr(std::string_view str) {
            if (str.length() != 2) {
                return Piece{kNoneId};
            }

            return Piece{kNoneId};
        }

        [[nodiscard]] constexpr explicit operator bool() const {
            return m_id != kNoneId;
        }

        [[nodiscard]] constexpr bool operator==(const Piece&) const = default;

        constexpr Piece& operator=(const Piece&) = default;
        constexpr Piece& operator=(Piece&&) = default;

    private:
        explicit constexpr Piece(u8 id) :
                m_id{id} {}

        u8 m_id{};

        enum : u8 {
            kBlackPawnId = 0,
            kWhitePawnId,
            kBlackPromotedPawnId,
            kWhitePromotedPawnId,
            kBlackLanceId,
            kWhiteLanceId,
            kBlackKnightId,
            kWhiteKnightId,
            kBlackPromotedLanceId,
            kWhitePromotedLanceId,
            kBlackPromotedKnightId,
            kWhitePromotedKnightId,
            kBlackSilverId,
            kWhiteSilverId,
            kBlackPromotedSilverId,
            kWhitePromotedSilverId,
            kBlackGoldId,
            kWhiteGoldId,
            kBlackBishopId,
            kWhiteBishopId,
            kBlackRookId,
            kWhiteRookId,
            kBlackPromotedBishopId,
            kWhitePromotedBishopId,
            kBlackPromotedRookId,
            kWhitePromotedRookId,
            kBlackKingId,
            kWhiteKingId,
            kNoneId,
        };

        friend struct Pieces;

        friend inline std::ostream& operator<<(std::ostream& stream, Piece piece) {
            switch (piece.raw()) {
                case kBlackPawnId:
                    stream << " P";
                    break;
                case kWhitePawnId:
                    stream << " p";
                    break;
                case kBlackPromotedPawnId:
                    stream << "+P";
                    break;
                case kWhitePromotedPawnId:
                    stream << "+p";
                    break;
                case kBlackLanceId:
                    stream << " L";
                    break;
                case kWhiteLanceId:
                    stream << " l";
                    break;
                case kBlackKnightId:
                    stream << " N";
                    break;
                case kWhiteKnightId:
                    stream << " n";
                    break;
                case kBlackPromotedLanceId:
                    stream << "+L";
                    break;
                case kWhitePromotedLanceId:
                    stream << "+l";
                    break;
                case kBlackPromotedKnightId:
                    stream << "+N";
                    break;
                case kWhitePromotedKnightId:
                    stream << "+n";
                    break;
                case kBlackSilverId:
                    stream << " S";
                    break;
                case kWhiteSilverId:
                    stream << " s";
                    break;
                case kBlackPromotedSilverId:
                    stream << "+S";
                    break;
                case kWhitePromotedSilverId:
                    stream << "+s";
                    break;
                case kBlackGoldId:
                    stream << " G";
                    break;
                case kWhiteGoldId:
                    stream << " g";
                    break;
                case kBlackBishopId:
                    stream << " B";
                    break;
                case kWhiteBishopId:
                    stream << " b";
                    break;
                case kBlackRookId:
                    stream << " R";
                    break;
                case kWhiteRookId:
                    stream << " r";
                    break;
                case kBlackPromotedBishopId:
                    stream << "+B";
                    break;
                case kWhitePromotedBishopId:
                    stream << "+b";
                    break;
                case kBlackPromotedRookId:
                    stream << "+R";
                    break;
                case kWhitePromotedRookId:
                    stream << "+r";
                    break;
                case kBlackKingId:
                    stream << " K";
                    break;
                case kWhiteKingId:
                    stream << " k";
                    break;
                default:
                    stream << " ?";
                    break;
            }

            return stream;
        }
    };

    constexpr Piece PieceType::withColor(Color c) const {
        assert(*this != PieceTypes::kNone);
        assert(c != Colors::kNone);

        return Piece::fromRaw((m_id << 1) | c.raw());
    }

    struct Pieces {
        Pieces() = delete;

        static constexpr Piece kBlackPawn{Piece::kBlackPawnId};
        static constexpr Piece kWhitePawn{Piece::kWhitePawnId};
        static constexpr Piece kBlackPromotedPawn{Piece::kBlackPromotedPawnId};
        static constexpr Piece kWhitePromotedPawn{Piece::kWhitePromotedPawnId};
        static constexpr Piece kBlackLance{Piece::kBlackLanceId};
        static constexpr Piece kWhiteLance{Piece::kWhiteLanceId};
        static constexpr Piece kBlackKnight{Piece::kBlackKnightId};
        static constexpr Piece kWhiteKnight{Piece::kWhiteKnightId};
        static constexpr Piece kBlackPromotedLance{Piece::kBlackPromotedLanceId};
        static constexpr Piece kWhitePromotedLance{Piece::kWhitePromotedLanceId};
        static constexpr Piece kBlackPromotedKnight{Piece::kBlackPromotedKnightId};
        static constexpr Piece kWhitePromotedKnight{Piece::kWhitePromotedKnightId};
        static constexpr Piece kBlackSilver{Piece::kBlackSilverId};
        static constexpr Piece kWhiteSilver{Piece::kWhiteSilverId};
        static constexpr Piece kBlackPromotedSilver{Piece::kBlackPromotedSilverId};
        static constexpr Piece kWhitePromotedSilver{Piece::kWhitePromotedSilverId};
        static constexpr Piece kBlackGold{Piece::kBlackGoldId};
        static constexpr Piece kWhiteGold{Piece::kWhiteGoldId};
        static constexpr Piece kBlackBishop{Piece::kBlackBishopId};
        static constexpr Piece kWhiteBishop{Piece::kWhiteBishopId};
        static constexpr Piece kBlackRook{Piece::kBlackRookId};
        static constexpr Piece kWhiteRook{Piece::kWhiteRookId};
        static constexpr Piece kBlackPromotedBishop{Piece::kBlackPromotedBishopId};
        static constexpr Piece kWhitePromotedBishop{Piece::kWhitePromotedBishopId};
        static constexpr Piece kBlackPromotedRook{Piece::kBlackPromotedRookId};
        static constexpr Piece kWhitePromotedRook{Piece::kWhitePromotedRookId};
        static constexpr Piece kBlackKing{Piece::kBlackKingId};
        static constexpr Piece kWhiteKing{Piece::kWhiteKingId};
        static constexpr Piece kNone{Piece::kNoneId};

        static constexpr usize kCount = kNone.idx();
    };
} // namespace stoat
