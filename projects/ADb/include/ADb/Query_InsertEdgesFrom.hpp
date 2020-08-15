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

#ifndef ADB_QUERY_INSERTEDGESFROM_HPP
#define ADB_QUERY_INSERTEDGESFROM_HPP

#include "Query.hpp"

namespace adb
{
class Query::InsertEdgesFrom : public Query::Wrapper
{
public:
    using Query::Wrapper::Wrapper;

    [[nodiscard]] auto from(std::vector<acore::size_type> ids) && -> Query::InsertEdgesTo;
    [[nodiscard]] auto from(const Placeholder::Ids &placeholder) && -> Query::InsertEdgesTo;
    [[nodiscard]] auto from(Query::Ids subQuery) && -> Query::InsertEdgesTo;
    [[nodiscard]] auto fromEach(std::vector<acore::size_type> ids) && -> Query::InsertEdgesToEach;
    [[nodiscard]] auto fromEach(const Placeholder::Ids &placeholder) && -> Query::InsertEdgesToEach;
    [[nodiscard]] auto fromEach(Query::Ids subQuery) && -> Query::InsertEdgesToEach;
};
}

#endif
