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

#ifndef ADB_QUERY_INSERTNODESVALUES_HPP
#define ADB_QUERY_INSERTNODESVALUES_HPP

#include "Query.hpp"

namespace adb
{
class Query::InsertNodesValues : public Query::Wrapper
{
public:
    using Query::Wrapper::Wrapper;

    [[nodiscard]] auto values(std::vector<adb::KeyValue> values) && -> Query::Ids;
    [[nodiscard]] auto values(const Placeholder::Values &placeholder) && -> Query::Ids;
    [[nodiscard]] auto values(Query::Values subQuery) && -> Query::Ids;
    [[nodiscard]] auto values(std::vector<std::vector<adb::KeyValue>> values) && -> Query::Ids;
    [[nodiscard]] auto values(const Placeholder::MultiValues &placeholder) && -> Query::Ids;
    [[nodiscard]] auto values(Query::MultiValues subQuery) && -> Query::Ids;
};
}

#endif
