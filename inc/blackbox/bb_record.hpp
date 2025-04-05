#pragma once

#include <string>
#include <sys/time.h>

#include "blackbox/bb_blackbox.hpp"
#include "blackbox/bb_node.hpp"

namespace blackbox
{

/// @brief Publisher + レコードのクラス
/// @tparam MessageT メッセージ型
/// @tparam IS_ENABLE_RECORD レコードを有効にするかを指定する．（デフォルトはtrue）
template<typename MessageT, bool IS_ENABLE_RECORD=true>
class Record : private BlackBoxWriter<MessageT>
{
public:
    Record() : BlackBoxWriter<MessageT>(){
    }

    /// @brief 初期化
    /// @param bb blackbox::BlackBoxのポインタ
    /// @param record_name レコード名（"/record/namespace/record_name"になる）
    /// @param qos QoS
    /// @param drop_count ドロップ数（0はドロップなし）
    void init(BlackBox* bb, std::string record_name, size_t drop_count=0){
        std::string ns = bb->get_namespace();
        if(ns.size() != 1){
            ns += '/';
        }

        if(IS_ENABLE_RECORD)
            BlackBoxWriter<MessageT>::BlackBoxWriter_cons(bb, "/record" + ns + record_name, drop_count);
    }

    /// @brief メッセージの送信
    /// @param msg メッセージ
    void record(std::unique_ptr<MessageT> msg){
            this->record(std::move(msg), blackbox::get_bb_tim());
    }

    /// @brief メッセージの送信
    /// @param msg メッセージ
    /// @param tim レコード用のタイムスタンプ
    void record(std::unique_ptr<MessageT> msg, bb_time_t tim)
    {
        if(IS_ENABLE_RECORD)
            BlackBoxWriter<MessageT>::write(std::move(msg), tim);
    }
};

}
