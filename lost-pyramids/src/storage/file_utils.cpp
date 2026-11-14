#include "storage/file_utils.h"

#include <filesystem>
#include <fstream>

bool readLines(const std::string& filePath, std::vector<std::string>& lines)
{
    lines.clear();
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    return true;
}

bool ensureParentDirectory(const std::string& filePath)
{
    const std::filesystem::path parent = std::filesystem::path(filePath).parent_path();
    if (parent.empty())
    {
        return true;
    }

    std::error_code error;
    std::filesystem::create_directories(parent, error);
    return !error;
}
