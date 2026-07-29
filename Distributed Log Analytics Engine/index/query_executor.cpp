#include "query_executor.h"

#include "../ingest/segment_reader.h"

QueryExecutor::QueryExecutor(
    const InvertedIndex& index,
    const std::vector<RecordLocation>& locations
)
    : index_(index),
      locations_(locations)
{
}

std::vector<int> QueryExecutor::execute(
    const QueryNode& query
) const
{
    return query.evaluate(*this);
}

const InvertedIndex& QueryExecutor::index() const
{
    return index_;
}

std::string QueryExecutor::get_message(
    int record_id
) const
{
    if (record_id < 0 ||
        record_id >= static_cast<int>(locations_.size()))
    {
        return "";
    }

    const RecordLocation& location =
        locations_[record_id];

    std::vector<LogRecord> records =
        SegmentReader::read_segment(
            location.segment_file
        );

    if (location.position < 0 ||
        location.position >= static_cast<int>(records.size()))
    {
        return "";
    }

    return records[location.position].message;
}