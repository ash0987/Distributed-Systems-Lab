#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>


class WalWriter
{
private:

    std::string path_;
    std::ofstream file_;
    int append_count_ = 0;
    static constexpr int FLUSH_INTERVAL = 1000;


public:

    // Opens WAL file in append mode
    explicit WalWriter(const std::string& path)
    : path_(path)
    {
    file_.open(path_, std::ios::out | std::ios::app);
        if (!file_.is_open()) {
            std::cerr << "ERROR: could not open WAL file at: " << path_ << "\n";
            std::exit(1);
        }
    }


    ~WalWriter()
    {
        if (file_.is_open())
        {
            file_.close();
        }
    }


    // Append one raw log line to WAL
    void append(const std::string& raw_line)
    {
       if (!file_.is_open()) return;
        file_ << raw_line << "\n";

        append_count_++;
        if (append_count_ >= FLUSH_INTERVAL) {
            file_.flush();
            append_count_ = 0;
        }
    }



    // Clear WAL after records are safely persisted elsewhere
    void truncate()
    {
        if (file_.is_open())
        {
            file_.close();
        }
        append_count_ = 0;

        std::ofstream clear_file(
            path_,
            std::ios::out | std::ios::trunc
        );


        clear_file.close();


        // Reopen for future appends
        file_.open(
            path_,
            std::ios::out | std::ios::app
        );
    }



    // Prevent copying
    WalWriter(const WalWriter&) = delete;

    WalWriter& operator=(const WalWriter&) = delete;
};