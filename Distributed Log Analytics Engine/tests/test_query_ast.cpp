#include <iostream>
#include <memory>

#include "../index/query_ast.h"

int main()
{
    auto query =
        std::make_unique<AndNode>(
            std::make_unique<FieldMatchNode>(
                "service",
                "auth"
            ),
            std::make_unique<OrNode>(
                std::make_unique<FieldMatchNode>(
                    "level",
                    "ERROR"
                ),
                std::make_unique<TermNode>(
                    "timeout"
                )
            )
        );

    std::cout
        << query->to_string()
        << "\n";

    return 0;
}