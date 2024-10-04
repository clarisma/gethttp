cmake_minimum_required(VERSION 3.14)
project(gethttp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

# Set the CMAKE_CXX_FLAGS_RELWITHDEBINFO variable to remove the NDEBUG definition
# This approach ensures that assertions are enabled in RelWithDebInfo configuration
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -U NDEBUG")

set(BUILD_SHARED_LIBS OFF)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

# Create the executable target
add_executable(gethttp ${SOURCE_FILES})

target_include_directories(gethttp PRIVATE src)
target_link_libraries(gethttp PRIVATE winhttp)

add_custom_command(TARGET gethttp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:gethttp>
        "C:/geodesk/tests/gethttp.exe"
)
