#pragma once

#include "core/auth.h"

#include <random>
#include <string>
#include <vector>

// Player accounts in data/users.txt, one line per player: "username;salt;passwordHash".

struct AuthResponse
{
    bool success = false;
    std::string message;   // shown to the player
    std::string username;  // the stored spelling of the name on success
};

// Reads all accounts. A missing file simply means there are no accounts yet.
std::vector<UserAccount> loadUsers(const std::string& filePath);

// Validates the input, checks the name is free and adds the account to the file.
AuthResponse registerUser(const std::string& filePath, const std::string& username, const std::string& password,
                          const std::string& confirmation, std::mt19937& rng);

// Checks the name and password against the file.
AuthResponse loginUser(const std::string& filePath, const std::string& username, const std::string& password);
