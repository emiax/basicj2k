set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${BasicJ2k_SOURCE_DIR}/include)
include_directories(${OPENJPEG_INCLUDE_DIR})
link_directories(${OPENJPEG_LIBRARY_DIR})

add_library(BasicJ2k STATIC j2kconverter.cpp)
target_include_directories(BasicJ2k PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(BasicJ2k PUBLIC ${OPENJPEG_LIBRARIES} libopenjp2.so) 