#include "query_ast.h"

#include <utility>



//
// FieldMatchNode
//

FieldMatchNode::FieldMatchNode(
    const std::string& field,
    const std::string& value
)
    : field_(field),
      value_(value)
{
}

std::string FieldMatchNode::to_string() const
{
    return
        field_
        + ":"
        + value_;
}



//
// TermNode
//

TermNode::TermNode(
    const std::string& term
)
    : term_(term)
{
}

std::string TermNode::to_string() const
{
    return term_;
}



//
// AndNode
//

AndNode::AndNode(
    std::unique_ptr<QueryNode> left,
    std::unique_ptr<QueryNode> right
)
    : left_(std::move(left)),
      right_(std::move(right))
{
}

std::string AndNode::to_string() const
{
    return
        "("
        + left_->to_string()
        + " AND "
        + right_->to_string()
        + ")";
}



//
// OrNode
//

OrNode::OrNode(
    std::unique_ptr<QueryNode> left,
    std::unique_ptr<QueryNode> right
)
    : left_(std::move(left)),
      right_(std::move(right))
{
}

std::string OrNode::to_string() const
{
    return
        "("
        + left_->to_string()
        + " OR "
        + right_->to_string()
        + ")";
}



//
// NotNode
//

NotNode::NotNode(
    std::unique_ptr<QueryNode> child
)
    : child_(std::move(child))
{
}

std::string NotNode::to_string() const
{
    return
        "(NOT "
        + child_->to_string()
        + ")";
}



//
// RegexNode
//

RegexNode::RegexNode(
    const std::string& field,
    const std::string& pattern
)
    : field_(field),
      pattern_(pattern)
{
}

std::string RegexNode::to_string() const
{
    return
        "regex("
        + field_
        + ",/"
        + pattern_
        + "/)";
}