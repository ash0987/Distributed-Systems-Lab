#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "../ingest/segment_reader.h"
#include "../index/tokenizer.h"
#include "../index/inverted_index.h"

namespace fs = std::filesystem;

struct RecordLocation
{
    std::string segment_file;
    size_t record_position;
};

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr
            << "Usage: "
            << argv[0]
            << " <segment_directory>\n";

        return 1;
    }

    std::string segment_directory = argv[1];

    InvertedIndex index;

    std::vector<RecordLocation> side_table;

    int next_record_id = 0;

    auto start =
        std::chrono::steady_clock::now();

    for (const auto& entry :
         fs::directory_iterator(segment_directory))
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

        std::cout
            << "Reading "
            << filename
            << "\n";

        auto records =
            SegmentReader::read_segment(filename);

        for (size_t i = 0;
             i < records.size();
             i++)
        {
            auto terms =
                tokenize(records[i]);

            index.add_record(
                next_record_id,
                terms
            );

            side_table.push_back(
                {
                    filename,
                    i
                }
            );

            next_record_id++;
        }
    }

    auto end =
        std::chrono::steady_clock::now();

    double elapsed =
        std::chrono::duration<double>(
            end - start
        ).count();

    //
    // Estimate index size.
    //
    // Each posting stores one int.
    // Each term contributes its string bytes.
    //

    size_t estimated_bytes = 0;

    // NOTE:
    // This requires a getter from InvertedIndex.
    // See below.

    for (const auto& [term, postings] :
         index.data())
    {
        estimated_bytes += term.size();

        estimated_bytes +=
            postings.size() * sizeof(int);
    }

    std::cout << "\n";
    std::cout << "===========================\n";
    std::cout << "Index Build Complete\n";
    std::cout << "===========================\n";

    std::cout
        << "Records Indexed : "
        << next_record_id
        << "\n";

    std::cout
        << "Distinct Terms  : "
        << index.term_count()
        << "\n";

    std::cout
        << "Build Time      : "
        << elapsed
        << " sec\n";

    std::cout
        << "Estimated Index : "
        << estimated_bytes / (1024.0 * 1024.0)
        << " MB\n";

    return 0;
}