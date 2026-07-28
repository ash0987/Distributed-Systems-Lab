#pragma once

#include <string>
#include <vector>

#include "../core/log_record.h"

class SegmentReader
{
public:

    static std::vector<LogRecord> read_segment(
        const std::string& filename
    );
};