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

#include "../types.h"

#include <memory>
#include <span>
#include <string_view>
#include <variant>
#include <vector>

#include "../core.h"
#include "../position.h"
#include "../pv.h"

namespace stoat::protocol {
    struct EngineState {
        Position pos{Position::startpos()};
        std::vector<u64> keyHistory{};
    };

    struct CpDisplayScore {
        Score score;
    };

    struct MateDisplayScore {
        i32 moves;
    };

    using DisplayScore = std::variant<CpDisplayScore, MateDisplayScore>;

    enum class CommandResult {
        Continue = 0,
        Quit,
        Unknown,
    };

    class IProtocolHandler {
    public:
        virtual ~IProtocolHandler() = default;

        virtual void printInitialInfo() const = 0;

        // gui -> engine
        [[nodiscard]] virtual CommandResult handleCommand(
            std::string_view command,
            std::span<std::string_view> args
        ) = 0;

        // engine -> gui
        virtual void printSearchInfo(
            i32 depth,
            i32 seldepth,
            f64 timeSec,
            usize nodes,
            DisplayScore score,
            const PvList& pv
        ) const = 0;
        virtual void printInfoString(std::string_view str) const = 0;
        virtual void printBestMove(Move move) const = 0;
    };

    constexpr std::string_view kDefaultHandler = "usi";

    [[nodiscard]] std::unique_ptr<IProtocolHandler> createHandler(std::string_view name, EngineState& state);
} // namespace stoat::protocol
