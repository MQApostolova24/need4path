#pragma once

#include <string>
#include <vector>

// Small helpers for the text files in data/.

// Reads all lines of a file. Returns false (and no lines) if the file does not exist yet.
bool readLines(const std::string& filePath, std::vector<std::string>& lines);

// Creates the folder that will contain filePath (e.g. "data/") if it is missing.
bool ensureParentDirectory(const std::string& filePath);
