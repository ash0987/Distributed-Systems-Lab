#include "query_ast.h"

#include "query_executor.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <set>
#include <stdexcept>
#include <utility>

namespace
{

    std::string to_lower(
        const std::string &str)
    {
        std::string result;
        result.reserve(str.size());

        for (char ch : str)
        {
            result.push_back(
                static_cast<char>(
                    std::tolower(
                        static_cast<unsigned char>(ch))));
        }

        return result;
    }

} // namespace

//
// FieldMatchNode
//

FieldMatchNode::FieldMatchNode(
    const std::string &field,
    const std::string &value)
    : field_(field),
      value_(value)
{
}

std::vector<int> FieldMatchNode::evaluate(
    const QueryExecutor &executor) const
{
    std::string key =
        to_lower(field_) +
        ":" +
        to_lower(value_);

    const auto &postings =
        executor.index().lookup(key);

    return std::vector<int>(
        postings.begin(),
        postings.end());
}

std::string FieldMatchNode::to_string() const
{
    return field_ + ":" + value_;
}

//
// TermNode
//

TermNode::TermNode(
    const std::string &term)
    : term_(to_lower(term))
{
}

std::string TermNode::to_string() const
{
    return term_;
}

std::vector<int> TermNode::evaluate(
    const QueryExecutor &executor) const
{
    const auto &postings =
        executor.index().lookup(term_);

    return std::vector<int>(
        postings.begin(),
        postings.end());
}

//
// AndNode
//

AndNode::AndNode(
    std::unique_ptr<QueryNode> left,
    std::unique_ptr<QueryNode> right)
    : left_(std::move(left)),
      right_(std::move(right))
{
}

std::string AndNode::to_string() const
{
    return "(" +
           left_->to_string() +
           " AND " +
           right_->to_string() +
           ")";
}

std::vector<int> AndNode::evaluate(
    const QueryExecutor &executor) const
{
    auto left =
        left_->evaluate(executor);

    auto right =
        right_->evaluate(executor);

    std::vector<int> result;

    std::set_intersection(
        left.begin(),
        left.end(),
        right.begin(),
        right.end(),
        std::back_inserter(result));

    return result;
}

//
// OrNode
//

OrNode::OrNode(
    std::unique_ptr<QueryNode> left,
    std::unique_ptr<QueryNode> right)
    : left_(std::move(left)),
      right_(std::move(right))
{
}

std::string OrNode::to_string() const
{
    return "(" +
           left_->to_string() +
           " OR " +
           right_->to_string() +
           ")";
}

std::vector<int> OrNode::evaluate(
    const QueryExecutor &executor) const
{
    auto left =
        left_->evaluate(executor);

    auto right =
        right_->evaluate(executor);

    std::vector<int> result;

    std::set_union(
        left.begin(),
        left.end(),
        right.begin(),
        right.end(),
        std::back_inserter(result));

    return result;
}

//
// NotNode
//

NotNode::NotNode(
    std::unique_ptr<QueryNode> child)
    : child_(std::move(child))
{
}

std::string NotNode::to_string() const
{
    return "(NOT " +
           child_->to_string() +
           ")";
}

std::vector<int> NotNode::evaluate(
    const QueryExecutor &executor) const
{
    auto child =
        child_->evaluate(executor);

    std::vector<int> universe;

    for (int i = 0;
         i < static_cast<int>(
                 executor.index().total_records());
         i++)
    {
        universe.push_back(i);
    }

    std::vector<int> result;

    std::set_difference(
        universe.begin(),
        universe.end(),
        child.begin(),
        child.end(),
        std::back_inserter(result));

    return result;
}

//
// RegexNode
//

RegexNode::RegexNode(
    const std::string &field,
    const std::string &pattern)
    : field_(field),
      pattern_(pattern)
{
}

std::string RegexNode::to_string() const
{
    return "regex(" +
           field_ +
           ",/" +
           pattern_ +
           "/)";
}
std::vector<int> RegexNode::evaluate(
    const QueryExecutor& executor
) const
{
    if (to_lower(field_) != "message")
    {
        throw std::invalid_argument(
            "Regex is only supported on the message field."
        );
    }

    std::vector<int> result;

    std::regex pattern(
        pattern_,
        std::regex_constants::icase
    );

    int total =
        static_cast<int>(
            executor.index().total_records()
        );

    for (int id = 0; id < total; id++)
    {
        std::string message =
            executor.get_message(id);

        if (std::regex_search(
                message,
                pattern))
        {
            result.push_back(id);
        }
    }

    return result;
}