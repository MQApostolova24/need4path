cmake_minimum_required(VERSION 3.16)
project(LostPyramids LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# All the project files (code, assets, docs, tests) are in the lost-pyramids folder.
set(GAME_DIR "${CMAKE_SOURCE_DIR}/lost-pyramids")

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
    ${GAME_DIR}/src/core/auth.cpp
    ${GAME_DIR}/src/core/game.cpp
    ${GAME_DIR}/src/core/leaderboard.cpp
    ${GAME_DIR}/src/core/location.cpp
    ${GAME_DIR}/src/core/maze.cpp
    ${GAME_DIR}/src/core/maze_generator.cpp
    ${GAME_DIR}/src/core/profile.cpp
    ${GAME_DIR}/src/core/skins.cpp
    ${GAME_DIR}/src/core/utils.cpp
    ${GAME_DIR}/src/core/validation.cpp
)
target_include_directories(lost_pyramids_core PUBLIC ${GAME_DIR}/include)
target_compile_options(lost_pyramids_core PRIVATE ${PROJECT_WARNINGS})

# ---------- Saving and loading the data files ----------
add_library(lost_pyramids_storage STATIC
    ${GAME_DIR}/src/storage/account_storage.cpp
    ${GAME_DIR}/src/storage/file_utils.cpp
    ${GAME_DIR}/src/storage/profile_storage.cpp
    ${GAME_DIR}/src/storage/score_storage.cpp
)
target_link_libraries(lost_pyramids_storage PUBLIC lost_pyramids_core)
target_compile_options(lost_pyramids_storage PRIVATE ${PROJECT_WARNINGS})

# ---------- The game ----------
add_executable(lost_pyramids
    ${GAME_DIR}/src/main.cpp
    ${GAME_DIR}/src/ui/app.cpp
    ${GAME_DIR}/src/ui/draw_utils.cpp
    ${GAME_DIR}/src/ui/fonts.cpp
    ${GAME_DIR}/src/ui/location_theme.cpp
    ${GAME_DIR}/src/ui/maze_renderer.cpp
    ${GAME_DIR}/src/ui/screens_auth.cpp
    ${GAME_DIR}/src/ui/screens_expedition.cpp
    ${GAME_DIR}/src/ui/screens_game.cpp
    ${GAME_DIR}/src/ui/screens_menu.cpp
    ${GAME_DIR}/src/ui/screens_trading.cpp
    ${GAME_DIR}/src/ui/sprites.cpp
    ${GAME_DIR}/src/ui/widgets.cpp
)
target_link_libraries(lost_pyramids PRIVATE lost_pyramids_storage raylib)
target_compile_options(lost_pyramids PRIVATE ${PROJECT_WARNINGS})

# Copy the fonts next to the program, so it finds them from any folder.
add_custom_command(TARGET lost_pyramids POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${GAME_DIR}/assets" "$<TARGET_FILE_DIR:lost_pyramids>/assets"
)

# Visual Studio solutions: run from the lost-pyramids folder, so data/ is created there.
set_target_properties(lost_pyramids PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${GAME_DIR}")

# ---------- QA tests ----------
add_executable(lost_pyramids_tests ${GAME_DIR}/tests/test_main.cpp)
target_link_libraries(lost_pyramids_tests PRIVATE lost_pyramids_storage)
target_compile_options(lost_pyramids_tests PRIVATE ${PROJECT_WARNINGS})

enable_testing()
add_test(NAME lost_pyramids_tests COMMAND lost_pyramids_tests)
