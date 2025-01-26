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

#include "uci_like.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "../perft.h"
#include "../util/parse.h"
#include "common.h"

#include "../movegen.h"

namespace stoat::protocol {
    UciLikeHandler::UciLikeHandler(EngineState& state) :
            m_state{state} {
#define REGISTER_HANDLER(Command) registerCommandHandler(#Command, [this](auto tokens) { handle_##Command(tokens); })

        REGISTER_HANDLER(isready);
        REGISTER_HANDLER(position);
        REGISTER_HANDLER(go);

        REGISTER_HANDLER(d);
        REGISTER_HANDLER(splitperft);

#undef REGISTER_HANDLER
    }

    void UciLikeHandler::printInitialInfo() const {
        std::cout << "id name " << kName << ' ' << kVersion << '\n';
        std::cout << "id author " << kAuthor << '\n';

        //TODO options

        finishInitialInfo();
    }

    CommandResult UciLikeHandler::handleCommand(std::string_view command, std::span<std::string_view> args) {
        if (command == "quit") {
            return CommandResult::Quit;
        }

        if (auto itr = m_cmdHandlers.find(command); itr != m_cmdHandlers.end()) {
            itr->second(args);
            return CommandResult::Continue;
        }

        return CommandResult::Unknown;
    }

    void UciLikeHandler::printSearchInfo(std::ostream& stream, const SearchInfo& info) const {
        stream << "info depth " << info.depth;

        if (info.seldepth) {
            stream << " seldepth " << *info.seldepth;
        }

        if (info.timeSec) {
            const auto ms = static_cast<usize>(*info.timeSec * 1000.0);
            stream << " time " << ms;
        }

        stream << " nodes " << info.nodes;

        if (info.timeSec) {
            const auto nps = static_cast<usize>(static_cast<f64>(info.nodes) / *info.timeSec);
            stream << " nps " << nps;
        }

        stream << " pv";

        for (usize i = 0; i < info.pv.length; ++i) {
            stream << ' ';
            printMove(stream, info.pv.moves[i]);
        }

        stream << std::endl;
    }

    void UciLikeHandler::printInfoString(std::ostream& stream, std::string_view str) const {
        stream << "info string " << str << std::endl;
    }

    void UciLikeHandler::printBestMove(std::ostream& stream, Move move) const {
        stream << "bestmove ";
        printMove(stream, move);
        stream << std::endl;
    }

    void UciLikeHandler::registerCommandHandler(std::string_view command, CommandHandlerType handler) {
        if (m_cmdHandlers.contains(command)) {
            std::cerr << "tried to overwrite command handler for '" << command << "'" << std::endl;
            return;
        }

        m_cmdHandlers[std::string{command}] = std::move(handler);
    }

    void UciLikeHandler::handleNewGame() {
        //
    }

    void UciLikeHandler::handle_isready([[maybe_unused]] std::span<std::string_view> args) {
        std::cout << "readyok" << std::endl;
    }

    void UciLikeHandler::handle_position(std::span<std::string_view> args) {
        if (args.empty()) {
            return;
        }

        usize next = 0;

        if (args[0] == "startpos") {
            m_state.pos = Position::startpos();
            m_state.keyHistory.clear();

            next = 1;
        } else {
            const auto count = std::distance(args.begin(), std::ranges::find(args, "moves"));
            if (auto parsed = parsePosition(args.subspan(0, count))) {
                m_state.pos = parsed.take();
                m_state.keyHistory.clear();
            } else {
                if (const auto err = parsed.takeErr()) {
                    std::cerr << *err << std::endl;
                }
                return;
            }

            next = count;
        }

        assert(next <= args.size());

        if (next >= args.size() || args[next] != "moves") {
            return;
        }

        for (usize i = next + 1; i < args.size(); ++i) {
            if (auto parsedMove = parseMove(args[i])) {
                m_state.keyHistory.push_back(m_state.pos.key());
                m_state.pos = m_state.pos.applyMove(parsedMove.take());
            } else {
                std::cerr << "Invalid move '" << args[i] << "'" << std::endl;
                break;
            }
        }
    }

    void UciLikeHandler::handle_go(std::span<std::string_view> args) {
        movegen::MoveList moves{};
        movegen::generateAll(moves, m_state.pos);

        u32 start = 0;

        while (true) {
            const auto idx = start + m_rng.nextU32(moves.size() - start);
            const auto move = moves[idx];

            if (m_state.pos.isLegal(move)) {
                m_state.keyHistory.push_back(m_state.pos.key());
                const auto newPos = m_state.pos.applyMove(move);
                const auto sennichite = newPos.testSennichite(m_state.keyHistory);
                m_state.keyHistory.pop_back();

                // avoid accidental perpetual
                if (sennichite != SennichiteStatus::Win) {
                    auto pv = PvList{};
                    pv.moves[0] = move;
                    pv.length = 1;

                    const SearchInfo info = {
                        .depth = 1,
                        .seldepth = 1,
                        .nodes = 1,
                        .score = CpDisplayScore{0},
                        .pv = pv,
                    };

                    printSearchInfo(std::cout, info);
                    printBestMove(std::cout, move);

                    break;
                }
            }

            std::swap(moves[start], moves[idx]);
            ++start;
        }
    }

    void UciLikeHandler::handle_d([[maybe_unused]] std::span<std::string_view> args) {
        const auto printKey = [](u64 key) {
            std::ostringstream str{};
            str << "0x" << std::hex << std::setw(16) << std::setfill('0') << key;
            std::cout << str.view();
        };

        std::cout << '\n';
        printBoard(std::cout, m_state.pos);

        std::cout << "\n\n";
        printFenLine(std::cout, m_state.pos);

        std::cout << "Key: ";
        printKey(m_state.pos.key());

        std::cout << "\nCheckers:";

        auto checkers = m_state.pos.checkers();
        while (!checkers.empty()) {
            std::cout << ' ' << checkers.popLsb();
        }

        std::cout << "\nPinned:";

        auto pinned = m_state.pos.pinned();
        while (!pinned.empty()) {
            std::cout << ' ' << pinned.popLsb();
        }

        std::cout << std::endl;
    }

    void UciLikeHandler::handle_splitperft(std::span<std::string_view> args) {
        if (args.empty()) {
            return;
        }

        if (const auto depth = util::tryParse<i32>(args[0])) {
            splitPerft(m_state.pos, *depth);
        }
    }
} // namespace stoat::protocol
