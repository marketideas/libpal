cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE Debug)
set(APPVERSION "1.0.0")
set(LIBRARY_NAME pal)

include(./lib${LIBRARY_NAME}/cmake/CMakeHeader.txt)

set(SOURCE
  ${PROJECT_ROOT}/${PROJECT_ID}.cpp
)

include_directories(BEFORE 
  ${PROJECT_ROOT}
  ${PROJECT_ROOT}/lib${LIBRARY_NAME}/include/${LIBRARY_NAME}
)

add_subdirectory(${PROJECT_ROOT}/lib${LIBRARY_NAME})

include(${PROJECT_ROOT}/lib${LIBRARY_NAME}/cmake/CMakeApp.txt)

add_executable( ${PROJECT_NAME} ${SOURCE})

target_link_libraries (
${PROJECT_NAME}
${LIBRARY_NAME}
pthread
PocoFoundation
PocoNet
PocoUtil
PocoNetSSL
PocoCrypto
)

include(./lib${LIBRARY_NAME}/cmake/CMakeCommands.txt)




