/*
* Author   : Harry Yu
* Email    : harryyunull@gmail.com
* Link     : https://github.com/harrynull
*/
#include "logger.h"

std::array<const char*, 6> Logger::levelTags{
    "[verbose]", "[debug]", "[info]", "[warning]", "[error]", "[fatal]"
};

std::array<Colors::ColorFunc*, 6> Logger::colors{
    Colors::white, Colors::white, Colors::lwhite, Colors::yellow, Colors::lred, Colors::red
};
