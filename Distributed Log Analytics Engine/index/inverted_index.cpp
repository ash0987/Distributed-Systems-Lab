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

    if (static_cast<size_t>(record_id + 1) > total_records_)
    {
        total_records_ = record_id + 1;
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

size_t InvertedIndex::term_count() const
{
    return index_.size();
}

size_t InvertedIndex::total_records() const
{
    return total_records_;
}

const std::map<std::string, std::vector<int>>&
InvertedIndex::data() const
{
    return index_;
}