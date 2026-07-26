struct LogRecord {
    uint64_t timestamp;
    std::string host_id;
    std::string service_name;
    uint64_t seq_no;
    uint32_t thread_id;
    uint32_t process_id;
    std::string level;
    std::string message;
};
