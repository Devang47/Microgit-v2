# CMakeLists.txt for MicroGit C++ version

# 1. Defines the minimum required CMake version
cmake_minimum_required(VERSION 3.10)

# 2. Sets project name and version
project(MicroGit VERSION 1.0)

# 3. Configures compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define which commands are available
add_compile_definitions(
  INIT_COMMAND_AVAILABLE=1
  # Enable all commands
  ADD_COMMAND_AVAILABLE=1
  SAVE_COMMAND_AVAILABLE=1
  LOG_COMMAND_AVAILABLE=1
  STATUS_COMMAND_AVAILABLE=1
  CHECKOUT_COMMAND_AVAILABLE=1
  REMOVE_COMMAND_AVAILABLE=1
)

# 4. Find external dependencies
find_package(OpenSSL REQUIRED)

# Add nlohmann_json dependency - using a newer version
include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3  # Updated version
  CONFIGURE_COMMAND ""  # Skip configuration to avoid CMake version issues
  BUILD_COMMAND ""      # Skip build
)
# Try to make available but handle failure
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  message(STATUS "Downloading nlohmann/json...")
  # Use TRY_COMPILE to avoid hard failures
  try_compile(JSON_FETCH_SUCCESS ${CMAKE_BINARY_DIR} 
              ${CMAKE_SOURCE_DIR}/cmake/check_json.cpp
              OUTPUT_VARIABLE JSON_FETCH_OUTPUT)
  if(NOT JSON_FETCH_SUCCESS)
    message(WARNING "Could not download nlohmann/json automatically. Using header-only version.")
    # Create include directory for the json header
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/_deps/json-src/include/nlohmann)
    # Download single header file version instead
    file(DOWNLOAD 
         https://github.com/nlohmann/json/releases/download/v3.12.0/json.hpp
         ${CMAKE_BINARY_DIR}/_deps/json-src/include/nlohmann/json.hpp
         STATUS DOWNLOAD_STATUS)
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(NOT STATUS_CODE EQUAL 0)
      message(FATAL_ERROR "Failed to download json.hpp. Error: ${DOWNLOAD_STATUS}")
    endif()
  endif()
endif()

# Set source files with all implemented commands
set(SOURCE_FILES
  main.cpp
  cmd/root.cpp
  cmd/init.cpp
  cmd/add.cpp
  cmd/save.cpp
  cmd/log.cpp
  cmd/status.cpp
  cmd/checkout.cpp
  cmd/remove.cpp
  utils/main.cpp
  utils/json.cpp
)

# 5. Specifies source files to compile
add_executable(microgit ${SOURCE_FILES})

# 6. Configures include directories - add fallback path for JSON header
target_include_directories(microgit PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}/_deps/json-src/include  # Add path to downloaded JSON header
)

# 7. Links external libraries
target_link_libraries(microgit PRIVATE
  ${OPENSSL_LIBRARIES}
)

# 8. Handles platform/compiler-specific settings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
  target_link_libraries(microgit PRIVATE stdc++fs)
endif()

# Output helpful information about the build
message(STATUS "Building MicroGit with C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
