cmake_minimum_required(VERSION 3.0)
project(libpal)


set (CMAKE_CXX_STANDARD 11)

set ( PROJECT_NAME "libpal" )

set(LIBRARY_NAME "pal")
set(LIBVERSION "2.0.0")

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

set (TARGET_BIN "${LIBRARY_NAME}")

set(PROJECT_DEFINES 
" \
-DLIBVERSION=${LIBVERSION} \
-DAPPVERSION=${APPVERSION} \
" 
)

set(CMAKE_C_FLAGS
" \
${PROJECT_DEFINES} \
-fbounds-check \
-ffunction-sections \
-fdata-sections \
-fno-strict-aliasing \
-funwind-tables \
"
)
set(CMAKE_CXX_FLAGS_DEBUG "${PROJECT_DEFINES} ${DEBUG_OPT} ")
set(CMAKE_CXX_FLAGS_RELEASE "${PROJECT_DEFINES}  ${RELEASE_OPT}")

set(CMAKE_EXE_LINKER_FLAGS
" \
")

set(SOURCE
  ${PROJECT_ROOT}/src/main.cpp
  ${PROJECT_ROOT}/src/baseapp.cpp
  ${PROJECT_ROOT}/src/pallogger.cpp
  ${PROJECT_ROOT}/src/eventtask.cpp
  ${PROJECT_ROOT}/src/palutils.cpp
  #${PROJECT_ROOT}/src/httpserver.cpp
  #${PROJECT_ROOT}/src/httpclient.cpp
)

include_directories(BEFORE 
  ${PROJECT_ROOT}/include/pal
  ./
  ${BASE_DIR}
)

add_library( ${LIBRARY_NAME} ${SOURCE})

INSTALL( TARGETS ${LIBRARY_NAME} 
	DESTINATION /usr/local/lib 
	INCLUDES DESTINATION /usr/local/include/${LIBRARY_NAME} 
)








