#include "log_record.h"

#include <sstream>
#include <vector>


static std::vector<std::string> split(
    const std::string& line,
    char delimiter
)
{
    std::vector<std::string> parts;
    parts.reserve(8);

    size_t start = 0;

    for (int i = 0; i < 7; i++)
    {
        size_t pos = line.find(delimiter, start);

        if (pos == std::string::npos)
        {
            return {};
        }

        parts.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }

    // Last field (message)
    parts.push_back(line.substr(start));

    return parts;
}



bool parse_log_line(
    const std::string& line,
    LogRecord& record
)
{
    auto parts = split(line, '|');


    // Expected:
    //
    // 0 timestamp
    // 1 host
    // 2 service
    // 3 seq
    // 4 thread
    // 5 process
    // 6 level
    // 7 message


    if (parts.size() != 8)
    {
        return false;
    }


    try
    {
        record.timestamp = parts[0];

        record.host = parts[1];

        record.service = parts[2];


        record.seq_no =
            std::stoull(parts[3]);


        record.thread_id =
            std::stoi(parts[4]);


        record.process_id =
            std::stoi(parts[5]);


        record.level = parts[6];

        record.message = parts[7];
    }
    catch (...)
    {
        return false;
    }


    return true;
}

std::string serialize_record(
    const LogRecord& record
)
{
    return
        record.timestamp + "|" +
        record.host + "|" +
        record.service + "|" +
        std::to_string(record.seq_no) + "|" +
        std::to_string(record.thread_id) + "|" +
        std::to_string(record.process_id) + "|" +
        record.level + "|" +
        record.message;
}
