cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_COMPILER gcc-14)
set(CMAKE_CXX_COMPILER g++-14)

# Include directories and library paths
set(PROJECT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib/libsrtp/include /opt/homebrew/Cellar/openssl@3/3.3.1/include)
set(OPENSSL_LIB_DIR /opt/homebrew/Cellar/openssl@3/3.3.1/lib)
set(PROJECT_LINK_LIB ${OPENSSL_LIB_DIR}/libcrypto.a ${OPENSSL_LIB_DIR}/libssl.a ${CMAKE_SOURCE_DIR}/lib/libsrtp/libsrtp3.a)

# Define port number
add_definitions(-DPORT=5000)

# Output directory
set(OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${OUTPUT_DIRECTORY})

# Project definitions
project(transmitter_receiver VERSION 0.1.0 LANGUAGES C CXX)

# Function to set up targets
function(add_transceiver_executable target)
    add_executable(${target} src/${target}.cpp)
    target_include_directories(${target} PRIVATE ${PROJECT_INCLUDE_DIR})
    target_link_libraries(${target} PRIVATE ${PROJECT_LINK_LIB})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "${target} has been built and placed in ${OUTPUT_DIRECTORY}"
        COMMENT "Custom Command: Notifying ${target} build"
    )
endfunction()

# Transmitter and Receiver
add_transceiver_executable(transmitter)
add_transceiver_executable(receiver)

# Ensure custom commands are run
add_custom_target(copy_binaries ALL
    DEPENDS transmitter receiver
    COMMAND ${CMAKE_COMMAND} -E echo "Ensuring custom commands are executed"
)

# Print output directory for verification
message(STATUS "Output directory: ${OUTPUT_DIRECTORY}")