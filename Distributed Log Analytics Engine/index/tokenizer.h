#pragma once

#include <string>
#include <vector>

#include "../core/log_record.h"

std::vector<std::string> tokenize(
    const LogRecord& record
);