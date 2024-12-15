cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(PROJECT_NAME genai)
set(GENAI_LIB_NAME genai)

add_compile_definitions(CURL_STATICLIB=1)

add_library(${GENAI_LIB_NAME} STATIC
    src/genai/genai.cxx
    src/genai/chat.cxx
)

target_include_directories(${GENAI_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

target_link_directories(${GENAI_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/
)

target_link_libraries(${GENAI_LIB_NAME}
    libcurl.a
    z
)

target_compile_definitions(${GENAI_LIB_NAME} PRIVATE CURL_STATICLIB=1)
target_compile_options(${GENAI_LIB_NAME} PRIVATE -fPIC)

export(
    TARGETS ${GENAI_LIB_NAME}
    FILE genai-config.cmake
)