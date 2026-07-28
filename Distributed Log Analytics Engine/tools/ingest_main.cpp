#include "../core/log_record.h"
#include "../ingest/thread_safe_queue.h"
#include "../ingest/wal_writer.h"
#include "../ingest/segment_writer.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <map>
#include <chrono>
#include <filesystem>


using Queue = ThreadSafeQueue<LogRecord>;


// Worker thread
void worker(
    Queue& queue,
    WalWriter& wal,
    SegmentWriter& segment_writer
)
{
    LogRecord record;


    while (queue.pop(record))
    {
        std::string raw_line = serialize_record(record);

        // durability first
        wal.append(raw_line);

        // then memory buffer
        segment_writer.add(record);
    }


    // write remaining buffered records
    segment_writer.flush();
    wal.truncate();
}



// Reader thread
void reader(
    const std::string& filename,
    std::map<std::string, Queue*>& queues
)
{
    std::ifstream file(filename);


    if (!file.is_open())
    {
        std::cerr
            << "Cannot open input file: "
            << filename
            << std::endl;


        return;
    }



    std::string line;


    while (std::getline(file, line))
    {
        LogRecord record;


        if (!parse_log_line(line, record))
        {
            continue;
        }



        auto it = queues.find(record.service);


        if (it != queues.end())
        {
            it->second->push(std::move(record));
        }
    }



    // signal all workers
    for (auto& [service, queue] : queues)
    {
        queue->shutdown();
    }
}



int main(
    int argc,
    char* argv[]
)
{
    if (argc != 2)
    {
        std::cerr
            << "Usage: ./ingest_main <generated_logs.txt>"
            << std::endl;

        return 1;
    }



    std::string input_file = argv[1];



    // ----------------------------
    // Queues
    // ----------------------------

    Queue data_node_queue;
    Queue metadata_queue;
    Queue api_queue;
    Queue kms_queue;
    Queue auth_queue;



    // ----------------------------
    // WAL writers
    // ----------------------------

    WalWriter data_node_wal(
        "data-node.wal"
    );

    WalWriter metadata_wal(
        "metadata-store.wal"
    );

    WalWriter api_wal(
        "api.wal"
    );

    WalWriter kms_wal(
        "kms.wal"
    );

    WalWriter auth_wal(
        "auth.wal"
    );



    // ----------------------------
    // Segment writers
    // ----------------------------

    SegmentWriter data_node_segment(
        "data-node"
    );

    SegmentWriter metadata_segment(
        "metadata-store"
    );

    SegmentWriter api_segment(
        "api"
    );

    SegmentWriter kms_segment(
        "kms"
    );

    SegmentWriter auth_segment(
        "auth"
    );



    // ----------------------------
    // Service routing table
    // ----------------------------

    std::map<std::string, Queue*> queues = {

        {"data-node", &data_node_queue},
        {"metadata-store", &metadata_queue},
        {"api", &api_queue},
        {"kms", &kms_queue},
        {"auth", &auth_queue}
    };



    auto start =
        std::chrono::steady_clock::now();



    // ----------------------------
    // Workers
    // ----------------------------

    std::thread data_worker(
        worker,
        std::ref(data_node_queue),
        std::ref(data_node_wal),
        std::ref(data_node_segment)
    );


    std::thread metadata_worker(
        worker,
        std::ref(metadata_queue),
        std::ref(metadata_wal),
        std::ref(metadata_segment)
    );


    std::thread api_worker(
        worker,
        std::ref(api_queue),
        std::ref(api_wal),
        std::ref(api_segment)
    );


    std::thread kms_worker(
        worker,
        std::ref(kms_queue),
        std::ref(kms_wal),
        std::ref(kms_segment)
    );


    std::thread auth_worker(
        worker,
        std::ref(auth_queue),
        std::ref(auth_wal),
        std::ref(auth_segment)
    );



    // ----------------------------
    // Reader
    // ----------------------------

    std::thread reader_thread(
        reader,
        input_file,
        std::ref(queues)
    );



    // Wait for completion

    reader_thread.join();


    data_worker.join();
    metadata_worker.join();
    api_worker.join();
    kms_worker.join();
    auth_worker.join();

    std::cout << "Compression ratios:\n";
    std::cout << "  data-node: " << data_node_segment.compression_ratio() << "\n";
    std::cout << "  metadata-store: " << metadata_segment.compression_ratio() << "\n";
    std::cout << "  api: " << api_segment.compression_ratio() << "\n";
    std::cout << "  kms: " << kms_segment.compression_ratio() << "\n";
    std::cout << "  auth: " << auth_segment.compression_ratio() << "\n";


    auto end =
        std::chrono::steady_clock::now();



    double seconds =
        std::chrono::duration<double>(
            end - start
        ).count();



    uintmax_t bytes =
        std::filesystem::file_size(input_file);



    double mb =
        static_cast<double>(bytes)
        /
        (1024.0 * 1024.0);

    double throughput =
        mb / seconds;



    std::cout
        << "Ingestion complete\n"
        << "Input size: "
        << mb
        << " MB\n"
        << "Time: "
        << seconds
        << " seconds\n"
        << "Throughput: "
        << throughput
        << " MB/s\n";

    return 0;
}