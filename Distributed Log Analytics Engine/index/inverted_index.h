
#pragma once

#include <map>
#include <string>
#include <vector>

class InvertedIndex
{
private:

    std::map<std::string, std::vector<int>> index_;

    size_t total_records_ = 0;

public:

    void add_record(
        int record_id,
        const std::vector<std::string>& terms
    );

    const std::vector<int>& lookup(
        const std::string& term
    ) const;

    size_t term_count() const;
    
    size_t total_records() const;

    const std::map<std::string, std::vector<int>>& data() const;

};