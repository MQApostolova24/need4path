#pragma once

#include <random>
#include <string>
#include <vector>

// Player accounts. Passwords are never stored - only a salted hash.
// Reading and writing the accounts file is done in storage/account_storage.

struct UserAccount
{
    std::string username;
    std::string salt;
    std::string passwordHash;
};

std::string hashPassword(const std::string& password, const std::string& salt);
std::string generateSalt(std::mt19937& rng);

// Case-insensitive lookup by username.
bool findUser(const std::vector<UserAccount>& users, const std::string& username, UserAccount& found);
