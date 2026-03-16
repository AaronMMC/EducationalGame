cmake_minimum_required(VERSION 3.20)
project(TypingGame)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

add_executable(TypingGame main.cpp)

target_link_libraries(TypingGame PRIVATE sfml-graphics sfml-window sfml-system)