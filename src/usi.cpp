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

#include "usi.h"

#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "perft.h"
#include "position.h"
#include "util/parse.h"
#include "util/split.h"
#include "util/string_map.h"

namespace stoat::usi {
    namespace {
        constexpr std::string_view kName = "Stoat";
        constexpr std::string_view kVersion = ST_STRINGIFY(ST_VERSION);
        constexpr std::string_view kAuthor = "Ciekce";

        class UsiHandler {
        public:
            UsiHandler();

            i32 run();

        private:
            using CommandHandlerType = std::function<void(std::span<std::string_view>)>;

            util::UnorderedStringMap<CommandHandlerType> m_handlers{};

            Position m_pos{Position::startpos()};

            void handle_usi(std::span<std::string_view> args);
            void handle_usinewgame(std::span<std::string_view> args);
            void handle_isready(std::span<std::string_view> args);
            void handle_position(std::span<std::string_view> args);

            // nonstandard
            void handle_d(std::span<std::string_view> args);
            void handle_splitperft(std::span<std::string_view> args);
        };

        UsiHandler::UsiHandler() {
#define REGISTER_HANDLER(Command) m_handlers[#Command] = [this](auto tokens) { handle_##Command(tokens); }

            REGISTER_HANDLER(usi);
            REGISTER_HANDLER(usinewgame);
            REGISTER_HANDLER(isready);
            REGISTER_HANDLER(position);

            REGISTER_HANDLER(d);
            REGISTER_HANDLER(splitperft);

#undef REGISTER_HANDLER
        }

        i32 UsiHandler::run() {
            std::vector<std::string_view> tokens{};

            std::string line{};
            while (std::getline(std::cin, line)) {
                tokens.clear();
                util::split(tokens, line);

                if (tokens.empty()) {
                    continue;
                }

                const auto command = tokens[0];

                if (command == "quit") {
                    break;
                } else if (auto itr = m_handlers.find(command); itr != m_handlers.end()) {
                    itr->second(std::span{tokens}.subspan<1>());
                } else {
                    std::cerr << "Unknown command '" << command << "'" << std::endl;
                }
            }

            return 0;
        }

        void UsiHandler::handle_usi([[maybe_unused]] std::span<std::string_view> args) {
            std::cout << "id name " << kName << ' ' << kVersion << '\n';
            std::cout << "id author " << kAuthor << '\n';

            std::cout << "usiok" << std::endl;
        }

        void UsiHandler::handle_usinewgame([[maybe_unused]] std::span<std::string_view> args) {
            //
        }

        void UsiHandler::handle_isready([[maybe_unused]] std::span<std::string_view> args) {
            std::cout << "readyok" << std::endl;
        }

        void UsiHandler::handle_position(std::span<std::string_view> args) {
            if (args.empty()) {
                return;
            }

            usize next = 0;

            if (args[0] == "startpos") {
                m_pos = Position::startpos();
                next = 1;
            } else if (args[0] == "sfen") {
                if (args.size() == 1) {
                    std::cerr << "Missing sfen" << std::endl;
                    return;
                }

                const auto count = std::distance(args.begin(), std::ranges::find(args, "moves")) - 1;
                if (auto parsed = Position::fromSfenParts(args.subspan(1, count))) {
                    m_pos = parsed.take();
                } else {
                    std::cerr << "Failed to parse sfen: " << parsed.takeErr().message() << std::endl;
                    return;
                }

                next = 1 + count;
            }

            assert(next <= args.size());

            if (next >= args.size() || args[next] != "moves") {
                return;
            }

            for (usize i = next + 1; i < args.size(); ++i) {
                auto parsedMove = Move::fromStr(args[i]);

                if (parsedMove) {
                    m_pos = m_pos.applyMove(parsedMove.take());
                } else {
                    std::cerr << "Invalid move '" << args[i] << "'" << std::endl;
                    break;
                }
            }
        }

        void UsiHandler::handle_d([[maybe_unused]] std::span<std::string_view> args) {
            const auto printKey = [](u64 key) {
                std::ostringstream str{};
                str << "0x" << std::hex << std::setw(16) << std::setfill('0') << key;
                std::cout << str.view();
            };

            std::cout << '\n' << m_pos;

            std::cout << "\n\nSfen: " << m_pos.sfen();

            std::cout << "\nKey: ";
            printKey(m_pos.key());

            std::cout << "\nCheckers:";

            auto checkers = m_pos.checkers();
            while (!checkers.empty()) {
                std::cout << ' ' << checkers.popLsb();
            }

            std::cout << "\nPinned:";

            auto pinned = m_pos.pinned();
            while (!pinned.empty()) {
                std::cout << ' ' << pinned.popLsb();
            }

            std::cout << std::endl;
        }

        void UsiHandler::handle_splitperft(std::span<std::string_view> args) {
            if (args.empty()) {
                return;
            }

            if (const auto depth = util::tryParse<i32>(args[0])) {
                splitPerft(m_pos, *depth);
            }
        }
    } // namespace

    i32 run() {
        UsiHandler handler{};
        return handler.run();
    }
} // namespace stoat::usi
