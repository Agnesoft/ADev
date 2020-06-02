// Copyright 2020 Michael Vlach
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ACORE_NUMERIC_HPP
#define ACORE_NUMERIC_HPP

#include "ACoreModule.hpp"

#include <cmath>

namespace acore
{
//! Returns the smallest ^2 of the \a value.
//! If the \a value is already ^2 this function
//! simply returns it.
[[nodiscard]] inline auto nextPowerOfTwo(size_type value) -> size_type
{
    if (value <= 2)
    {
        return 2;
    }

    return static_cast<size_type>(std::uint64_t{1} << (static_cast<std::uint64_t>(std::log2(value - 1)) + 1));
}
}

#endif