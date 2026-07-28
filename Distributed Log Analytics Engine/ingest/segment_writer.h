#pragma once

#include "../core/log_record.h"

#include <string>
#include <vector>
#include <map>


class SegmentWriter
{
private:

    std::string service_name_;

    // Keeps records separated by hour bucket
    std::map<std::string, std::vector<LogRecord>> buffers_;


    std::string get_bucket(
        const std::string& timestamp
    );

    uint64_t total_raw_bytes_ = 0;
    uint64_t total_compressed_bytes_ = 0;

public:

    explicit SegmentWriter(
        const std::string& service_name
    );

    double compression_ratio() const;

    void add(
        const LogRecord& record
    );


    void flush();
};