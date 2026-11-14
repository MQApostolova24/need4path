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

const int LOCATION_COUNT = 3;    // Egypt, Mexico, Cambodia
const int DIFFICULTY_COUNT = 3;  // Easy, Medium, Hard
const int RELIC_TYPES = LOCATION_COUNT * DIFFICULTY_COUNT;
const int SKIN_COUNT = 6;

const int LEADERBOARD_SIZE = 10;

// Points for every reward (before the difficulty bonus).
const int POINTS_PER_ROUTE_TILE = 10;
const int POINTS_PER_TREASURE = 50;
const int POINTS_PER_GEM = 150;
const int POINTS_FOR_RELIC = 500;
const int POINTS_PER_LIFE = 100;
const int POINTS_PER_SECOND_LEFT = 5;

// Seconds added to the location timer by a time pickup (hourglass, sun idol, rain charm).
const float TIME_PICKUP_SECONDS = 15.0f;

// Relics are rare: every expedition has a 1 in 5 chance to hide one, and after
// 3 expeditions in a row without a relic the next one always has one.
// On average that is one relic about every 3 expeditions.
const int RELIC_CHANCE_ONE_IN = 5;
const int RELIC_GUARANTEED_AFTER = 3;

// Coins (the shop currency) - earned only when the player escapes.
const int COINS_PER_TREASURE = 2;
const int COINS_PER_GEM = 8;

const char* const USERS_FILE_PATH = "data/users.txt";
const char* const LEADERBOARD_FILE_PATH = "data/leaderboard.txt";
const char* const PROFILES_FILE_PATH = "data/profiles.txt";
