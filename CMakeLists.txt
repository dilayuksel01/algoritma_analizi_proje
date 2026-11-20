cmake_minimum_required(VERSION 3.16)
project(plagiarism CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/O2 /W4)
else()
    add_compile_options(-O2 -Wall)
endif()

# Single main.cpp as requested for direct PDF inclusion
add_executable(plagiarism main.cpp)

# Automatically copy sample data to build output folder for direct execution
add_custom_command(TARGET plagiarism POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/data/sample_set/main_doc.txt
        ${CMAKE_CURRENT_BINARY_DIR}/main_doc.txt
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data/sample_set/documents
        ${CMAKE_CURRENT_BINARY_DIR}/documents
    COMMENT "Copying sample_set test data to binary directory for direct execution..."
)
