cmake_minimum_required(VERSION 3.16)
project(TombOfTheForgottenPharaoh LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- raylib ----------
# Uses an installed raylib 5.5 if one is found, otherwise downloads it from GitHub.
find_package(raylib 5.5 QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

# ---------- Warnings ----------
if (MSVC)
    set(PROJECT_WARNINGS /W4)
else()
    set(PROJECT_WARNINGS -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

# ---------- Game logic (no graphics) ----------
add_library(tomb_core STATIC
    src/core/auth.cpp
    src/core/game.cpp
    src/core/leaderboard.cpp
    src/core/maze.cpp
    src/core/maze_generator.cpp
    src/core/utils.cpp
    src/core/validation.cpp
)
target_include_directories(tomb_core PUBLIC src)
target_compile_options(tomb_core PRIVATE ${PROJECT_WARNINGS})

# ---------- The game ----------
add_executable(tomb
    src/main.cpp
    src/ui/app.cpp
    src/ui/maze_renderer.cpp
    src/ui/screens_auth.cpp
    src/ui/screens_game.cpp
    src/ui/screens_menu.cpp
    src/ui/widgets.cpp
)
target_link_libraries(tomb PRIVATE tomb_core raylib)
target_compile_options(tomb PRIVATE ${PROJECT_WARNINGS})

# Run from the project folder so data/ is created next to the sources.
set_target_properties(tomb PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# ---------- QA tests ----------
add_executable(tomb_tests tests/test_main.cpp)
target_link_libraries(tomb_tests PRIVATE tomb_core)
target_compile_options(tomb_tests PRIVATE ${PROJECT_WARNINGS})

enable_testing()
add_test(NAME tomb_tests COMMAND tomb_tests)
