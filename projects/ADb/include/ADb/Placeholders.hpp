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

#ifndef ADB_PLACEHOLDERS_HPP
#define ADB_PLACEHOLDERS_HPP

#include "ADbModule.hpp"
#include "KeyValue.hpp"
#include "QueryData.hpp"

#include <vector>

namespace adb
{
//! \relates adb::Query
//! Types of values that can be bound to a query
//! data.
using PlaceholderValue = std::variant<acore::size_type,
                                      std::vector<acore::size_type>,
                                      std::vector<adb::KeyValue>,
                                      std::vector<std::vector<adb::KeyValue>>>;

//! \relates adb::Query
//! Wrapper around the name of placeholder.
struct PlaceholderBase
{
    //! Constructs the placeholder with name \a n.
    PlaceholderBase(std::string n) :
        name(std::move(n))
    {
    }

    //! Placeholder's name.
    std::string name;
};

//! \relates adb::Query
//! Used when the placeholder represents count.
struct PlaceholderCount : PlaceholderBase
{
    using PlaceholderBase::PlaceholderBase;
};

//! \relates adb::Query
//! Used when the placeholder represents an element
//! id.
struct PlaceholderId : PlaceholderBase
{
    using PlaceholderBase::PlaceholderBase;
};

//! \relates adb::Query
//! Used when the placeholder represents values.
struct PlaceholderValues : PlaceholderBase
{
    using PlaceholderBase::PlaceholderBase;
};

//! \cond IMPLEMENTAION_DETAIL
using BindPlaceholderFunction = auto (*)(PlaceholderValue &&value, QueryData *data) -> void;

inline auto bindInsertNodeValues(PlaceholderValue &&value, QueryData *data)
{
    std::get<InsertNodeValues>(*data).values = std::move(std::get<std::vector<adb::KeyValue>>(value));
}

inline auto bindInsertNodesValues(PlaceholderValue &&value, QueryData *data) -> void
{
    std::get<InsertNodesValues>(*data).values = std::move(std::get<std::vector<std::vector<adb::KeyValue>>>(value));
}

inline auto bindInsertNodesCount(PlaceholderValue &&value, QueryData *data) -> void
{
    std::get<InsertNodesCount>(*data).count = std::get<acore::size_type>(value);
}

inline auto bindInsertEdgesCountFrom(PlaceholderValue &&value, QueryData *data) -> void
{
    std::get<InsertEdgesCount>(*data).from = std::get<acore::size_type>(value);
}

inline auto bindInsertEdgesCountTo(PlaceholderValue &&value, QueryData *data) -> void
{
    std::get<InsertEdgesCount>(*data).to = std::get<acore::size_type>(value);
}

struct Placeholder
{
    std::string name;
    BindPlaceholderFunction bind;
};
//! \endcond
}

#endif