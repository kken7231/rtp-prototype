# Specify the minimum required version of CMake
cmake_minimum_required(VERSION 3.25)

# Set the C and C++ compilers
set(CMAKE_C_COMPILER gcc-14)
set(CMAKE_CXX_COMPILER g++-14)

# Define include directories and library paths
set(PROJECT_INCLUDE_DIR 
    ${CMAKE_SOURCE_DIR}/lib/libsrtp/include 
    /opt/homebrew/Cellar/openssl@3/3.3.1/include
)
set(OPENSSL_LIB_DIR /opt/homebrew/Cellar/openssl@3/3.3.1/lib)
set(PROJECT_LINK_LIB 
    ${OPENSSL_LIB_DIR}/libcrypto.a 
    ${OPENSSL_LIB_DIR}/libssl.a 
    ${CMAKE_SOURCE_DIR}/lib/libsrtp/libsrtp3.a
)

# Define a macro for the port number
add_definitions(-DPORT=5000)

# Set the output directory for the built binaries
set(OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${OUTPUT_DIRECTORY})

# Define the project name, version, and languages
project(transmitter_receiver VERSION 0.1.0 LANGUAGES C CXX)

# Add an executable target for the SRTP project
add_executable(srtp 
    src/util.h 
    src/util.cpp 
    src/srtp.cpp
)
target_include_directories(srtp PRIVATE ${PROJECT_INCLUDE_DIR})
target_link_libraries(srtp PRIVATE ${PROJECT_LINK_LIB})
set_target_properties(srtp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

# Add a custom command to run after building the SRTP target
add_custom_command(TARGET srtp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "srtp has been built and placed in ${OUTPUT_DIRECTORY}"
    COMMENT "Custom Command: Notifying srtp build"
)

# Ensure custom commands are executed by adding a custom target
add_custom_target(copy_binaries ALL
    DEPENDS srtp
    COMMAND ${CMAKE_COMMAND} -E echo "Ensuring custom commands are executed"
)

# Print the output directory for verification
message(STATUS "Output directory: ${OUTPUT_DIRECTORY}")