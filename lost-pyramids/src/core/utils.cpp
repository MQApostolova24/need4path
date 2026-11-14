#include "core/utils.h"

#include <cctype>
#include <cstdio>

std::vector<std::string> splitString(const std::string& text, char separator)
{
    std::vector<std::string> parts;
    std::string current;

    for (char symbol : text)
    {
        if (symbol == separator)
        {
            parts.push_back(current);
            current.clear();
        }
        else if (symbol != '\r')
        {
            current.push_back(symbol);
        }
    }

    parts.push_back(current);
    return parts;
}

std::string toLowerCopy(const std::string& text)
{
    std::string result = text;
    for (char& symbol : result)
    {
        symbol = static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
    }
    return result;
}

std::string formatTime(float seconds)
{
    if (seconds < 0.0f)
    {
        seconds = 0.0f;
    }

    const int totalSeconds = static_cast<int>(seconds);
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", totalSeconds / 60, totalSeconds % 60);
    return buffer;
}
