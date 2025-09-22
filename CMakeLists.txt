cmake_minimum_required(VERSION 3.16)



project(BayerClash LANGUAGES C)

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


add_executable(tetratest tetratest.c useful.c thomson.c comparator.c tetrapal/tetrapal.c exoquant/exoquant.c)
target_include_directories(tetratest PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb ${PROJECT_SOURCE_DIR}/stb
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/exoquant 
    ${PROJECT_SOURCE_DIR}/Containers/src/include)
target_link_libraries(tetratest container)

add_executable(bayermo5 bayermo5.c useful.c thomson.c comparator.c)
target_include_directories(bayermo5 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/Containers/src/include)
target_link_libraries(bayermo5 container)

add_executable(bayermo6 bayermo6.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c exoquant/exoquant.c)
target_include_directories(bayermo6 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/exoquant
    ${PROJECT_SOURCE_DIR}/Containers/src/include)
target_link_libraries(bayermo6 container)



if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_link_libraries(tetratest)
else()
target_link_libraries(tetratest m)
endif()
