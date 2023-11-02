
if(${USE_SGX})
   if(${SGX_MODE} )
      message("Building in hardware mode")
   else()
      message("Building in sim mode")
   endif()
endif()

set(UNWARN "-Wno-unused-result")
set(SGX_COMMON_FLAGS "${SGX_COMMON_FLAGS} ${UNWARN}")
set(CMAKE_C_FLAGS  "${SGX_COMMON_FLAGS}")


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(CMAKE_C_FLAGS " -maes -lgmp") 

FILE(GLOB CSYMFiles "symmetria/*.c")
FILE(GLOB CppSYMFiles "symmetria/*.cpp")
set(HeFiles "${CSYMFiles};${CppSYMFiles};../../utils/LogBase.cpp;../../utils/config.cpp;../../utils/KeyManager.cpp;../../utils/Timer.cpp;../../utils/Timestamp.cpp;")


filter_regex(EXCLUDE "(ore|ORE)" HeFiles ${HeFiles})

set(OreFiles
symmetria/ore_blk.c
symmetria/ore_crypto.c
symmetria/ORE_STR22STRUCT.cpp
symmetria/t.cpp 
)

include_directories(${PROJECT_SOURCE_DIR}/db_include ${ORE_Lib} libudf/decider/ symmetria)
get_property(dirs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(">>> include_dirs=${dirs}") #打印一下目录情况
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS " -fPIC")

add_library(${HE_Lib} STATIC ${HeFiles})
add_library(${ORE_Lib} STATIC ${OreFiles})

combine_archives(${SYM_Lib} "${HE_Lib};${ORE_Lib}")


target_link_libraries(${SYM_Lib} ${HE_Lib} ${ORE_Lib} ${GMP_LIBRARIES} ${LOG4CPP_LIBRARIES})
target_include_directories(${SYM_Lib} BEFORE PUBLIC ${PROJECT_SOURCE_DIR}/db_include)
add_subdirectory(libudf)