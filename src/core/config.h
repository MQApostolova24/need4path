#pragma once

// Shared constants for the game logic.
// Nothing in the "core" folder uses graphics or console input/output (cin/cout).

const int MIN_MAZE_SIZE = 10;
const int MAX_MAZE_SIZE = 50;
const int DEFAULT_MAZE_SIZE = 15;

const int MIN_USERNAME_LENGTH = 3;
const int MAX_USERNAME_LENGTH = 16;
const int MIN_PASSWORD_LENGTH = 6;
const int MAX_PASSWORD_LENGTH = 24;

const int LEADERBOARD_SIZE = 10;

const char* const USERS_FILE_PATH = "data/users.txt";
const char* const LEADERBOARD_FILE_PATH = "data/leaderboard.txt";
