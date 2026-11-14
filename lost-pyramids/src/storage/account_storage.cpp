#include "storage/account_storage.h"

#include "core/utils.h"
#include "core/validation.h"
#include "storage/file_utils.h"

#include <fstream>

namespace
{
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

AuthResponse failure(const std::string& message)
{
    AuthResponse response;
    response.message = message;
    return response;
}
}  // namespace

std::vector<UserAccount> loadUsers(const std::string& filePath)
{
    std::vector<std::string> lines;
    std::vector<UserAccount> users;
    readLines(filePath, lines);

    for (const std::string& line : lines)
    {
        const std::vector<std::string> parts = splitString(line, ';');
        if (parts.size() == 3 && !parts[0].empty() && !parts[2].empty())
        {
            users.push_back(UserAccount{parts[0], parts[1], parts[2]});
        }
    }
    return users;
}

AuthResponse registerUser(const std::string& filePath, const std::string& username, const std::string& password,
                          const std::string& confirmation, std::mt19937& rng)
{
    std::string error = validateUsername(username);
    if (error.empty())
    {
        error = validatePassword(password);
    }
    if (!error.empty())
    {
        return failure(error);
    }
    if (password != confirmation)
    {
        return failure("The passwords do not match.");
    }

    UserAccount existing;
    if (findUser(loadUsers(filePath), username, existing))
    {
        return failure("This explorer name is already taken.");
    }

    UserAccount account;
    account.username = username;
    account.salt = generateSalt(rng);
    account.passwordHash = hashPassword(password, account.salt);
    if (!appendUser(filePath, account))
    {
        return failure("Could not save the account.");
    }

    AuthResponse response;
    response.success = true;
    response.username = username;
    response.message = "Account created! Log in to begin your expedition.";
    return response;
}

AuthResponse loginUser(const std::string& filePath, const std::string& username, const std::string& password)
{
    if (username.empty())
    {
        return failure("Please enter your username.");
    }
    if (password.empty())
    {
        return failure("Please enter your password.");
    }

    // The same message for both cases, so nobody can guess which names exist.
    UserAccount account;
    if (!findUser(loadUsers(filePath), username, account) || hashPassword(password, account.salt) != account.passwordHash)
    {
        return failure("Wrong username or password.");
    }

    AuthResponse response;
    response.success = true;
    response.username = account.username;
    response.message = "Welcome back, " + account.username + "!";
    return response;
}
