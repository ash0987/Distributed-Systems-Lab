#pragma once

#include <map>
#include <string>
#include <vector>

class InvertedIndex
{
private:

    std::map<
        std::string,
        std::vector<int>
    > index_;

public:
    const std::map<std::string,std::vector<int>>& data() const;

    void add_record(
        int record_id,
        const std::vector<std::string>& terms
    );

    const std::vector<int>& lookup(
        const std::string& term
    ) const;

    size_t term_count() const;
};