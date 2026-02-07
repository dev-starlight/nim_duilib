#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_LUA

#include "LuaEngine.h"
#include "LuaControlBinding.h"
#include "LuaWindowBinding.h"
#include "LuaBoxBinding.h"
#include "LuaEventBinding.h"
#include "LuaUtilBinding.h"
#include "LuaAdvancedControlBinding.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Utils/LogUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"

#include "sol/sol.hpp"

#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <sstream>
#include <algorithm>

namespace ui {

static void LuaDebugLog(const DString& logText)
{
#ifdef _DEBUG
    LogUtil::OutputLine(logText);
#else
    UNUSED_VARIABLE(logText);
#endif
}

// Execution timeout hook
struct LuaTimeoutData {
    std::chrono::steady_clock::time_point startTime;
    int timeoutMs;
    int instructionCount;
    int checkInterval;
};

static thread_local LuaTimeoutData* g_timeoutData = nullptr;

static void LuaHook(lua_State* L, lua_Debug* ar) {
    (void)L;
    (void)ar;
    if (g_timeoutData == nullptr) {
        return;
    }
    g_timeoutData->instructionCount++;
    if (g_timeoutData->instructionCount % g_timeoutData->checkInterval != 0) {
        return;
    }
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_timeoutData->startTime).count();
    if (elapsed > g_timeoutData->timeoutMs) {
        luaL_error(L, "Script execution timeout after %d ms", g_timeoutData->timeoutMs);
    }
}

struct LuaEngine::Impl {
    std::unique_ptr<sol::state> lua;
    DString scriptRootPath;
    bool initialized = false;
    std::unordered_set<DString> loadedScripts;
    std::unordered_map<Window*, sol::environment> windowEnvironments;
    std::unordered_map<std::string, std::function<void(sol::state&)>> modules;
    int timeoutMs = 5000;
    size_t memoryLimit = 64 * 1024 * 1024;
};

class ScopedTimeout {
public:
    ScopedTimeout(lua_State* L, int timeoutMs) : m_L(L) {
        m_data.startTime = std::chrono::steady_clock::now();
        m_data.timeoutMs = timeoutMs;
        m_data.instructionCount = 0;
        m_data.checkInterval = 1000;
        g_timeoutData = &m_data;
        lua_sethook(m_L, LuaHook, LUA_MASKCOUNT, m_data.checkInterval);
    }
    ~ScopedTimeout() {
        lua_sethook(m_L, nullptr, 0, 0);
        g_timeoutData = nullptr;
    }
private:
    lua_State* m_L;
    LuaTimeoutData m_data;
};

LuaEngine& LuaEngine::Instance() {
    static LuaEngine instance;
    return instance;
}

LuaEngine::LuaEngine()
    : m_impl(std::make_unique<Impl>()) {
}

LuaEngine::~LuaEngine() {
    Shutdown();
}

bool LuaEngine::Initialize(const DString& scriptRootPath) {
    LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::Initialize] begin: scriptRootPath=%s"),
                                          scriptRootPath.c_str()));
    if (m_impl->initialized) {
        LuaDebugLog(_T("[LuaEngine::Initialize] skipped: already initialized"));
        return true;
    }

    try {
        m_impl->lua = std::make_unique<sol::state>();
        m_impl->scriptRootPath = scriptRootPath;

        // Open safe standard libraries
        SetupSandbox();

        // Set up search paths
        SetupSearchPaths(scriptRootPath);

        // Register all built-in bindings
        RegisterAllBindings();

        m_impl->initialized = true;
        LuaDebugLog(_T("[LuaEngine::Initialize] success"));
        return true;
    }
    catch (const std::exception& e) {
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::Initialize] failed: %s"),
                                              StringConvert::UTF8ToT(e.what()).c_str()));
        return false;
    }
}

void LuaEngine::Shutdown() {
    LuaDebugLog(_T("[LuaEngine::Shutdown] begin"));
    if (!m_impl->initialized) {
        LuaDebugLog(_T("[LuaEngine::Shutdown] skipped: not initialized"));
        return;
    }

    // Clear window environments
    m_impl->windowEnvironments.clear();

    // Clear loaded scripts
    m_impl->loadedScripts.clear();

    // Release Lua state
    m_impl->lua.reset();

    m_impl->initialized = false;
    LuaDebugLog(_T("[LuaEngine::Shutdown] end"));
}

bool LuaEngine::IsInitialized() const {
    return m_impl->initialized;
}

bool LuaEngine::LoadScript(const DString& scriptPath) {
    LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::LoadScript] begin: scriptPath=%s"),
                                          scriptPath.c_str()));
    if (!m_impl->initialized || !m_impl->lua) {
        LuaDebugLog(_T("[LuaEngine::LoadScript] failed: engine not initialized"));
        return false;
    }

    try {
        FilePath fullPath(m_impl->scriptRootPath);
        fullPath.JoinFilePath(FilePath(scriptPath));

        sol::load_result script = m_impl->lua->load_file(fullPath.ToStringA());
        if (!script.valid()) {
            sol::error err = script;
            LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::LoadScript] failed: load_file error=%s"),
                                                  StringConvert::UTF8ToT(err.what()).c_str()));
            return false;
        }

        sol::protected_function func = script;
        lua_State* L = m_impl->lua->lua_state();
        ScopedTimeout timeout(L, m_impl->timeoutMs);

        sol::protected_function_result result = func();
        if (!result.valid()) {
            sol::error err = result;
            LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::LoadScript] failed: execute error=%s"),
                                                  StringConvert::UTF8ToT(err.what()).c_str()));
            return false;
        }

        m_impl->loadedScripts.insert(scriptPath);
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::LoadScript] success: scriptPath=%s"),
                                              scriptPath.c_str()));
        return true;
    }
    catch (const std::exception& e) {
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::LoadScript] failed: exception=%s"),
                                              StringConvert::UTF8ToT(e.what()).c_str()));
        return false;
    }
}

bool LuaEngine::DoString(const DString& luaCode) {
    LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::DoString] begin: codeLength=%u"),
                                          static_cast<uint32_t>(luaCode.size())));
    if (!m_impl->initialized || !m_impl->lua) {
        LuaDebugLog(_T("[LuaEngine::DoString] failed: engine not initialized"));
        return false;
    }

    try {
        lua_State* L = m_impl->lua->lua_state();
        ScopedTimeout timeout(L, m_impl->timeoutMs);

        sol::protected_function_result result = m_impl->lua->script(StringConvert::TToUTF8(luaCode));
        if (!result.valid()) {
            sol::error err = result;
            LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::DoString] failed: execute error=%s"),
                                                  StringConvert::UTF8ToT(err.what()).c_str()));
            return false;
        }
        LuaDebugLog(_T("[LuaEngine::DoString] success"));
        return true;
    }
    catch (const std::exception& e) {
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::DoString] failed: exception=%s"),
                                              StringConvert::UTF8ToT(e.what()).c_str()));
        return false;
    }
}

bool LuaEngine::ReloadScript(const DString& scriptPath) {
    LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::ReloadScript] begin: scriptPath=%s"),
                                          scriptPath.c_str()));
    if (!m_impl->initialized) {
        LuaDebugLog(_T("[LuaEngine::ReloadScript] failed: engine not initialized"));
        return false;
    }

    // Check if script was previously loaded
    if (m_impl->loadedScripts.find(scriptPath) == m_impl->loadedScripts.end()) {
        LuaDebugLog(_T("[LuaEngine::ReloadScript] failed: script not loaded before"));
        return false;
    }

    return LoadScript(scriptPath);
}

void LuaEngine::ReloadAll() {
    LuaDebugLog(_T("[LuaEngine::ReloadAll] begin"));
    if (!m_impl->initialized) {
        LuaDebugLog(_T("[LuaEngine::ReloadAll] skipped: engine not initialized"));
        return;
    }

    // Copy the list to avoid iterator invalidation
    std::vector<DString> scripts(m_impl->loadedScripts.begin(), m_impl->loadedScripts.end());
    LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::ReloadAll] count=%u"),
                                          static_cast<uint32_t>(scripts.size())));

    for (const auto& script : scripts) {
        ReloadScript(script);
    }
}

bool LuaEngine::CreateWindowEnvironment(Window* pWindow) {
    LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::CreateWindowEnvironment] begin: window=%p"),
                                          pWindow));
    if (!m_impl->initialized || !m_impl->lua || !pWindow) {
        LuaDebugLog(_T("[LuaEngine::CreateWindowEnvironment] failed: invalid state or window"));
        return false;
    }

    try {
        sol::environment env(*m_impl->lua, sol::create, m_impl->lua->globals());
        m_impl->windowEnvironments[pWindow] = env;
        LuaDebugLog(_T("[LuaEngine::CreateWindowEnvironment] success"));
        return true;
    }
    catch (const std::exception& e) {
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::CreateWindowEnvironment] failed: %s"),
                                              StringConvert::UTF8ToT(e.what()).c_str()));
        return false;
    }
}

void LuaEngine::DestroyWindowEnvironment(Window* pWindow) {
    if (!pWindow) {
        return;
    }

    auto it = m_impl->windowEnvironments.find(pWindow);
    if (it != m_impl->windowEnvironments.end()) {
        m_impl->windowEnvironments.erase(it);
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::DestroyWindowEnvironment] removed: window=%p"),
                                              pWindow));
    }
}

void LuaEngine::RegisterModule(const std::string& moduleName,
                                std::function<void(sol::state&)> registerFunc) {
    if (!m_impl->initialized || !m_impl->lua) {
        LuaDebugLog(_T("[LuaEngine::RegisterModule] skipped: engine not initialized"));
        return;
    }

    m_impl->modules[moduleName] = registerFunc;

    try {
        registerFunc(*m_impl->lua);
    }
    catch (const std::exception& e) {
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::RegisterModule] failed: module=%s, error=%s"),
                                              StringConvert::UTF8ToT(moduleName).c_str(),
                                              StringConvert::UTF8ToT(e.what()).c_str()));
    }
}

sol::state& LuaEngine::GetState() {
    return *m_impl->lua;
}

void LuaEngine::RegisterAllBindings() {
    if (!m_impl->lua) {
        return;
    }

    LuaControlBinding::Register(*m_impl->lua);
    LuaWindowBinding::Register(*m_impl->lua);
    LuaBoxBinding::Register(*m_impl->lua);
    LuaEventBinding::Register(*m_impl->lua);
    LuaUtilBinding::Register(*m_impl->lua);
    LuaAdvancedControlBinding::Register(*m_impl->lua);
}

void LuaEngine::SetupSearchPaths(const DString& scriptRootPath) {
    if (!m_impl->lua) {
        return;
    }

    // Add script root path to Lua package.path
    std::string path = StringConvert::TToUTF8(scriptRootPath);
    // Convert backslashes to forward slashes for Lua
    std::replace(path.begin(), path.end(), '\\', '/');

    std::string luaPath = "package.path = package.path .. ';" + path + "/?.lua;" + path + "/?/init.lua'";
    m_impl->lua->script(luaPath);
    LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::SetupSearchPaths] path=%s"),
                                          scriptRootPath.c_str()));
}

void LuaEngine::SetupSandbox() {
    if (!m_impl->lua) {
        return;
    }

    // Open safe standard libraries only
    m_impl->lua->open_libraries(
        sol::lib::base,      // Basic functions (print, type, pairs, ipairs...)
        sol::lib::string,    // String operations
        sol::lib::table,     // Table operations
        sol::lib::math,      // Math functions
        sol::lib::utf8       // UTF-8 support
    );

    // Remove dangerous functions from base library
    (*m_impl->lua)["dofile"] = sol::nil;
    (*m_impl->lua)["loadfile"] = sol::nil;
}

int LuaEngine::LuaErrorHandler(lua_State* L) {
    // TODO: Better error handling with stack trace
    UNUSED_VARIABLE(L);
    return 0;
}

// Template implementations
template<typename... Args>
void LuaEngine::CallFunction(const std::string& funcName, Args&&... args) {
    if (!m_impl->initialized || !m_impl->lua) {
        return;
    }

    try {
        sol::protected_function func = (*m_impl->lua)[funcName];
        if (func.valid()) {
            func(std::forward<Args>(args)...);
        }
        else {
            LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::CallFunction] function not found: %s"),
                                                  StringConvert::UTF8ToT(funcName).c_str()));
        }
    }
    catch (const std::exception& e) {
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::CallFunction] failed: function=%s, error=%s"),
                                              StringConvert::UTF8ToT(funcName).c_str(),
                                              StringConvert::UTF8ToT(e.what()).c_str()));
    }
}

template<typename... Args>
void LuaEngine::CallWindowFunction(Window* pWindow, const std::string& funcName, Args&&... args) {
    if (!m_impl->initialized || !m_impl->lua || !pWindow) {
        return;
    }

    auto it = m_impl->windowEnvironments.find(pWindow);
    if (it == m_impl->windowEnvironments.end()) {
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::CallWindowFunction] window env not found: window=%p"),
                                              pWindow));
        return;
    }

    try {
        sol::protected_function func = it->second[funcName];
        if (func.valid()) {
            func(std::forward<Args>(args)...);
        }
        else {
            LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::CallWindowFunction] function not found: %s"),
                                                  StringConvert::UTF8ToT(funcName).c_str()));
        }
    }
    catch (const std::exception& e) {
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::CallWindowFunction] failed: function=%s, error=%s"),
                                              StringConvert::UTF8ToT(funcName).c_str(),
                                              StringConvert::UTF8ToT(e.what()).c_str()));
    }
}

template<typename Func>
void LuaEngine::RegisterFunction(const std::string& name, Func&& func) {
    if (!m_impl->initialized || !m_impl->lua) {
        return;
    }

    try {
        (*m_impl->lua)[name] = std::forward<Func>(func);
    }
    catch (const std::exception& e) {
        LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::RegisterFunction] failed: name=%s, error=%s"),
                                              StringConvert::UTF8ToT(name).c_str(),
                                              StringConvert::UTF8ToT(e.what()).c_str()));
    }
}

void LuaEngine::SetTimeout(int timeoutMs) {
    m_impl->timeoutMs = timeoutMs;
    LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::SetTimeout] timeoutMs=%d"), timeoutMs));
}

int LuaEngine::GetTimeout() const {
    return m_impl->timeoutMs;
}

void LuaEngine::SetMemoryLimit(size_t bytes) {
    m_impl->memoryLimit = bytes;
    LuaDebugLog(StringUtil::Printf(_T("[LuaEngine::SetMemoryLimit] bytes=%llu"),
                                          static_cast<unsigned long long>(bytes)));
    if (m_impl->lua) {
        lua_State* L = m_impl->lua->lua_state();
        lua_gc(L, LUA_GCCOLLECT, 0);
    }
}

size_t LuaEngine::GetMemoryLimit() const {
    return m_impl->memoryLimit;
}

size_t LuaEngine::GetMemoryUsage() const {
    if (!m_impl->lua) {
        return 0;
    }
    lua_State* L = m_impl->lua->lua_state();
    int kbytes = lua_gc(L, LUA_GCCOUNT, 0);
    return static_cast<size_t>(kbytes) * 1024;
}

DString LuaEngine::FormatError(const std::string& error) {
    DString result = _T("Lua Error: ");
    result += StringConvert::UTF8ToT(error);
    return result;
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_LUA

