#include "core/validation.h"

#include "core/config.h"

#include <cctype>

std::string validateUsername(const std::string& username)
{
    const int length = static_cast<int>(username.size());
    if (length == 0)
    {
        return "Please enter a username.";
    }
    if (length < MIN_USERNAME_LENGTH || length > MAX_USERNAME_LENGTH)
    {
        return "Username must be " + std::to_string(MIN_USERNAME_LENGTH) + "-" +
               std::to_string(MAX_USERNAME_LENGTH) + " characters long.";
    }
    if (!std::isalpha(static_cast<unsigned char>(username[0])))
    {
        return "Username must start with a letter.";
    }
    for (char symbol : username)
    {
        if (!std::isalnum(static_cast<unsigned char>(symbol)) && symbol != '_')
        {
            return "Username may contain only letters, digits and _.";
        }
    }
    return "";
}

std::string validatePassword(const std::string& password)
{
    const int length = static_cast<int>(password.size());
    if (length == 0)
    {
        return "Please enter a password.";
    }
    if (length < MIN_PASSWORD_LENGTH || length > MAX_PASSWORD_LENGTH)
    {
        return "Password must be " + std::to_string(MIN_PASSWORD_LENGTH) + "-" +
               std::to_string(MAX_PASSWORD_LENGTH) + " characters long.";
    }

    bool hasLetter = false;
    bool hasDigit = false;
    for (char symbol : password)
    {
        const unsigned char character = static_cast<unsigned char>(symbol);
        if (std::isspace(character))
        {
            return "Password cannot contain spaces.";
        }
        hasLetter = hasLetter || std::isalpha(character);
        hasDigit = hasDigit || std::isdigit(character);
    }

    if (!hasLetter || !hasDigit)
    {
        return "Password needs at least one letter and one digit.";
    }
    return "";
}

bool parseMazeSize(const std::string& text, int& size, std::string& error)
{
    const std::string rangeMessage = "The size must be between " + std::to_string(MIN_MAZE_SIZE) +
                                     " and " + std::to_string(MAX_MAZE_SIZE) + ".";
    error.clear();

    if (text.empty())
    {
        error = "Please enter a tomb size.";
        return false;
    }
    for (char symbol : text)
    {
        if (!std::isdigit(static_cast<unsigned char>(symbol)))
        {
            error = "The size must be a whole number.";
            return false;
        }
    }
    if (text.size() > 3)
    {
        error = rangeMessage;
        return false;
    }

    const int value = std::stoi(text);
    if (value < MIN_MAZE_SIZE || value > MAX_MAZE_SIZE)
    {
        error = rangeMessage;
        return false;
    }

    size = value;
    return true;
}
