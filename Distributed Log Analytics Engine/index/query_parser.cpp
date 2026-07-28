#include "query_parser.h"

#include <cctype>
#include <stdexcept>

QueryParser::QueryParser(
    const std::string& query
)
{
    tokenize(query);
}


std::unique_ptr<QueryNode> QueryParser::parse()
{
    auto result = parse_or();
    if (has_more())
    {
        throw std::runtime_error("Unexpected trailing tokens in query: " + peek());
    }
    return result;
}

//
// OR
//

std::unique_ptr<QueryNode>
QueryParser::parse_or()
{
    auto node =
        parse_and();

    while (
        has_more() &&
        peek() == "OR"
    )
    {
        consume();

        auto rhs =
            parse_and();

        node =
            std::make_unique<OrNode>(
                std::move(node),
                std::move(rhs)
            );
    }

    return node;
}

//
// AND
//

std::unique_ptr<QueryNode>
QueryParser::parse_and()
{
    auto node =
        parse_not();

    while (
        has_more() &&
        peek() == "AND"
    )
    {
        consume();

        auto rhs =
            parse_not();

        node =
            std::make_unique<AndNode>(
                std::move(node),
                std::move(rhs)
            );
    }

    return node;
}

//
// NOT
//

std::unique_ptr<QueryNode>
QueryParser::parse_not()
{
    if (
        has_more() &&
        peek() == "NOT"
    )
    {
        consume();

        return
            std::make_unique<NotNode>(
                parse_not()
            );
    }

    return parse_primary();
}

//
// Primary
//

std::unique_ptr<QueryNode>
QueryParser::parse_primary()
{
    if (!has_more())
    {
        throw std::runtime_error(
            "Unexpected end of query."
        );
    }

    if (peek() == "(")
    {
        consume();

        auto node =
            parse_or();

        if (
            !has_more() ||
            consume() != ")"
        )
        {
            throw std::runtime_error(
                "Expected ')'"
            );
        }

        return node;
    }

    std::string token =
        consume();

    //
    // Field match?
    //

    size_t pos =
        token.find(':');

    if (pos != std::string::npos)
    {
        return std::make_unique<FieldMatchNode>(
            token.substr(0, pos),
            token.substr(pos + 1)
        );
    }

    return std::make_unique<TermNode>(
        token
    );
}

//
// Helpers
//

bool QueryParser::match(
    const std::string& token
)
{
    if (
        has_more() &&
        peek() == token
    )
    {
        current_++;

        return true;
    }

    return false;
}

bool QueryParser::has_more() const
{
    return current_ < tokens_.size();
}

const std::string&
QueryParser::peek() const
{
    return tokens_[current_];
}

std::string
QueryParser::consume()
{
    return tokens_[current_++];
}

//
// Very simple tokenizer
//

void QueryParser::tokenize(
    const std::string& query
)
{
    std::string current;

    for (char ch : query)
    {
        if (std::isspace(
                static_cast<unsigned char>(ch)
            ))
        {
            if (!current.empty())
            {
                tokens_.push_back(current);
                current.clear();
            }

            continue;
        }

        if (ch == '(' || ch == ')')
        {
            if (!current.empty())
            {
                tokens_.push_back(current);
                current.clear();
            }

            tokens_.push_back(
                std::string(1, ch)
            );

            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty())
    {
        tokens_.push_back(current);
    }
}