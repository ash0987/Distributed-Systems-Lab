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

    QueryParser parser2(
        "service:auth AND level:ERROR OR timeout"
    );

    auto tree2=
        parser2.parse();

    std::cout
        << tree2->to_string()
        << "\n";

    try
    {
        QueryParser parser3("a)");
        auto tree3 = parser3.parse();
        std::cout << tree3->to_string() << "\n";
    }
    catch (const std::exception& e)
    {
        std::cout << "threw: " << e.what() << "\n";
    }

     try
    {
        QueryParser parser3("(a AND)");
        auto tree3 = parser3.parse();
        std::cout << tree3->to_string() << "\n";
    }
    catch (const std::exception& e)
    {
        std::cout << "threw: " << e.what() << "\n";
    }

    try
    {
        QueryParser parser3("(a AND) OR (b)");
        auto tree3 = parser3.parse();
        std::cout << tree3->to_string() << "\n";
    }
    catch (const std::exception& e)
    {
        std::cout << "threw: " << e.what() << "\n";
    }
}