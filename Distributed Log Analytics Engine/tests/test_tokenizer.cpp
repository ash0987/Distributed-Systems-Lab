#include <iostream>

#include "../index/tokenizer.h"

int main()
{
    LogRecord record;

    record.timestamp = "2026-07-26T10:15:00Z";
    record.host = "host1";
    record.service = "Data-Node";
    record.seq_no = 1;
    record.thread_id = 100;
    record.process_id = 200;
    record.level = "ERROR";
    record.message =
        "Disk failure while reading block, retry after 5 seconds.";

    auto terms =
        tokenize(record);

    for (const auto& term : terms)
    {
        std::cout
            << term
            << "\n";
    }

    return 0;
}