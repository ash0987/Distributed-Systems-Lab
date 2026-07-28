#include "ingest/wal_writer.h"

#include <iostream>
#include <fstream>

int main()
{
    WalWriter wal("test_service.wal");

    wal.append("2026-07-26T10:00:00Z|data-node-1|data-node|1|2|1000|INFO|hello");
    wal.append("2026-07-26T10:00:01Z|data-node-2|data-node|2|3|1001|ERROR|failed");

    // CHECK 1: read here, right after the two appends, before truncate
    std::cout << "--- after appends ---\n";
    {
        std::ifstream file("test_service.wal");
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << std::endl;
        }
    }

    wal.truncate();

    // CHECK 2: read here, right after truncate — expect nothing printed
    std::cout << "--- after truncate ---\n";
    {
        std::ifstream file("test_service.wal");
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << std::endl;
        }
    }

    wal.append("2026-07-26T10:00:02Z|data-node-3|data-node|1|1|1002|INFO|new entry after truncate");

    // CHECK 3: read here — expect ONLY the new entry, not the old two
    std::cout << "--- after append post-truncate ---\n";
    {
        std::ifstream file("test_service.wal");
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << std::endl;
        }
    }

    return 0;
}