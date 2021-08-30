cmake_minimum_required(VERSION 2.8)

#It seems Cmake does not set default bild type so we force it
if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Debug CACHE STRING "Debug" FORCE )
endif()

project(opcuaServer)

set( CMAKE_CXX_FLAGS "-std=c++11" )

add_executable(opcuaClient src/example_client.cpp)

if(UNIX)
    find_package(spdlog REQUIRED) 
    if(spdlog_FOUND)
        message(STATUS "Found spdlog")           
        target_link_libraries(opcuaClient spdlog::spdlog) 
        #add_compile_options(-DHAVE_SYSTEM_SPDLOG) 
        set(CMAKE_CXX_FLAGS "-DHAVE_SYSTEM_SPDLOG ${CMAKE_CXX_FLAGS}")  
    endif()
    find_package(FreeOpcUa REQUIRED)                               
    if(FreeOpcUa_FOUND)                                                      
        message(STATUS "Found FreeOpcUa")                                   
        target_link_libraries(opcuaClient opcuacore opcuaclient opcuaserver opcuaprotocol)     
        #add_compile_options(-DHAVE_SYSTEM_SPDLOG)  
        #set(CMAKE_CXX_FLAGS "-DHAVE_SYSTEM_SPDLOG ${CMAKE_CXX_FLAGS}")       
    endif() 
endif()

