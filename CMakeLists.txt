cmake_minimum_required(VERSION 3.19)

project(Oblivious-Shuffle VERSION 0.1.0)

set(libOTe_DIR extern/libOTe)

find_package(libOTe HINTS ${libOTe_DIR} REQUIRED)

aux_source_directory(src DIR_SRC)
aux_source_directory(src/shuffle DIR_SHUFFLE)
aux_source_directory(src/khprf DIR_KHPRF)
aux_source_directory(src/utils DIR_UTILS)
include_directories(src extern src/shuffle src/khprf src/utils )

add_executable(
    shuffle_matrix
    src/examples/shuffle_matrix.cpp
    ${DIR_SRC}
    ${DIR_SHUFFLE}
    ${DIR_KHPRF}
    ${DIR_UTILS}
)
target_link_libraries(shuffle_matrix oc::libOTe)
target_compile_options(shuffle_matrix PUBLIC -march=native -std=c++20)
target_include_directories(shuffle_matrix PUBLIC ${libOTe_DIR})

add_executable(
    shuffle_vector
    src/examples/shuffle_vector.cpp
    ${DIR_SRC}
    ${DIR_SHUFFLE}
    ${DIR_KHPRF}
    ${DIR_UTILS}
)
target_link_libraries(shuffle_vector oc::libOTe)
target_compile_options(shuffle_vector PUBLIC -march=native -std=c++20)
target_include_directories(shuffle_vector PUBLIC ${libOTe_DIR})
