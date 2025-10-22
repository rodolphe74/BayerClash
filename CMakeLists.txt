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

# libimagequant quantizer
file(GLOB LIBIMAGEQUANT_SOURCES "${PROJECT_SOURCE_DIR}/libimagequant/*.c")
add_library(imquant STATIC
    ${LIBIMAGEQUANT_SOURCES}
)
target_compile_options(imquant PRIVATE -Wno-attributes)


add_executable(bayermo bayermo.c thomson.c color.c gx2reduce.c image.c k7.c tetrapal/tetrapal.c comparator.c)
target_include_directories(bayermo PRIVATE 
    ${PROJECT_SOURCE_DIR}/stb 
    ${PROJECT_SOURCE_DIR}/tetrapal
    ${PROJECT_SOURCE_DIR}/libimagequant
    ${PROJECT_SOURCE_DIR}/Containers/src/include
    ${PROJECT_SOURCE_DIR}/SimpleBlueNoise)

    # Lien avec les bibliothèques principales
target_link_libraries(bayermo PRIVATE container imquant)

# Ajout conditionnel de libm sous Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(bayermo PRIVATE m)
endif()