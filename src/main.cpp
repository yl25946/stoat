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

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "protocol/handler.h"
#include "util/split.h"

using namespace stoat;

i32 main() {
    protocol::EngineState state{};

    std::string_view currHandler = protocol::kDefaultHandler;
    auto handler = protocol::createHandler(currHandler, state);

    std::vector<std::string_view> tokens{};

    std::string line{};
    while (std::getline(std::cin, line)) {
        tokens.clear();
        util::split(tokens, line);

        if (tokens.empty()) {
            continue;
        }

        const auto command = tokens[0];
        const auto args = std::span{tokens}.subspan<1>();

        if (command == currHandler) {
            handler->printInitialInfo();
            continue;
        }

        const auto result = handler->handleCommand(command, args);

        if (result == protocol::CommandResult::Quit) {
            break;
        } else if (result == protocol::CommandResult::Unknown) {
            if (auto newHandler = protocol::createHandler(command, state)) {
                currHandler = command;
                handler = std::move(newHandler);

                handler->printInitialInfo();
            } else {
                std::cerr << "Unknown command '" << command << "'" << std::endl;
            }
        }
    }
}
