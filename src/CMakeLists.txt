set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${BasicJ2k_SOURCE_DIR}/include)
include_directories(${OPENJPEG_INCLUDE_DIR})

add_library(BasicJ2k STATIC j2kconverter.cpp)
target_include_directories(BasicJ2k PUBLIC ${PROJECT_SOURCE_DIR}/include)

set(OPENJPEG_LIBRARY_NAME "${OPENJPEG_LIBRARY_DIR}${CMAKE_STATIC_LIBRARY_PREFIX}openjp2${CMAKE_STATIC_LIBRARY_SUFFIX}")
target_link_libraries(BasicJ2k PUBLIC ${OPENJPEG_LIBRARY_NAME})
