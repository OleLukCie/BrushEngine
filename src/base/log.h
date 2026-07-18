#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace be {

enum class LogLevel {
    Trace = SPDLOG_LEVEL_TRACE,     // 最详细，调试用
    Debug = SPDLOG_LEVEL_DEBUG,     // 开发日常
    Info  = SPDLOG_LEVEL_INFO,      // 正常运行信息
    Warn  = SPDLOG_LEVEL_WARN,      // 警告，不影响功能
    Error = SPDLOG_LEVEL_ERROR,     // 错误，功能受损
    Fatal = SPDLOG_LEVEL_CRITICAL   // 致命，即将崩溃
};

class Log {
public:
    // 初始化日志系统，应在 engine_init 中最早调用
    // logDir: 日志存放目录，默认 "logs"
    // level:  初始日志级别
    static void init(const std::string& logDir = "logs", LogLevel level = LogLevel::Debug);
    
    // 关闭日志系统，刷新所有缓冲区
    static void shutdown();
    
    // 动态调整日志级别（可在运行时通过 UI 或快捷键切换）
    static void setLevel(LogLevel level);
    static LogLevel getLevel();
    
    // 获取核心 logger，一般不需要直接调用，用下面的宏即可
    static std::shared_ptr<spdlog::logger>& core();
    
    // 是否已初始化
    static bool isInitialized();

private:
    static std::shared_ptr<spdlog::logger> s_logger;
    static bool s_initialized;
};

} // namespace be

// ===== 便捷宏 =====
// 带源文件位置信息 [file:line function]
#define BE_LOG_TRACE(...) SPDLOG_LOGGER_TRACE(::be::Log::core(), __VA_ARGS__)
#define BE_LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(::be::Log::core(), __VA_ARGS__)
#define BE_LOG_INFO(...)  SPDLOG_LOGGER_INFO (::be::Log::core(), __VA_ARGS__)
#define BE_LOG_WARN(...)  SPDLOG_LOGGER_WARN (::be::Log::core(), __VA_ARGS__)
#define BE_LOG_ERROR(...) SPDLOG_LOGGER_ERROR(::be::Log::core(), __VA_ARGS__)
#define BE_LOG_FATAL(...) SPDLOG_LOGGER_CRITICAL(::be::Log::core(), __VA_ARGS__)

// 带返回值的错误检查宏
#define BE_CHECK(cond, ...) do { \
    if (!(cond)) { \
        BE_LOG_ERROR("CHECK failed: {} | {}", #cond, fmt::format(__VA_ARGS__)); \
    } \
} while(0)

#define BE_CHECK_RET(cond, ret, ...) do { \
    if (!(cond)) { \
        BE_LOG_FATAL("CHECK_RET failed: {} | {}", #cond, fmt::format(__VA_ARGS__)); \
        return ret; \
    } \
} while(0)

#define BE_CHECK_FATAL(cond, ...) do { \
    if (!(cond)) { \
        BE_LOG_FATAL("FATAL: {} | {}", #cond, fmt::format(__VA_ARGS__)); \
        ::be::Log::shutdown(); \
        std::abort(); \
    } \
} while(0)