#pragma once

#include <memory>
#include <string>
#include <vector>

class QueryExecutor;

// Base class for all query AST nodes.
class QueryNode
{
public:
    virtual ~QueryNode() = default;

    // Returns a human-readable representation of the query tree.
    virtual std::string to_string() const = 0;

    // Executes this node against the inverted index.
    virtual std::vector<int> evaluate(
        const QueryExecutor &executor) const = 0;
};

//
// service:auth
// level:error
//
class FieldMatchNode : public QueryNode
{
public:
    FieldMatchNode(
        const std::string &field,
        const std::string &value);

    std::string to_string() const override;

    std::vector<int> evaluate(
        const QueryExecutor &executor) const override;

private:
    std::string field_;
    std::string value_;
};

//
// timeout
// disk
// failure
//
class TermNode : public QueryNode
{
public:
    explicit TermNode(
        const std::string &term);

    std::string to_string() const override;

    std::vector<int> evaluate(
        const QueryExecutor &executor) const override;

private:
    std::string term_;
};

//
// left AND right
//
class AndNode : public QueryNode
{
public:
    AndNode(
        std::unique_ptr<QueryNode> left,
        std::unique_ptr<QueryNode> right);

    std::string to_string() const override;

    std::vector<int> evaluate(
        const QueryExecutor &executor) const override;

private:
    std::unique_ptr<QueryNode> left_;
    std::unique_ptr<QueryNode> right_;
};

//
// left OR right
//
class OrNode : public QueryNode
{
public:
    OrNode(
        std::unique_ptr<QueryNode> left,
        std::unique_ptr<QueryNode> right);

    std::string to_string() const override;

    std::vector<int> evaluate(
        const QueryExecutor &executor) const override;

private:
    std::unique_ptr<QueryNode> left_;
    std::unique_ptr<QueryNode> right_;
};

//
// NOT child
//
class NotNode : public QueryNode
{
public:
    explicit NotNode(
        std::unique_ptr<QueryNode> child);

    std::string to_string() const override;

    std::vector<int> evaluate(
        const QueryExecutor &executor) const override;

private:
    std::unique_ptr<QueryNode> child_;
};

//
// regex(message,/timeout.*/)
//
class RegexNode : public QueryNode
{
public:
    RegexNode(
        const std::string &field,
        const std::string &pattern);

    std::string to_string() const override;

    std::vector<int> evaluate(
        const QueryExecutor &executor) const override;

private:
    std::string field_;
    std::string pattern_;
};