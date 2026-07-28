#include "ingest/segment_reader.h"
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./segment_reader_test <segment_file>\n";
        return 1;
    }

    auto records = SegmentReader::read_segment(argv[1]);

    std::cout << "Read " << records.size() << " records\n";

    for (size_t i = 0; i < 5 && i < records.size(); i++)
    {
        std::cout << records[i].timestamp << " | " << records[i].host << " | "
                   << records[i].service << " | " << records[i].message << "\n";
    }

    return 0;
}