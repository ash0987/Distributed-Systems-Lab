#include <iostream>

#include "../index/inverted_index.h"

int main()
{
    InvertedIndex index;

    index.add_record(
        0,
        {
            "error",
            "disk"
        }
    );

    index.add_record(
        1,
        {
            "error",
            "network"
        }
    );

    std::cout << "error:\n";

    for (int id : index.lookup("error"))
    {
        std::cout << id << " ";
    }

    std::cout << "\n\n";

    std::cout << "disk:\n";

    for (int id : index.lookup("disk"))
    {
        std::cout << id << " ";
    }

    std::cout << "\n\n";

    std::cout << "network:\n";

    for (int id : index.lookup("network"))
    {
        std::cout << id << " ";
    }

    std::cout << "\n\n";

    std::cout << "timeout:\n";

    for (int id : index.lookup("timeout"))
    {
        std::cout << id << " ";
    }

    std::cout << "\n\n";

    std::cout
        << "Distinct terms: "
        << index.term_count()
        << "\n";

    return 0;
}