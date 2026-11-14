cmake_minimum_required(VERSION 3.16)
project(LostPyramids LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- raylib ----------
# Uses an installed raylib 6.0 if one is found, otherwise downloads it from GitHub.
find_package(raylib 6.0 QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.0
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

# ---------- Game logic (no graphics, no files) ----------
add_library(lost_pyramids_core STATIC
    src/core/auth.cpp
    src/core/game.cpp
    src/core/leaderboard.cpp
    src/core/location.cpp
    src/core/maze.cpp
    src/core/maze_generator.cpp
    src/core/profile.cpp
    src/core/skins.cpp
    src/core/utils.cpp
    src/core/validation.cpp
)
target_include_directories(lost_pyramids_core PUBLIC include)
target_compile_options(lost_pyramids_core PRIVATE ${PROJECT_WARNINGS})

# ---------- Saving and loading the data files ----------
add_library(lost_pyramids_storage STATIC
    src/storage/account_storage.cpp
    src/storage/file_utils.cpp
    src/storage/profile_storage.cpp
    src/storage/score_storage.cpp
)
target_link_libraries(lost_pyramids_storage PUBLIC lost_pyramids_core)
target_compile_options(lost_pyramids_storage PRIVATE ${PROJECT_WARNINGS})

# ---------- The game ----------
add_executable(lost_pyramids
    src/main.cpp
    src/ui/app.cpp
    src/ui/draw_utils.cpp
    src/ui/fonts.cpp
    src/ui/location_theme.cpp
    src/ui/maze_renderer.cpp
    src/ui/screens_auth.cpp
    src/ui/screens_expedition.cpp
    src/ui/screens_game.cpp
    src/ui/screens_menu.cpp
    src/ui/screens_trading.cpp
    src/ui/sprites.cpp
    src/ui/widgets.cpp
)
target_link_libraries(lost_pyramids PRIVATE lost_pyramids_storage raylib)
target_compile_options(lost_pyramids PRIVATE ${PROJECT_WARNINGS})

# Copy the fonts next to the program, so it finds them from any folder.
add_custom_command(TARGET lost_pyramids POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:lost_pyramids>/assets"
)

# Visual Studio solutions: run from the project folder so data/ is created next to the sources.
set_target_properties(lost_pyramids PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# ---------- QA tests ----------
add_executable(lost_pyramids_tests tests/test_main.cpp)
target_link_libraries(lost_pyramids_tests PRIVATE lost_pyramids_storage)
target_compile_options(lost_pyramids_tests PRIVATE ${PROJECT_WARNINGS})

enable_testing()
add_test(NAME lost_pyramids_tests COMMAND lost_pyramids_tests)
