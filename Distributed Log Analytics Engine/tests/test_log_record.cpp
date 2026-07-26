#include "core/log_record.h"
#include <iostream>


int main()
{
    std::string line =
        "2026-07-26T10:10:00Z|data-node-1|data-node|5|10|1234|INFO|Reading block id=req-100";


    LogRecord record;


    if(parse_log_line(line, record))
    {
        std::cout
            << record.service
            << " "
            << record.host
            << " "
            << record.seq_no
            << " "
            << record.message
            << "\n";
    }
    else
    {
        std::cout << "parse failed"<<std::endl;
    }
}