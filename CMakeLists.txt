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

# wu quantizer old style C lib
add_library(wu STATIC
    ${PROJECT_SOURCE_DIR}/cq.c
)
target_compile_options(wu PRIVATE -Wno-old-style-definition)


# kmean quantizer
file(GLOB LIBIMAGEQUANT_SOURCES "${PROJECT_SOURCE_DIR}/libimagequant/*.c")
add_library(imquant STATIC
    ${LIBIMAGEQUANT_SOURCES}
)
target_compile_options(imquant PRIVATE -Wno-attributes)

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

add_executable(bayermo52 bayermo52.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c)
target_include_directories(bayermo52 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/Containers/src/include
    ${PROJECT_SOURCE_DIR}/SimpleBlueNoise)
target_link_libraries(bayermo52 container)

add_executable(bayermo53 bayermo53.c useful.c thomson.c comparator.c)
target_include_directories(bayermo53 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/Containers/src/include
    ${PROJECT_SOURCE_DIR}/SimpleBlueNoise)
target_link_libraries(bayermo53 container)

add_executable(bayermo54 bayermo54.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c)
target_include_directories(bayermo54 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/Containers/src/include
    ${PROJECT_SOURCE_DIR}/SimpleBlueNoise)
target_link_libraries(bayermo54 container)

add_executable(bayermo55 bayermo55.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c)
target_include_directories(bayermo55 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/Containers/src/include)
target_link_libraries(bayermo55 container)


add_executable(bayermo6 bayermo6.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c exoquant/exoquant.c)
target_include_directories(bayermo6 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/exoquant
    ${PROJECT_SOURCE_DIR}/Containers/src/include)
target_link_libraries(bayermo6 container)

add_executable(bayermo62 bayermo62.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c exoquant/exoquant.c)
target_include_directories(bayermo62 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/exoquant
    ${PROJECT_SOURCE_DIR}/Containers/src/include
    ${PROJECT_SOURCE_DIR}/libimagequant)
target_link_libraries(bayermo62 container imquant)

add_executable(bayermo63 bayermo63.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c)
target_include_directories(bayermo63 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/Containers/src/include
    ${PROJECT_SOURCE_DIR}/libimagequant
    ${PROJECT_SOURCE_DIR}/SimpleBlueNoise)
target_link_libraries(bayermo63 container imquant)

add_executable(bayermo635 bayermo635.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c)
target_include_directories(bayermo635 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/libimagequant
    ${PROJECT_SOURCE_DIR}/Containers/src/include
    ${PROJECT_SOURCE_DIR}/SimpleBlueNoise)
target_link_libraries(bayermo635 container imquant)

add_executable(bayermo64 bayermo64.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c exoquant/exoquant.c)
target_include_directories(bayermo64 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/exoquant
    ${PROJECT_SOURCE_DIR}/Containers/src/include
    ${PROJECT_SOURCE_DIR}/libimagequant)
target_link_libraries(bayermo64 container imquant)


add_executable(bayermo65 bayermo65.c useful.c thomson.c k7.c comparator.c tetrapal/tetrapal.c)
target_include_directories(bayermo65 PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/libimagequant
    ${PROJECT_SOURCE_DIR}/Containers/src/include)
target_link_libraries(bayermo65 container imquant)


if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_link_libraries(tetratest)
else()
target_link_libraries(tetratest m)
endif()
