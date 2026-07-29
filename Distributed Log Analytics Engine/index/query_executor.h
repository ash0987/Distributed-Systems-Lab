#pragma once

#include <string>
#include <vector>

#include "inverted_index.h"
#include "query_ast.h"

struct RecordLocation
{
    std::string segment_file;
    int position;
};

class QueryExecutor
{
public:

    QueryExecutor(
        const InvertedIndex& index,
        const std::vector<RecordLocation>& locations
    );

    std::vector<int> execute(
        const QueryNode& query
    ) const;

    const InvertedIndex& index() const;

    std::string get_message(
        int record_id
    ) const;

private:

    const InvertedIndex& index_;

    const std::vector<RecordLocation>& locations_;
};