#pragma once

#include <string>

// Validation of everything the player types in.
// Each function returns an empty string when the input is valid,
// otherwise a message that the UI can show.

std::string validateUsername(const std::string& username);
std::string validatePassword(const std::string& password);

// Parses the tomb size typed by the player. On success stores it in "size".
bool parseMazeSize(const std::string& text, int& size, std::string& error);
