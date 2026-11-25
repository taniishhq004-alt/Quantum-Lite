cmake_minimum_required(VERSION 3.16)
project(QuantumLite VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core Quantum Simulation Library (OOP Engine)
add_library(quantum_core STATIC
    src/MatrixUtils.cpp
    src/Qubit.cpp
    src/Gates.cpp
    src/QuantumCircuit.cpp
    src/QuantumSimulator.cpp
)

target_include_directories(quantum_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (MSVC)
    target_compile_options(quantum_core PRIVATE /W4 /utf-8)
else()
    target_compile_options(quantum_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Crow C++ REST API + Web Server Executable
add_executable(quantum_lite_server
    src/ApiServer.cpp
    src/main.cpp
)

target_include_directories(quantum_lite_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/crow
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio
)

target_compile_definitions(quantum_lite_server PRIVATE
    ASIO_STANDALONE
)

target_link_libraries(quantum_lite_server PRIVATE quantum_core)

if (WIN32)
    target_compile_definitions(quantum_lite_server PRIVATE
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    target_link_libraries(quantum_lite_server PRIVATE ws2_32 mswsock)
    if (MINGW)
        # Static runtime linking for portability across Windows machines
        target_link_options(quantum_lite_server PRIVATE -static-libgcc -static-libstdc++ -static -lpthread)
    endif()
else()
    find_package(Threads REQUIRED)
    target_link_libraries(quantum_lite_server PRIVATE Threads::Threads)
endif()

# Unit Tests Executable
enable_testing()
add_executable(quantum_lite_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests/simulation_tests.cpp
)
target_link_libraries(quantum_lite_tests PRIVATE quantum_core)
if (WIN32 AND MINGW)
    target_link_options(quantum_lite_tests PRIVATE -static-libgcc -static-libstdc++ -static -lpthread)
endif()

add_test(NAME QuantumLiteSimulationTests COMMAND quantum_lite_tests)
