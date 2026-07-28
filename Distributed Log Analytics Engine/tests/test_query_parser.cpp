#include <iostream>

#include "../index/query_parser.h"

int main()
{
    QueryParser parser(
        "service:auth AND (level:ERROR OR timeout)"
    );

    auto tree =
        parser.parse();

    std::cout
        << tree->to_string()
        << "\n";
}