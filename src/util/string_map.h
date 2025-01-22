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

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace stoat::util {
    struct StringHash {
        using is_transparent = void;

        using TransparentKeyEqual = std::equal_to<>;

        [[nodiscard]] usize operator()(const char* str) const {
            return std::hash<std::string_view>{}(str);
        }

        [[nodiscard]] usize operator()(const std::string& str) const {
            return std::hash<std::string_view>{}(str);
        }

        [[nodiscard]] usize operator()(std::string_view str) const {
            return std::hash<std::string_view>{}(str);
        }
    };

    template <typename T, typename Allocator = std::allocator<std::pair<const std::string, T>>>
    using UnorderedStringMap =
        std::unordered_map<std::string, T, StringHash, StringHash::TransparentKeyEqual, Allocator>;

    template <typename Allocator = std::allocator<const std::string>>
    using UnorderedStringSet = std::unordered_set<std::string, StringHash, StringHash::TransparentKeyEqual, Allocator>;
} // namespace stoat::util
