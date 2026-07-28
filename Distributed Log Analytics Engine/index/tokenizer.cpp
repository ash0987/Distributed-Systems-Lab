#include "tokenizer.h"

#include <cctype>
#include <string>
#include <vector>

namespace
{

std::string to_lower(
    const std::string& str
)
{
    std::string result;
    result.reserve(str.size());

    for (char ch : str)
    {
        result.push_back(
            static_cast<char>(
                std::tolower(
                    static_cast<unsigned char>(ch)
                )
            )
        );
    }

    return result;
}

bool is_identifier_token(
    const std::string& token
)
{
    if (token.empty())
    {
        return false;
    }

    // Skip tokens that are entirely numeric.
    // Example:
    // "12345"
    // "987654321"
    for (char ch : token)
    {
        if (!std::isdigit(
                static_cast<unsigned char>(ch)
            ))
        {
            return false;
        }
    }

    return true;
}

} // namespace



std::vector<std::string> tokenize(
    const LogRecord& record
)
{
    std::vector<std::string> terms;

    std::string current_word;

    for (char ch : record.message)
    {
        if (std::isalnum(
                static_cast<unsigned char>(ch)
            ))
        {
            current_word.push_back(
                static_cast<char>(
                    std::tolower(
                        static_cast<unsigned char>(ch)
                    )
                )
            );
        }
        else
        {
            if (!current_word.empty())
            {
                if (!is_identifier_token(current_word))
                {
                    terms.push_back(current_word);
                }

                current_word.clear();
            }
        }
    }

    // Last word
    if (!current_word.empty())
    {
        if (!is_identifier_token(current_word))
        {
            terms.push_back(current_word);
        }
    }

    // Structured field terms

    terms.push_back(
        "service:" +
        to_lower(record.service)
    );

    terms.push_back(
        "level:" +
        to_lower(record.level)
    );

    return terms;
}