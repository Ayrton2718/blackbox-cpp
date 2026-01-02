#include "blackbox/bb_logger.hpp"

#include <stdarg.h>

namespace blackbox
{

void Logger::log(std::shared_ptr<Logger> obj, const char* file, const char* func, size_t line, std::string str)
{
    if(obj == nullptr || obj->_bb == nullptr || obj->_is_enable == false)
    {
        return;
    }

    auto now = get_bb_tim();

    std::unique_ptr<foxglove::Log> msg = std::make_unique<foxglove::Log>();
    auto stamp = msg->mutable_timestamp();
    stamp->set_seconds(now.tv_sec);
    stamp->set_nanos(now.tv_nsec);
    msg->set_name(obj->_tag_name);
    msg->set_level(obj->_foxglove_level);
    msg->set_file(std::string(file) + "." + func);
    msg->set_line(line);
    msg->set_message(str);
    obj->write(msg.get(), now);

    if(obj->_bb->_bb_debug_mode == debug_mode_t::DEBUG)
    {
        switch(obj->_log_type)
        {
        case log_type_t::ERR:
            std::cout << "[ERROR] " << str << std::endl;
            break;

        case log_type_t::WARN:
            std::cout << "[WARN] " << str << std::endl;
            break;

        case log_type_t::INFO:
            std::cout << "[INFO] " << str << std::endl;
            break;

        case log_type_t::DEBUG:
            std::cout << "[DEBUG] " << str << std::endl;
            break;

        default:
            break;
        }
    }
}


void Logger::log(std::shared_ptr<Logger> obj, const char* file, const char* func, size_t line, const char* fmt, ...)
{
    const int MAX_LOG_SIZE = 1024;
    
    std::string str;
    va_list ap;
    va_list ap_copy;
    va_copy(ap_copy, ap);

    va_start(ap, fmt);
    str.resize(MAX_LOG_SIZE);
    int len = vsnprintf(str.data(), str.size(), fmt, ap);
    va_end(ap);

    if(len < MAX_LOG_SIZE)
    {
        str.resize(len + 1);  // need space for NUL
    }else{
        str.resize(len + 1);  // need space for NUL

        va_start(ap_copy, fmt);
        vsnprintf(str.data(), str.size(), fmt, ap_copy);
        va_end(ap_copy);
    }

    Logger::log(obj, file, func, line, str);
}

}


