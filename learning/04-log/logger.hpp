#pragma once

#include <fstream>

#include "lockfree.hpp"
#include "thread.hpp"
#include "time.hpp"

namespace common {

constexpr size_t LOGGER_QUEUE_SIZE = 8 * 1024 * 1024;

enum class LogType : int8_t
{
    CHAR = 0,
    INT = 1,
    LONG = 2,
    LLONG = 3,
    UINT = 4,
    ULONG = 5,
    ULLONG = 6,
    FLOAT = 7,
    DOUBLE = 8,
};

struct LogEntry
{
    LogType type_;
    union {
        char c;
        int i;
        long l;
        long long ll;
        unsigned int ui;
        unsigned long ul;
        unsigned long long ull;
        float f;
        double d;
    } data_;
};

class Logger final
{
    const std::string filename_;
    Spsc<LogEntry> queue_;
    std::thread *thread_;
    std::ofstream file_;
    std::atomic<bool> running_;

public:
    explicit Logger(const std::string& filename)
    : filename_{filename}, queue_{LOGGER_QUEUE_SIZE}
    , running_{true}
    {
        std::string start_time {};
        std::cerr << get_now_str(&start_time) << " starting to init logger " << filename << std::endl;

        file_.open(filename);
        ASSERT(file_.is_open(), "cannot open file " + filename);
        thread_ = create_and_start_thread(-1, "common::logger::" + filename, [this]{flush_queue();});
        ASSERT(thread_, "failed to start logger thread");

        std::cerr << get_now_str(&start_time) << " logger " << filename << " initiated" << std::endl;
    }

    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    ~Logger();

    template<typename... A>
    void log(const char* str, A&&... args)
    {
        // could be optimized with memcpy
        while (*str) {
            if (*str == '%') {
                [[unlikely]] if (*(str + 1) == '%') {
                    ++str;
                } else {
                    if constexpr (sizeof...(args) > 0) {
                        [this](const char* s, auto&& val, auto&&... others) 
                        {
                            push(val);
                            log(s + 1, std::forward<decltype(others)>(others)...);
                        }(str, std::forward<A>(args)...);
                        return;
                    } else {
                        FATAL("missing args to log: " + std::string(str));
                    }
                }
            }
            push(*str);
            ++str;
        }
    }

    template<typename... A>
    void log(const std::string& str, A&&... args)
    {
        log(str.c_str(), std::forward<A>(args)...);
    }

private:

    void flush_queue() noexcept;

    void push(const LogEntry& e) noexcept
    {
        (*queue_.get_next_set()) = e;
        queue_.advance_set();
    }

    void push(const char x) noexcept { push({.type_=LogType::CHAR, .data_={.c=x}}); }
    void push(const int x) noexcept { push({.type_=LogType::INT, .data_={.i=x}}); }
    void push(const long x) noexcept { push({.type_=LogType::LONG, .data_={.l=x}}); }
    void push(const long long x) noexcept { push({.type_=LogType::LLONG, .data_={.ll=x}}); }
    void push(const unsigned int x) noexcept { push({.type_=LogType::INT, .data_={.ui=x}}); }
    void push(const unsigned long x) noexcept { push({.type_=LogType::LONG, .data_={.ul=x}}); }
    void push(const unsigned long long x) noexcept { push({.type_=LogType::LLONG, .data_={.ull=x}}); }
    void push(const float x) noexcept { push({.type_=LogType::FLOAT, .data_={.f=x}}); }
    void push(const double x) noexcept { push({.type_=LogType::DOUBLE, .data_={.d=x}}); }
};

Logger::~Logger()
{
    std::string close_time {};
    std::cerr << get_now_str(&close_time) << " flushing and closing logger " << filename_ << std::endl;

    while (queue_.size()) {
        std::this_thread::sleep_for(1s);
    }
    running_ = false;
    thread_->join();
    file_.close();
    std::cerr << get_now_str(&close_time) << " logger " << filename_ << " exiting" << std::endl;
}

void Logger::flush_queue() noexcept
{
    while (running_) {
        for (auto get = queue_.get_next_get(); get; get = queue_.get_next_get()) {
            switch (get->type_) {
                case LogType::CHAR:
                    file_ << get->data_.c; break;
                case LogType::INT:
                    file_ << get->data_.i; break;
                case LogType::LONG:
                    file_ << get->data_.l; break;
                case LogType::LLONG:
                    file_ << get->data_.ll; break;
                case LogType::UINT:
                    file_ << get->data_.ui; break;
                case LogType::ULONG:
                    file_ << get->data_.ul; break;
                case LogType::ULLONG:
                    file_ << get->data_.ull; break;
                case LogType::FLOAT:
                    file_ << get->data_.f; break;
                case LogType::DOUBLE:
                    file_ << get->data_.d; break;
            }
            queue_.advance_get();
        }
        file_.flush();
        std::this_thread::sleep_for(10ms);
    }
}

} // namespace common