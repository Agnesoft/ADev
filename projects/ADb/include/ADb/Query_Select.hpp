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

#ifndef ADB_QUERY_SELECT_HPP
#define ADB_QUERY_SELECT_HPP

#include "Query.hpp"

namespace adb
{
//! The SelectQuery class helps constructing a
//! select query. Its public methods can only be
//! called on an r-value (temporary) object.
class Query::Select
{
public:
    //! DUMMY
    [[nodiscard]] auto count() && -> Query::Count;

    //! DUMMY
    [[nodiscard]] auto multiValues() && -> Query::MultiValues;

    //! DUMMY
    [[nodiscard]] auto values() && -> Query::Values;
};
}

#endif
