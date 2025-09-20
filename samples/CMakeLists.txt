cmake_minimum_required(VERSION 3.16)



project(Dither LANGUAGES C)

# c_container library
file(GLOB C_CONTAINER_SOURCES "${PROJECT_SOURCE_DIR}/Containers/src/*.c")
add_library(container STATIC
    ${C_CONTAINER_SOURCES}
)
target_include_directories(container PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_include_directories(container PUBLIC
    ${PROJECT_SOURCE_DIR}/containers/src/include
)


add_executable(dither dither.c useful.c exoquant/exoquant.c)
target_include_directories(dither PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb ${PROJECT_SOURCE_DIR}/stb
    ${PROJECT_SOURCE_DIR}/exoquant
    ${PROJECT_SOURCE_DIR}/Containers/src/include)
target_link_libraries(dither container)



if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_link_libraries(dither ws2_32)
else()
target_link_libraries(dither m)
endif()
