#include "log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>

#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace be {

std::shared_ptr<spdlog::logger> Log::s_logger = nullptr;
bool Log::s_initialized = false;

void Log::init(const std::string& logDir, LogLevel level) {
    if (s_initialized) return;

    namespace fs = std::filesystem;
    
    // 创建日志目录
    fs::path dir(logDir);
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }

    // 生成带时间戳的日志文件名: logs/be_2026-07-14_18-44-33.log
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
    std::string filename = (dir / ("be_" + ss.str() + ".log")).string();

    // 创建 sinks
    std::vector<spdlog::sink_ptr> sinks;

    // 1. 控制台彩色输出（开发调试用）
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("[%^%l%$] %H:%M:%S.%e | %v");
    sinks.push_back(console_sink);

    // 2. 当前会话日志文件（详细记录，用于调试本次运行）
    auto session_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
    session_sink->set_level(spdlog::level::trace);
    session_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %@ | %v");
    sinks.push_back(session_sink);

    // 3. 持久化日志（按天轮转，保留最近 7 天，用于长期问题追踪）
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
        (dir / "be_daily.log").string(),  // 基础文件名
        0, 0,                              // 每天 0:00 切割
        false,                             // 不 truncate
        7                                  // 保留 7 天
    );
    daily_sink->set_level(spdlog::level::info);
    daily_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %s:%# %! | %v");
    sinks.push_back(daily_sink);

    // 创建异步 logger（避免阻塞主线程/GPU 提交）
    // 队列大小 8192，1 个后台刷新线程
    spdlog::init_thread_pool(8192, 1);
    s_logger = std::make_shared<spdlog::async_logger>(
        "BrushEngine",
        sinks.begin(), sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block
    );

    // 设置全局级别
    s_logger->set_level(static_cast<spdlog::level::level_enum>(level));
    
    // 遇到 Error 及以上立即 flush（确保崩溃前日志落盘）
    s_logger->flush_on(spdlog::level::err);
    
    // 注册到全局，方便 spdlog::get("BrushEngine") 获取
    spdlog::register_logger(s_logger);

    s_initialized = true;

    BE_LOG_INFO("========================================");
    BE_LOG_INFO("BrushEngine Log System Initialized");
    BE_LOG_INFO("Session log: {}", filename);
    BE_LOG_INFO("Log level: {}", spdlog::level::to_string_view(s_logger->level()));
    BE_LOG_INFO("========================================");
}

void Log::shutdown() {
    if (!s_initialized) return;
    
    BE_LOG_INFO("BrushEngine Log System shutting down...");
    s_logger->flush();
    spdlog::drop("BrushEngine");
    s_logger.reset();
    s_initialized = false;
}

void Log::setLevel(LogLevel level) {
    if (s_logger) {
        s_logger->set_level(static_cast<spdlog::level::level_enum>(level));
    }
}

LogLevel Log::getLevel() {
    if (!s_logger) return LogLevel::Info;
    return static_cast<LogLevel>(s_logger->level());
}

std::shared_ptr<spdlog::logger>& Log::core() {
    return s_logger;
}

bool Log::isInitialized() {
    return s_initialized;
}

} // namespace be