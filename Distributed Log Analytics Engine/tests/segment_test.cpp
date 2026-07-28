#include "../ingest/segment_writer.h"

#include <iostream>


int main()
{
    SegmentWriter writer("data-node");


    LogRecord hour11_first;

    hour11_first.timestamp = "2026-07-26T11:05:00Z";
    hour11_first.host = "data-node-1";
    hour11_first.service = "data-node";
    hour11_first.seq_no = 1;
    hour11_first.thread_id = 2;
    hour11_first.process_id = 1000;
    hour11_first.level = "INFO";
    hour11_first.message = "hour 11 first";



    LogRecord hour10 = hour11_first;

    hour10.timestamp = "2026-07-26T10:30:00Z";
    hour10.seq_no = 2;
    hour10.message = "hour 10 late arrival";



    LogRecord hour11_second = hour11_first;

    hour11_second.timestamp = "2026-07-26T11:45:00Z";
    hour11_second.seq_no = 3;
    hour11_second.message = "hour 11 second";



    // Intentionally out of order
    writer.add(hour11_first);
    writer.add(hour10);
    writer.add(hour11_second);
    // Flush everything once
    writer.flush();


    std::cout << "segments written\n";

    return 0;
}