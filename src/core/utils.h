#pragma once

#include <string>
#include <vector>

// Small helper functions shared by the game logic.

// Splits "a;b;c" into {"a", "b", "c"}. Carriage returns are ignored.
std::vector<std::string> splitString(const std::string& text, char separator);

// Returns a lower-case copy of the text (used for case-insensitive usernames).
std::string toLowerCopy(const std::string& text);

// Creates the folder that will contain filePath (e.g. "data/") if it is missing.
bool ensureParentDirectory(const std::string& filePath);

// Formats seconds as "mm:ss".
std::string formatTime(float seconds);
