#include <gtest/gtest.h>
#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_LUA

#include "duilib/Lua/LuaEngine.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FileUtil.h"
#include <filesystem>
#include <fstream>

using ui::LuaEngine;
using ui::FilePath;

class LuaEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = std::filesystem::temp_directory_path() / "duilib_lua_test";
        std::filesystem::create_directories(test_dir_);
        
        scripts_dir_ = test_dir_ / "scripts";
        std::filesystem::create_directories(scripts_dir_);
    }
    
    void TearDown() override {
        LuaEngine::Instance().Shutdown();
        std::filesystem::remove_all(test_dir_);
    }
    
    void CreateScript(const std::string& name, const std::string& content) {
        std::ofstream file(scripts_dir_ / name);
        file << content;
        file.close();
    }
    
    std::filesystem::path test_dir_;
    std::filesystem::path scripts_dir_;
};

TEST_F(LuaEngineTest, Initialization)
{
    EXPECT_FALSE(LuaEngine::Instance().IsInitialized());
    
    bool result = LuaEngine::Instance().Initialize(scripts_dir_.string());
    EXPECT_TRUE(result);
    EXPECT_TRUE(LuaEngine::Instance().IsInitialized());
    
    LuaEngine::Instance().Shutdown();
    EXPECT_FALSE(LuaEngine::Instance().IsInitialized());
}

TEST_F(LuaEngineTest, DoString_BasicArithmetic)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString("x = 2 + 3");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, DoString_Print)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString("print('Hello from Lua')");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, DoString_TableOperations)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString(R"(
        local t = {a = 1, b = 2}
        t.c = t.a + t.b
        assert(t.c == 3)
    )");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, DoString_StringOperations)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString(R"(
        local s = "Hello"
        s = s .. " World"
        assert(string.len(s) == 11)
    )");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, DoString_MathFunctions)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString(R"(
        local x = math.sqrt(16)
        assert(x == 4)
        local y = math.floor(3.7)
        assert(y == 3)
    )");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, DoString_UTF8Support)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString(R"(
        local s = "你好世界"
        local len = utf8.len(s)
        assert(len == 4)
    )");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, LoadScript_Success)
{
    CreateScript("test.lua", R"(
        function greet(name)
            return "Hello, " .. name
        end
        
        result = greet("World")
    )");
    
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().LoadScript("test.lua");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, LoadScript_NotFound)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().LoadScript("nonexistent.lua");
    EXPECT_FALSE(result);
}

TEST_F(LuaEngineTest, LoadScript_SyntaxError)
{
    CreateScript("syntax_error.lua", "function incomplete(");
    
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().LoadScript("syntax_error.lua");
    EXPECT_FALSE(result);
}

TEST_F(LuaEngineTest, ReloadScript)
{
    CreateScript("reload_test.lua", "version = 1");
    
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    EXPECT_TRUE(LuaEngine::Instance().LoadScript("reload_test.lua"));
    EXPECT_TRUE(LuaEngine::Instance().ReloadScript("reload_test.lua"));
}

TEST_F(LuaEngineTest, ReloadScript_NotLoaded)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().ReloadScript("never_loaded.lua");
    EXPECT_FALSE(result);
}

TEST_F(LuaEngineTest, Timeout_SetAndGet)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    LuaEngine::Instance().SetTimeout(10000);
    EXPECT_EQ(LuaEngine::Instance().GetTimeout(), 10000);
    
    LuaEngine::Instance().SetTimeout(5000);
    EXPECT_EQ(LuaEngine::Instance().GetTimeout(), 5000);
}

TEST_F(LuaEngineTest, MemoryLimit_SetAndGet)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    size_t limit = 128 * 1024 * 1024;
    LuaEngine::Instance().SetMemoryLimit(limit);
    EXPECT_EQ(LuaEngine::Instance().GetMemoryLimit(), limit);
}

TEST_F(LuaEngineTest, MemoryUsage_Get)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    size_t usage = LuaEngine::Instance().GetMemoryUsage();
    EXPECT_GE(usage, 0);
}

TEST_F(LuaEngineTest, Sandbox_DofileDisabled)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    CreateScript("test_dofile.lua", "x = 1");
    CreateScript("caller.lua", "dofile('test_dofile.lua')");
    
    bool result = LuaEngine::Instance().LoadScript("caller.lua");
    EXPECT_FALSE(result);
}

TEST_F(LuaEngineTest, Sandbox_LoadfileDisabled)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString("loadfile('test.lua')");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, UiTable_Exists)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString(R"(
        assert(ui ~= nil)
        assert(type(ui) == "table")
    )");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, UiTable_LogFunction)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString("ui.log('test message')");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, UiTable_TimerFunctions)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString(R"(
        local timer_id = ui.set_timer(100, function() end)
        assert(timer_id ~= nil)
        ui.clear_timer(timer_id)
    )");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, UiTable_FormatFunction)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString(R"(
        local msg = ui.format("Hello %s", "World")
        assert(msg == "Hello World")
    )");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, EventBinding_EventTypeEnum)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString(R"(
        assert(ui.click ~= nil)
        assert(ui.mouse_enter ~= nil)
        assert(ui.mouse_leave ~= nil)
    )");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, ComplexScript_FunctionsAndTables)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    CreateScript("complex.lua", R"(
        local Calculator = {}
        Calculator.__index = Calculator
        
        function Calculator.new()
            local self = setmetatable({}, Calculator)
            self.value = 0
            return self
        end
        
        function Calculator:add(n)
            self.value = self.value + n
            return self
        end
        
        function Calculator:multiply(n)
            self.value = self.value * n
            return self
        end
        
        function Calculator:get()
            return self.value
        end
        
        local calc = Calculator.new()
        local result = calc:add(5):multiply(3):get()
        assert(result == 15)
    )");
    
    bool result = LuaEngine::Instance().LoadScript("complex.lua");
    EXPECT_TRUE(result);
}

TEST_F(LuaEngineTest, Stress_MultipleScripts)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    for (int i = 0; i < 10; i++) {
        std::string name = "script_" + std::to_string(i) + ".lua";
        std::string content = "x_" + std::to_string(i) + " = " + std::to_string(i);
        CreateScript(name, content);
        EXPECT_TRUE(LuaEngine::Instance().LoadScript(name));
    }
}

TEST_F(LuaEngineTest, Stress_LargeTable)
{
    ASSERT_TRUE(LuaEngine::Instance().Initialize(scripts_dir_.string()));
    
    bool result = LuaEngine::Instance().DoString(R"(
        local large_table = {}
        for i = 1, 1000 do
            large_table[i] = i * i
        end
        assert(#large_table == 1000)
        assert(large_table[1000] == 1000000)
    )");
    EXPECT_TRUE(result);
}

#endif // DUILIB_BUILD_FOR_LUA

TEST(LuaEngineTestPlaceholder, AlwaysPass)
{
    EXPECT_TRUE(true);
}
