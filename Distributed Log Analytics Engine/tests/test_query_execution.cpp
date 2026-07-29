#include <iostream>
#include <memory>
#include <vector>

#include "../index/inverted_index.h"
#include "../index/query_ast.h"
#include "../index/query_executor.h"

int main()
{
    InvertedIndex index;

    index.add_record(
        0,
        {
            "service:auth",
            "level:error"
        }
    );

    index.add_record(
        1,
        {
            "service:api",
            "level:info"
        }
    );

    index.add_record(
        2,
        {
            "service:auth",
            "level:warn"
        }
    );

    index.add_record(
        3,
        {
            "service:web",
            "level:error"
        }
    );

    index.add_record(
        4,
        {
            "service:db",
            "level:info"
        }
    );

    std::vector<RecordLocation> locations =
    {
        {"", 0},
        {"", 0},
        {"", 0},
        {"", 0},
        {"", 0}
    };

    QueryExecutor executor(
        index,
        locations
    );

    std::unique_ptr<QueryNode> query =
        std::make_unique<NotNode>(
            std::make_unique<FieldMatchNode>(
                "service",
                "auth"
            )
        );

    auto result =
        executor.execute(*query);

    std::cout << "Matching record IDs:\n";

    for (int id : result)
    {
        std::cout << id << " ";
    }

    std::cout << std::endl;

    return 0;
}