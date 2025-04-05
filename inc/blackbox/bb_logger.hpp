#pragma once

#include <string>
#include <sys/time.h>
#include "blackbox/bb_blackbox.hpp"
#include "foxglove/Log.pb.h"

namespace blackbox
{

/// @brief ログの重要度を指定する
enum log_type_t{
    ERR,            // 常にレコード、Debug modeはSTDOUT
    WARN,            // 常にレコード、Debug modeはSTDOUT
    INFO,            // 常にレコード、 Debug modeはSTDOUT
    DEBUG,          // Debug modeのときだけレコード + STDOUT
    LIB_INFO,      // 常にレコード
    LIB_DEBUG     // Debug modeのときだけレコード
};

class Logger : BlackBoxWriter<foxglove::Log>
{
private:
    BlackBox*       _bb = nullptr;
    log_type_t      _log_type;
    std::string     _tag_name;
    bool            _is_enable = true;

    foxglove::Log_Level _foxglove_level;

public:
    Logger(void){}

    /// @brief ログの初期化
    /// @param handle blackbox::BlackBoxNodeのポインタ（blackbox::LogRecorderのポインタ）
    /// @param log_type ログの重要度
    /// @param tag_name ログのタグ名
    void init(BlackBox* handle, log_type_t log_type, std::string tag_name){
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


    static void log(Logger* obj, const char* file, const char* func, size_t line, std::string str);
    static void log(Logger* obj, const char* file, const char* func, size_t line, const char* fmt, ...);

    static bool is_enable(Logger* obj){
        return (obj != NULL && obj->_is_enable && obj->_bb != NULL);
    }

    static bool is_enable(Logger& obj){
        return is_enable(&obj);
    }
};

}

// obj: blackbox::Loggerのインスタンス
// ...: ログメッセージ（fmt or std::string）
#define TAGGER(obj, ...) if(blackbox::Logger::is_enable(obj)){blackbox::Logger::log(obj, basename(__FILE__), __func__, __LINE__, __VA_ARGS__);}