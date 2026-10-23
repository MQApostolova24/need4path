#pragma once

#include <random>
#include <string>
#include <vector>

// Player accounts, stored in data/users.txt as "username;salt;passwordHash".
// Passwords are never written to the file - only a salted hash.

struct UserAccount
{
    std::string username;
    std::string salt;
    std::string passwordHash;
};

struct AuthResponse
{
    bool success = false;
    std::string message;   // shown to the player
    std::string username;  // the stored spelling of the name on success
};

std::string hashPassword(const std::string& password, const std::string& salt);
std::string generateSalt(std::mt19937& rng);

// Reads all accounts. A missing file simply means there are no accounts yet.
bool loadUsers(const std::string& filePath, std::vector<UserAccount>& users);

// Case-insensitive lookup by username.
bool findUser(const std::vector<UserAccount>& users, const std::string& username, UserAccount& found);

AuthResponse registerUser(const std::string& filePath, const std::string& username,
                          const std::string& password, const std::string& confirmation,
                          std::mt19937& rng);

AuthResponse loginUser(const std::string& filePath, const std::string& username,
                       const std::string& password);
