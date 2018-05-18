cmake_minimum_required(VERSION 2.8)

PROJECT(dmlua)

IF(UNIX)
    ADD_DEFINITIONS(-DLUA_USE_LINUX)
    ADD_DEFINITIONS(-DLUA_COMPAT_MODULE)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/dmlua)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/lua)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/tolua)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/test)
    LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib/lin)

    SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/lin)

    #FILE(GLOB_RECURSE DMLUA_SOURCES
        #${CMAKE_CURRENT_SOURCE_DIR}/include/dmlua/*.hpp
        #${CMAKE_CURRENT_SOURCE_DIR}/include/dmlua/*.h)

    #ADD_LIBRARY(dmlua ${DMLUA_SOURCES})

    FILE(GLOB_RECURSE TOLUA_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.h)

    ADD_LIBRARY(tolua ${TOLUA_SOURCES})

    FILE(GLOB_RECURSE TOLUA++_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/tolua.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/toluabind.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/*.h)

    ADD_EXECUTABLE(tolua++ ${TOLUA++_SOURCES})
    TARGET_LINK_LIBRARIES(tolua++ tolua m dl pthread)

    FILE(GLOB_RECURSE TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.pkg
        ${CMAKE_CURRENT_SOURCE_DIR}/include/dmlua/dmlua.h)

    ADD_EXECUTABLE(dmluatest ${TEST_SOURCES})
    TARGET_LINK_LIBRARIES(dmluatest tolua m dl pthread)

    ADD_CUSTOM_COMMAND(
		TARGET dmluatest
		PRE_BUILD
        COMMAND tolua++ -t -n interface -o interface.cpp -H interface.h interface.pkg
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/script
        DEPENDS tolua++
        )
    FILE(GLOB_RECURSE LUACHECK_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.pkg)

    ADD_EXECUTABLE(dmluacheck ${LUACHECK_SOURCES})
    TARGET_LINK_LIBRARIES(dmluacheck tolua)
ELSEIF(WIN32)
    ADD_DEFINITIONS(-DLUA_COMPAT_MODULE)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/dmlua)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/lua)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/tolua)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/test)

    LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib/win)

    SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/win)

    #FILE(GLOB_RECURSE DMLUA_SOURCES
        #${CMAKE_CURRENT_SOURCE_DIR}/include/dmlua/*.hpp
        #${CMAKE_CURRENT_SOURCE_DIR}/include/dmlua/*.h)

    #ADD_LIBRARY(dmlua ${DMLUA_SOURCES})

    FILE(GLOB_RECURSE TOLUA_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua/*.h

        ${CMAKE_CURRENT_SOURCE_DIR}/include/tolua.h)

    ADD_LIBRARY(tolua ${TOLUA_SOURCES})

    FILE(GLOB_RECURSE TOLUA++_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/tolua.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/toluabind.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/*.h

        ${CMAKE_CURRENT_SOURCE_DIR}/include/tolua.h)

    ADD_EXECUTABLE(tolua++ ${TOLUA++_SOURCES})
    TARGET_LINK_LIBRARIES(tolua++ tolua)

    FILE(GLOB_RECURSE TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.pkg
        ${CMAKE_CURRENT_SOURCE_DIR}/include/dmlua/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/dmlua/dmlua.h)

    ADD_EXECUTABLE(dmluatest ${TEST_SOURCES})
    TARGET_LINK_LIBRARIES(dmluatest tolua)

    ADD_CUSTOM_COMMAND(
		TARGET dmluatest
		PRE_BUILD
        COMMAND tolua++ -t -n interface -o interface.cpp -H interface.h interface.pkg
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/script
        DEPENDS tolua++
        )
    FILE(GLOB_RECURSE LUACHECK_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/luacheck/*.pkg)

    ADD_EXECUTABLE(dmluacheck ${LUACHECK_SOURCES})
    TARGET_LINK_LIBRARIES(dmluacheck tolua)
ENDIF(UNIX)
