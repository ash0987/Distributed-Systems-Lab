#include <chrono>
#include <filesystem>
#include <iostream>

#include "../index/inverted_index.h"
#include "../index/query_ast.h"
#include "../index/query_executor.h"
#include "../index/query_parser.h"
#include "../index/tokenizer.h"

#include "../index/record_locator.h"
#include "../ingest/segment_reader.h"

namespace fs = std::filesystem;

int main()
{
    InvertedIndex index;
    std::vector<RecordLocation> locations;

    int next_id = 0;

    std::cout << "Building index..." << std::endl;

    for (const auto& entry : fs::directory_iterator("./tools"))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        if (entry.path().extension() != ".segment")
        {
            continue;
        }

        std::string filename =
            entry.path().string();

        std::vector<LogRecord> records =
            SegmentReader::read_segment(filename);

        for (int i = 0;
             i < static_cast<int>(records.size());
             i++)
        {
            auto terms =
                tokenize(records[i]);

            index.add_record(
                next_id,
                terms
            );

            locations.push_back(
                {
                    filename,
                    i
                });

            next_id++;
        }
    }

    std::cout << "Indexed "
              << next_id
              << " records\n";

    QueryExecutor executor(
        index,
        locations
    );

    //
    // Boolean query benchmark
    //

    QueryParser parser(
        "service:data-node AND level:error"
    );

    auto tree =
        parser.parse();

    auto start =
        std::chrono::steady_clock::now();

    auto result =
        executor.execute(*tree);

    auto end =
        std::chrono::steady_clock::now();

    auto boolean_ms =
        std::chrono::duration_cast<
            std::chrono::milliseconds>(
            end - start);

    std::cout
        << "\nBoolean Query\n";

    std::cout
        << "Matches : "
        << result.size()
        << "\n";

    std::cout
        << "Latency : "
        << boolean_ms.count()
        << " ms\n";

    //
    // Regex benchmark
    //

    RegexNode regex(
        "message",
        "circuit breaker"
    );

    start =
        std::chrono::steady_clock::now();

    auto regex_result =
        executor.execute(regex);

    end =
        std::chrono::steady_clock::now();

    auto regex_ms =
        std::chrono::duration_cast<
            std::chrono::milliseconds>(
            end - start);

    std::cout
        << "\nRegex Query\n";

    std::cout
        << "Matches : "
        << regex_result.size()
        << "\n";

    std::cout
        << "Latency : "
        << regex_ms.count()
        << " ms\n";

    return 0;
}