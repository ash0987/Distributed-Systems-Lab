#include "segment_reader.h"
#include <fstream>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <zstd.h>

std::vector<LogRecord> SegmentReader::read_segment(
    const std::string& filename
)
{
    // Open file
    std::ifstream file(
        filename,
        std::ios::binary
    );

    if (!file.is_open())
    {
        throw std::runtime_error(
            "Could not open segment file: "
            + filename
        );
    }

    //
    // Read original (uncompressed) size
    //

    uint64_t original_size = 0;

    file.read(
        reinterpret_cast<char*>(&original_size),
        sizeof(original_size)
    );

    if (!file)
    {
        throw std::runtime_error(
            "Could not read segment header: "
            + filename
        );
    }

    //
    // Determine compressed payload size
    //

    file.seekg(0, std::ios::end);

    std::streampos end_pos =
        file.tellg();

    std::streamsize compressed_size =
        end_pos -
        static_cast<std::streamoff>(sizeof(uint64_t));

    if (compressed_size < 0)
    {
        throw std::runtime_error(
            "Invalid segment file: "
            + filename
        );
    }

    file.seekg(
        sizeof(uint64_t),
        std::ios::beg
    );

    //
    // Read compressed bytes
    //

    std::vector<char> compressed_buffer(
        compressed_size
    );

    file.read(
        compressed_buffer.data(),
        compressed_size
    );

    if (!file)
    {
        throw std::runtime_error(
            "Could not read compressed payload: "
            + filename
        );
    }

    file.close();

    //
    // Allocate decompression buffer
    //

    std::vector<char> raw_buffer(
        original_size
    );

    //
    // Decompress
    //

    size_t decompressed_size =
        ZSTD_decompress(
            raw_buffer.data(),
            original_size,
            compressed_buffer.data(),
            compressed_size
        );

    if (ZSTD_isError(decompressed_size))
    {
        throw std::runtime_error(
            std::string("ZSTD decompression failed: ")
            + ZSTD_getErrorName(decompressed_size)
        );
    }

    if (decompressed_size != original_size)
    {
        throw std::runtime_error(
            "Unexpected decompressed size."
        );
    }

    //
    // Convert into string
    //

    std::string raw_text(
        raw_buffer.begin(),
        raw_buffer.end()
    );

    //
    // Parse lines
    //

    std::vector<LogRecord> records;

    size_t start = 0;

    while (start < raw_text.size())
    {
        size_t end =
            raw_text.find('\n', start);

        if (end == std::string::npos)
        {
            end = raw_text.size();
        }

        // Skip empty lines.
        if (end > start)
        {
            std::string line =
                raw_text.substr(
                    start,
                    end - start
                );

            LogRecord record;

            if (!parse_log_line(
                    line,
                    record
                ))
            {
                throw std::runtime_error(
                    "Failed parsing record: "
                    + line
                );
            }

            records.push_back(
                std::move(record)
            );
        }

            start = end + 1;
    }

    return records;
}