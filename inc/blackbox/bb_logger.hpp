#pragma once

#include <string>
#include <sys/time.h>
#include "blackbox/bb_blackbox.hpp"
#include "foxglove/Log.pb.h"

#include <stdarg.h>

namespace blackbox
{

/// @brief ログの重要度を指定する
enum log_type_t{
    ERR,            // 常にレコード、Debug modeはSTDOUT
    WARN,           // 常にレコード、Debug modeはSTDOUT
    INFO,           // 常にレコード、 Debug modeはSTDOUT
    DEBUG,          // Debug modeのときだけレコード + STDOUT
    LIB_INFO,       // 常にレコード
    LIB_DEBUG,      // Debug modeのときだけレコード
};

class Logger : BlackBoxWriter<foxglove::Log>
{
public:
    // コピー・ムーブを禁止
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    /// @brief Loggerインスタンスを作成するファクトリメソッド
    /// @param handle blackbox::BlackBoxのshared_ptr
    /// @param log_type ログの重要度
    /// @param tag_name ログのタグ名
    /// @return std::unique_ptr<Logger> インスタンス
    static std::shared_ptr<Logger> create(std::shared_ptr<BlackBox> handle, log_type_t log_type, std::string tag_name){
        std::shared_ptr<Logger> logger(new Logger());
        logger->init(handle, log_type, tag_name);
        return logger;
    }

    static void log(std::shared_ptr<Logger> obj, const char* file, const char* func, size_t line, std::string str)
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


    static void log(std::shared_ptr<Logger> obj, const char* file, const char* func, size_t line, const char* fmt, ...)
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

private:
    Logger(void){}

    std::shared_ptr<BlackBox> _bb = nullptr;
    log_type_t      _log_type;
    std::string     _tag_name;
    bool            _is_enable = true;

    foxglove::Log_Level _foxglove_level;

    /// @brief ログの初期化（内部用）
    /// @param handle blackbox::BlackBoxのshared_ptr
    /// @param log_type ログの重要度
    /// @param tag_name ログのタグ名
    void init(std::shared_ptr<BlackBox> handle, log_type_t log_type, std::string tag_name){
        _bb = handle;
        _log_type = log_type;
        _tag_name = tag_name;

        if(_bb->_bb_debug_mode == debug_mode_t::RELEASE)
        {
            _is_enable = (_log_type != log_type_t::DEBUG);
        }else{
            _is_enable = true;
        }

        switch(_log_type)
        {
        case log_type_t::ERR:
            _foxglove_level = foxglove::Log_Level_ERROR;                
            break;  
        case log_type_t::WARN:
            _foxglove_level = foxglove::Log_Level_WARNING;
            break;
        case log_type_t::INFO:
        case log_type_t::LIB_INFO:
            _foxglove_level = foxglove::Log_Level_INFO;
            break;
        case log_type_t::DEBUG:
        case log_type_t::LIB_DEBUG:
            _foxglove_level = foxglove::Log_Level_DEBUG;
            break;
        default:
            _foxglove_level = foxglove::Log_Level_INFO;
            break;
        }

        std::string ns = _bb->get_namespace();
        if(ns.size() != 1){
            ns += '/';
        }
        this->BlackBoxWriter_cons(_bb, "/tagger" + ns + _bb->get_name(), 0);
    }
};

}

// obj: blackbox::Loggerのインスタンス
// ...: ログメッセージ（fmt or std::string）
#define TAGGER(obj, ...) blackbox::Logger::log(obj, basename(__FILE__), __func__, __LINE__, __VA_ARGS__);