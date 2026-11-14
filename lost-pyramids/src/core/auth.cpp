#include "core/auth.h"

#include "core/utils.h"

#include <cstdint>
#include <cstdio>

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
