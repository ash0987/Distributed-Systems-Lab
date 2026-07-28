#include "inverted_index.h"

void InvertedIndex::add_record(
    int record_id,
    const std::vector<std::string>& terms
)
{
    for (const auto& term : terms)
    {
        index_[term].push_back(record_id);
    }
}



const std::vector<int>& InvertedIndex::lookup(
    const std::string& term
) const
{
    static const std::vector<int> empty;

    auto it = index_.find(term);

    if (it == index_.end())
    {
        return empty;
    }

    return it->second;
}

const std::map<std::string,std::vector<int>>& InvertedIndex::data() const
{
    return index_;
}

size_t InvertedIndex::term_count() const
{
    return index_.size();
}