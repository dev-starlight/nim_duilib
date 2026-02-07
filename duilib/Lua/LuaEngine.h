#ifndef UI_LUA_LUA_ENGINE_H_
#define UI_LUA_LUA_ENGINE_H_

#ifdef DUILIB_BUILD_FOR_LUA

#include "duilib/duilib_defs.h"
#include <memory>
#include <functional>
#include <string>

// Forward declaration to avoid exposing sol3 in headers
namespace sol { class state; }
struct lua_State;

namespace ui
{

class Window;
class Control;

/** Lua script engine
 *  Manages Lua VM lifecycle, script loading and global binding registration.
 *  Each Window can have its own Lua environment sharing the same VM.
 */
class UILIB_API LuaEngine
{
public:
    /** Get global singleton instance */
    static LuaEngine& Instance();

    /** Initialize Lua engine
     *  @param [in] scriptRootPath Root directory path for scripts
     *  @return true if initialization successful
     */
    bool Initialize(const DString& scriptRootPath);

    /** Shutdown Lua engine and release all resources */
    void Shutdown();

    /** Check if initialized */
    bool IsInitialized() const;

    /** Load and execute Lua script file
     *  @param [in] scriptPath Script file path (relative to script root)
     *  @return true if loaded and executed successfully
     */
    bool LoadScript(const DString& scriptPath);

    /** Execute Lua code string
     *  @param [in] luaCode Lua code string
     *  @return true if executed successfully
     */
    bool DoString(const DString& luaCode);

    /** Reload specified script (hot reload)
     *  @param [in] scriptPath Script file path
     *  @return true if reload successful
     */
    bool ReloadScript(const DString& scriptPath);

    /** Reload all loaded scripts */
    void ReloadAll();

    /** Create independent Lua environment for window
     *  @param [in] pWindow Target window
     *  @return true if created successfully
     */
    bool CreateWindowEnvironment(Window* pWindow);

    /** Destroy window's Lua environment
     *  @param [in] pWindow Target window
     */
    void DestroyWindowEnvironment(Window* pWindow);

    /** Call Lua global function
     *  @param [in] funcName Function name
     *  @param [in] args Arguments (variadic template)
     */
    template<typename... Args>
    void CallFunction(const std::string& funcName, Args&&... args);

    /** Call Lua function in window environment
     *  @param [in] pWindow Target window
     *  @param [in] funcName Function name
     */
    template<typename... Args>
    void CallWindowFunction(Window* pWindow, const std::string& funcName, Args&&... args);

    /** Register C++ function to Lua global namespace
     *  @param [in] name Function name
     *  @param [in] func Function object
     */
    template<typename Func>
    void RegisterFunction(const std::string& name, Func&& func);

    /** Register C++ module to Lua
     *  @param [in] moduleName Module name
     *  @param [in] registerFunc Registration callback
     */
    void RegisterModule(const std::string& moduleName,
                        std::function<void(sol::state&)> registerFunc);

    /** Get underlying sol::state (advanced usage) */
    sol::state& GetState();

    /** Set script execution timeout in milliseconds (default: 5000ms) */
    void SetTimeout(int timeoutMs);

    /** Get current timeout setting */
    int GetTimeout() const;

    /** Set Lua memory limit in bytes (default: 64MB) */
    void SetMemoryLimit(size_t bytes);

    /** Get current memory limit */
    size_t GetMemoryLimit() const;

    /** Get current Lua memory usage in bytes */
    size_t GetMemoryUsage() const;

    /** Format Lua error with stack trace */
    static DString FormatError(const std::string& error);

private:
    LuaEngine();
    ~LuaEngine();
    LuaEngine(const LuaEngine&) = delete;
    LuaEngine& operator=(const LuaEngine&) = delete;

    /** Register all built-in bindings */
    void RegisterAllBindings();

    /** Set up script search paths */
    void SetupSearchPaths(const DString& scriptRootPath);

    /** Set up security sandbox */
    void SetupSandbox();

    /** Error handler */
    static int LuaErrorHandler(lua_State* L);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_LUA

#endif // UI_LUA_LUA_ENGINE_H_
