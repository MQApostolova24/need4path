#include "core/auth.h"

#include "core/utils.h"
#include "core/validation.h"

#include <cstdint>
#include <cstdio>
#include <fstream>

namespace
{
const std::uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
const std::uint64_t FNV_PRIME = 1099511628211ULL;
const int HASH_ROUNDS = 2000;

// FNV-1a hash. Good enough for a school project, NOT for real security.
std::uint64_t fnv1a(const std::string& data, std::uint64_t hash)
{
    for (unsigned char symbol : data)
    {
        hash ^= symbol;
        hash *= FNV_PRIME;
    }
    return hash;
}

std::string toHex(std::uint64_t value)
{
    char buffer[17];
    std::snprintf(buffer, sizeof(buffer), "%016llx", static_cast<unsigned long long>(value));
    return buffer;
}

bool appendUser(const std::string& filePath, const UserAccount& account)
{
    if (!ensureParentDirectory(filePath))
    {
        return false;
    }

    std::ofstream file(filePath, std::ios::app);
    if (!file.is_open())
    {
        return false;
    }

    file << account.username << ';' << account.salt << ';' << account.passwordHash << '\n';
    return file.good();
}
}  // namespace

std::string hashPassword(const std::string& password, const std::string& salt)
{
    std::uint64_t hash = FNV_OFFSET_BASIS;
    for (int round = 0; round < HASH_ROUNDS; ++round)
    {
        hash = fnv1a(salt + password + std::to_string(round), hash);
    }
    return toHex(hash);
}

std::string generateSalt(std::mt19937& rng)
{
    const std::uint64_t high = static_cast<std::uint64_t>(rng());
    const std::uint64_t low = static_cast<std::uint64_t>(rng());
    return toHex((high << 32) | low);
}

bool loadUsers(const std::string& filePath, std::vector<UserAccount>& users)
{
    users.clear();

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return true;  // no file yet = no registered users
    }

    std::string line;
    while (std::getline(file, line))
    {
        const std::vector<std::string> parts = splitString(line, ';');
        if (parts.size() != 3 || parts[0].empty() || parts[2].empty())
        {
            continue;  // skip broken lines
        }
        users.push_back(UserAccount{parts[0], parts[1], parts[2]});
    }
    return true;
}

bool findUser(const std::vector<UserAccount>& users, const std::string& username, UserAccount& found)
{
    const std::string wanted = toLowerCopy(username);
    for (const UserAccount& user : users)
    {
        if (toLowerCopy(user.username) == wanted)
        {
            found = user;
            return true;
        }
    }
    return false;
}

AuthResponse registerUser(const std::string& filePath, const std::string& username,
                          const std::string& password, const std::string& confirmation,
                          std::mt19937& rng)
{
    AuthResponse response;

    response.message = validateUsername(username);
    if (!response.message.empty())
    {
        return response;
    }

    response.message = validatePassword(password);
    if (!response.message.empty())
    {
        return response;
    }

    if (password != confirmation)
    {
        response.message = "The passwords do not match.";
        return response;
    }

    std::vector<UserAccount> users;
    UserAccount existing;
    if (!loadUsers(filePath, users))
    {
        response.message = "Could not read the accounts file.";
        return response;
    }
    if (findUser(users, username, existing))
    {
        response.message = "This explorer name is already taken.";
        return response;
    }

    UserAccount account;
    account.username = username;
    account.salt = generateSalt(rng);
    account.passwordHash = hashPassword(password, account.salt);

    if (!appendUser(filePath, account))
    {
        response.message = "Could not save the account.";
        return response;
    }

    response.success = true;
    response.username = username;
    response.message = "Account created! Log in to begin your expedition.";
    return response;
}

AuthResponse loginUser(const std::string& filePath, const std::string& username,
                       const std::string& password)
{
    AuthResponse response;

    if (username.empty())
    {
        response.message = "Please enter your username.";
        return response;
    }
    if (password.empty())
    {
        response.message = "Please enter your password.";
        return response;
    }

    std::vector<UserAccount> users;
    UserAccount account;
    loadUsers(filePath, users);

    // The same message for both cases, so nobody can guess which names exist.
    if (!findUser(users, username, account) || hashPassword(password, account.salt) != account.passwordHash)
    {
        response.message = "Wrong username or password.";
        return response;
    }

    response.success = true;
    response.username = account.username;
    response.message = "Welcome back, " + account.username + "!";
    return response;
}
