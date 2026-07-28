#pragma once

#include <memory>
#include <string>
#include <vector>

#include "query_ast.h"

class QueryParser
{
public:

    explicit QueryParser(
        const std::string& query
    );

    std::unique_ptr<QueryNode> parse();

private:

    //
    // Recursive-descent parsing
    //

    std::unique_ptr<QueryNode> parse_or();

    std::unique_ptr<QueryNode> parse_and();

    std::unique_ptr<QueryNode> parse_not();

    std::unique_ptr<QueryNode> parse_primary();

    //
    // Helpers
    //

    bool match(
        const std::string& token
    );

    bool has_more() const;

    const std::string& peek() const;

    std::string consume();

    void tokenize(
        const std::string& query
    );

private:

    std::vector<std::string> tokens_;

    size_t current_ = 0;
};