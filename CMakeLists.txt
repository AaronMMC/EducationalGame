cmake_minimum_required(VERSION 3.20)
project(SynType)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS OFF)

# ── CLion: copy assets next to the executable automatically ──────────────────
set(ASSETS_SRC "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_DST "${CMAKE_BINARY_DIR}/assets")

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

add_executable(SynType
    src/main.cpp
    src/Stats.cpp
    src/Avatar.cpp
    src/TextSnippets.cpp
    src/UIHelpers.cpp
    src/NetworkSim.cpp
    src/GameState.cpp
    src/Screens/TitleScreen.cpp
    src/Screens/AvatarSelectScreen.cpp
    src/Screens/VSScreen.cpp
    src/Screens/GameScreen.cpp
    src/Screens/ResultScreen.cpp
    src/Screens/ProfileScreen.cpp
    src/Screens/SettingsScreen.cpp
    src/Screens/SPSelectScreen.cpp
)

target_include_directories(SynType PRIVATE src)
target_link_libraries(SynType PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)

# ── Auto-copy assets folder into the build directory so CLion "Run" works ────
add_custom_command(TARGET SynType POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_SRC}"
        "$<TARGET_FILE_DIR:SynType>/assets"
    COMMENT "Copying assets to build output directory"
)

# ── Windows: copy SFML DLLs next to exe (only needed for shared builds) ──────
if(WIN32)
    add_custom_command(TARGET SynType POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:SynType>
            $<TARGET_FILE_DIR:SynType>
        COMMAND_EXPAND_LISTS
    )
endif()
