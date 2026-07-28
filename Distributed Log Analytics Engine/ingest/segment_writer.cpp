#include "segment_writer.h"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <cstdint>
#include <zstd.h>

SegmentWriter::SegmentWriter(
    const std::string& service_name
)
    : service_name_(service_name)
{
}



std::string SegmentWriter::get_bucket(
    const std::string& timestamp
)
{
    // Example:
    // 2026-07-26T10:15:00Z
    //
    // Bucket:
    // 2026-07-26T10

    if (timestamp.size() < 13)
    {
        return "unknown";
    }


    return timestamp.substr(0, 13);
}



void SegmentWriter::add(
    const LogRecord& record
)
{
    std::string bucket =
        get_bucket(record.timestamp);


    buffers_[bucket].push_back(record);
}

double SegmentWriter::compression_ratio() const
{
    if (total_raw_bytes_ == 0)
    {
        return 0.0;
    }

    return 1.0 -
        (static_cast<double>(total_compressed_bytes_) /
         static_cast<double>(total_raw_bytes_));
}

void SegmentWriter::flush()
{
    for (auto& [bucket, records] : buffers_)
    {
        if (records.empty())
        {
            continue;
        }

        std::string filename =
            service_name_
            + "_"
            + bucket
            + ".segment";

        // Build one big string containing all serialized records.
        std::string raw_data;

        for (const auto& record : records)
        {
            raw_data += serialize_record(record);
            raw_data += "\n";
        }

        // Compute the maximum compressed size.
        size_t compressed_capacity =
            ZSTD_compressBound(raw_data.size());

        std::vector<char> compressed_buffer(compressed_capacity);

        // Compress the data.
        size_t compressed_size =
            ZSTD_compress(
                compressed_buffer.data(),
                compressed_capacity,
                raw_data.data(),
                raw_data.size(),
                3   // Compression level
            );

        if (ZSTD_isError(compressed_size))
        {
            throw std::runtime_error(
                std::string("ZSTD compression failed: ")
                + ZSTD_getErrorName(compressed_size)
            );
        }

        // Track compression statistics.
        total_raw_bytes_ += raw_data.size();
        total_compressed_bytes_ += compressed_size;

        // Open the segment file in binary mode.
        std::ofstream file(
            filename,
            std::ios::out |
            std::ios::trunc |
            std::ios::binary
        );

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Could not open segment file: "
                + filename
            );
        }

        // Write the original (uncompressed) size as an 8-byte header.
        uint64_t original_size =
            static_cast<uint64_t>(raw_data.size());

        file.write(
            reinterpret_cast<const char*>(&original_size),
            sizeof(original_size)
        );

        // Write the compressed payload.
        file.write(
            compressed_buffer.data(),
            static_cast<std::streamsize>(compressed_size)
        );

        if (!file)
        {
            throw std::runtime_error(
                "Failed writing compressed segment: "
                + filename
            );
        }

        file.close();
    }

    buffers_.clear();
}