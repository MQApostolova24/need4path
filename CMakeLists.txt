cmake_minimum_required(VERSION 3.16)
project(LostPyramids LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- raylib ----------
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

# ---------- Game logic (no graphics) ----------
add_library(lost_pyramids_core STATIC
    lost-pyramids/src/core/auth.cpp
    lost-pyramids/src/core/game.cpp
    lost-pyramids/src/core/leaderboard.cpp
    lost-pyramids/src/core/location.cpp
    lost-pyramids/src/core/maze.cpp
    lost-pyramids/src/core/maze_generator.cpp
    lost-pyramids/src/core/profile.cpp
    lost-pyramids/src/core/skins.cpp
    lost-pyramids/src/core/utils.cpp
    lost-pyramids/src/core/validation.cpp
)
target_include_directories(lost_pyramids_core PUBLIC lost-pyramids/src)
target_compile_options(lost_pyramids_core PRIVATE ${PROJECT_WARNINGS})

# ---------- The game ----------
add_executable(lost_pyramids
    lost-pyramids/src/main.cpp
    lost-pyramids/src/ui/app.cpp
    lost-pyramids/src/ui/draw_utils.cpp
    lost-pyramids/src/ui/fonts.cpp
    lost-pyramids/src/ui/location_theme.cpp
    lost-pyramids/src/ui/maze_renderer.cpp
    lost-pyramids/src/ui/screens_auth.cpp
    lost-pyramids/src/ui/screens_expedition.cpp
    lost-pyramids/src/ui/screens_game.cpp
    lost-pyramids/src/ui/screens_menu.cpp
    lost-pyramids/src/ui/screens_trading.cpp
    lost-pyramids/src/ui/sprites.cpp
    lost-pyramids/src/ui/widgets.cpp
)
target_link_libraries(lost_pyramids PRIVATE lost_pyramids_core raylib)
target_compile_options(lost_pyramids PRIVATE ${PROJECT_WARNINGS})

# Copy the fonts next to the program
add_custom_command(TARGET lost_pyramids POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/lost-pyramids/assets" "$<TARGET_FILE_DIR:lost_pyramids>/assets"
)

set_target_properties(lost_pyramids PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lost-pyramids")

# ---------- QA tests ----------
add_executable(lost_pyramids_tests lost-pyramids/tests/test_main.cpp)
target_link_libraries(lost_pyramids_tests PRIVATE lost_pyramids_core)
target_compile_options(lost_pyramids_tests PRIVATE ${PROJECT_WARNINGS})

enable_testing()
add_test(NAME lost_pyramids_tests COMMAND lost_pyramids_tests)