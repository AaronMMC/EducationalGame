cmake_minimum_required(VERSION 3.20)
project(SynType)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS OFF)

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
