#pragma once

#include <string>
#include <cstdint>


struct LogRecord
{
    std::string timestamp;
    std::string host;
    std::string service;

    uint64_t seq_no;

    int thread_id;
    int process_id;

    std::string level;
    std::string message;
};


// Parse one raw generator line:
// timestamp|host|service|seq|thread_id|process_id|level|message

bool parse_log_line(
    const std::string& line,
    LogRecord& record
);