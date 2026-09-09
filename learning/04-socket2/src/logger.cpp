#include "logger.hpp"

namespace common {

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