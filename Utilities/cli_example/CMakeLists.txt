cmake_minimum_required(VERSION 3.10)

project(cli_example)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11")
add_executable(${PROJECT_NAME} "main.cpp" "main.h")

if (WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(DLL_SUFFIX "64")
    else()
        SET(DLL_SUFFIX "32")
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/lib/thirdParty/nlohmann/)
    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/lib/libhs_datalog/)
    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/lib/logger/)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libhs_datalog/${DLL_SUFFIX}bit/libhs_datalog_v2.dll)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

endif (WIN32)


if (UNIX)

    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/lib/thirdParty/nlohmann/)
    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/lib/libhs_datalog/)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libhs_datalog/linux/libhs_datalog_v2.so)
    target_link_libraries(${PROJECT_NAME} pthread)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

endif (UNIX)
