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

#include "types.h"

#include <iostream>

#include "position.h"
#include "util/split.h"

using namespace stoat;

namespace {} // namespace

i32 main() {
    static constexpr std::string_view Sfen =
        "8l/1l+R2P3/p2pBG1pp/kps1p4/Nn1P2G2/P1P1P2PP/1PS6/1KSG3+r1/LN2+p3L w Sbgn3p 124";

    std::cout << "Parsing SFEN: " << Sfen << "\n\n";

    auto parsed = Position::fromSfen(Sfen);

    if (parsed) {
        const auto pos = parsed.take();
        std::cout << pos << "\n\nSFEN: " << pos.sfen() << std::endl;
    } else {
        std::cerr << "failed to parse SFEN: " << parsed.takeErr().message() << std::endl;
    }

    std::cout << std::endl;

    std::cout << Move::makeNormal(Squares::k7G, Squares::k7F) << std::endl;
    std::cout << Move::makePromotion(Squares::k4E, Squares::k3C) << std::endl;
    std::cout << Move::makeDrop(PieceTypes::kPawn, Squares::k3D) << std::endl;

    return 0;
}
