cmake_minimum_required(VERSION 3.18)
project(duilib_tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
if(NOT BUILD_TESTING)
    return()
endif()

find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

set(DUILIB_TEST_GTEST_MAIN_TARGET GTest::gtest_main)
if(NOT TARGET ${DUILIB_TEST_GTEST_MAIN_TARGET} AND TARGET gtest_main)
    set(DUILIB_TEST_GTEST_MAIN_TARGET gtest_main)
endif()
if(NOT TARGET ${DUILIB_TEST_GTEST_MAIN_TARGET})
    message(FATAL_ERROR "GoogleTest gtest_main target not found")
endif()

get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

include(GoogleTest)

function(register_gtest_target target_name)
    target_link_libraries(${target_name} PRIVATE
        ${DUILIB_TEST_GTEST_MAIN_TARGET}
    )
    gtest_add_tests(TARGET ${target_name})
endfunction()

add_executable(duilib_tests
    Core/CompiledResourceLoaderTest.cpp
    ResourceCompiler/ResourceCompilerTest.cpp
    ResourceCompiler/ResourceCompilerTest.h
    Utils/test_StringConvert.cpp
    "${DUILIB_SRC_ROOT_DIR}/duilib/Core/CompiledResourceLoader.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/StringConvert.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/third_party/convert_utf/ConvertUTF.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/ResourceCompiler/ResourceCompiler.cpp"
)

target_include_directories(duilib_tests PRIVATE
    "${DUILIB_SRC_ROOT_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/ResourceCompiler"
)
register_gtest_target(duilib_tests)

# EventBus 单元测试（独立的测试可执行文件）
option(DUILIB_BUILD_EVENTBUS_TESTS "Build EventBus tests" OFF)
if(DUILIB_BUILD_EVENTBUS_TESTS)
    add_executable(eventbus_tests
        Core/EventBusTest.cpp
        "${DUILIB_SRC_ROOT_DIR}/duilib/Core/EventBus.cpp"
    )
    target_include_directories(eventbus_tests PRIVATE
        "${DUILIB_SRC_ROOT_DIR}"
    )
    register_gtest_target(eventbus_tests)
endif()

add_executable(core_types_tests
    Core/test_UiColor.cpp
    Core/test_UiColors.cpp
    Core/test_UiEstInt.cpp
    Core/test_UiFixedInt.cpp
    Core/test_UiFont.cpp
    Core/test_UiMargin.cpp
    Core/test_UiPadding.cpp
    Core/test_UiPoint.cpp
    Core/test_UiPointF.cpp
    Core/test_UiRect.cpp
    Core/test_UiRectF.cpp
    Core/test_UiSize16.cpp
    Core/test_UiSize64.cpp
    Core/test_UiSize.cpp
    Core/test_UiString.cpp
    Core/test_UiTypes.cpp
    Core/test_WindowCreateAttributes.cpp
    Core/test_WindowCreateParam.cpp
    "${DUILIB_SRC_ROOT_DIR}/duilib/Core/WindowCreateParam.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/Core/UiColors.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/StringUtil.cpp"
)
target_include_directories(core_types_tests PRIVATE
    "${DUILIB_SRC_ROOT_DIR}"
)
register_gtest_target(core_types_tests)

add_executable(stringutil_tests
    Utils/test_StringUtil.cpp
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/StringUtil.cpp"
)
target_include_directories(stringutil_tests PRIVATE
    "${DUILIB_SRC_ROOT_DIR}"
)
register_gtest_target(stringutil_tests)

add_executable(filesystem_tests
    Core/test_ResourceParam.cpp
    Utils/test_FilePath.cpp
    Utils/test_FileUtil.cpp
    Utils/test_FileTime.cpp
    Utils/test_FilePathUtil.cpp
    Utils/test_StringCharset.cpp
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/FilePath.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/FileUtil.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/FileTime.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/FilePathUtil.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/StringCharset.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/StringUtil.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/StringConvert.cpp"
    "${DUILIB_SRC_ROOT_DIR}/duilib/third_party/convert_utf/ConvertUTF.cpp"
)
target_include_directories(filesystem_tests PRIVATE
    "${DUILIB_SRC_ROOT_DIR}"
)
register_gtest_target(filesystem_tests)

option(DUILIB_BUILD_CREATOR_TESTS "Build Creator module tests" OFF)
if(DUILIB_BUILD_CREATOR_TESTS)
    add_executable(creator_tests
        Creator/ControlMetadataTest.cpp
        Creator/DocumentTest.cpp
        Creator/UndoManagerTest.cpp
        "${DUILIB_SRC_ROOT_DIR}/creator/Core/ControlMetadata.cpp"
        "${DUILIB_SRC_ROOT_DIR}/creator/Core/Document.cpp"
        "${DUILIB_SRC_ROOT_DIR}/creator/Core/UndoManager.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/third_party/xml/pugixml.cpp"
    )
    target_include_directories(creator_tests PRIVATE
        "${DUILIB_SRC_ROOT_DIR}"
        "${DUILIB_SRC_ROOT_DIR}/creator"
    )
    register_gtest_target(creator_tests)
endif()

# Lua binding tests
option(DUILIB_BUILD_LUA_TESTS "Build Lua binding tests" OFF)
if(DUILIB_BUILD_LUA_TESTS)
    # Collect Lua source files (exclude lua.c and luac.c)
    file(GLOB LUA_SRCS "${DUILIB_SRC_ROOT_DIR}/duilib/third_party/lua/src/*.c")
    list(FILTER LUA_SRCS EXCLUDE REGEX "lua\.c$")
    list(FILTER LUA_SRCS EXCLUDE REGEX "luac\.c$")
    
    add_executable(lua_tests
        Lua/LuaEngineTest.cpp
        "${DUILIB_SRC_ROOT_DIR}/duilib/Lua/LuaEngine.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Lua/LuaControlBinding.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Lua/LuaWindowBinding.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Lua/LuaBoxBinding.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Lua/LuaEventBinding.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Lua/LuaUtilBinding.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Lua/LuaAdvancedControlBinding.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/StringConvert.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/FilePath.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/FileUtil.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/Utils/StringUtil.cpp"
        "${DUILIB_SRC_ROOT_DIR}/duilib/third_party/convert_utf/ConvertUTF.cpp"
        ${LUA_SRCS}
    )
    
    target_include_directories(lua_tests PRIVATE
        "${DUILIB_SRC_ROOT_DIR}"
        "${DUILIB_SRC_ROOT_DIR}/duilib/third_party/lua/src"
        "${DUILIB_SRC_ROOT_DIR}/duilib/third_party/sol/include"
    )
    
    target_compile_definitions(lua_tests PRIVATE DUILIB_LUA=1)
    
    register_gtest_target(lua_tests)
endif()
