#pragma once

#include <string>
#include <sys/time.h>

#include "blackbox/bb_blackbox.hpp"

namespace blackbox
{

/// @brief Publisher + レコードのクラス
/// @tparam MessageT メッセージ型
/// @tparam IS_ENABLE_RECORD レコードを有効にするかを指定する．（デフォルトはtrue）
template<typename MessageT, bool IS_ENABLE_RECORD=true>
class Record : private BlackBoxWriter<MessageT>
{
public:
    // コピー・ムーブを禁止
    Record(const Record&) = delete;
    Record& operator=(const Record&) = delete;
    Record(Record&&) = delete;
    Record& operator=(Record&&) = delete;

    /// @brief Recordインスタンスを作成するファクトリメソッド
    /// @param bb blackbox::BlackBoxのshared_ptr
    /// @param record_name レコード名（"/record/namespace/record_name"になる）
    /// @param drop_count ドロップ数（0はドロップなし）
    /// @return std::unique_ptr<Record> インスタンス
    static std::shared_ptr<Record> create(std::shared_ptr<BlackBox> bb, std::string record_name, size_t drop_count=0){
        std::shared_ptr<Record> record(new Record());
        record->init(bb, record_name, drop_count);
        return record;
    }

    /// @brief メッセージの送信
    /// @param msg メッセージ
    /// @param tim レコード用のタイムスタンプ
    void record(std::unique_ptr<MessageT> msg, bb_time_t tim = blackbox::get_bb_tim()){
        if(IS_ENABLE_RECORD)
            BlackBoxWriter<MessageT>::write(*msg, tim);
    }

    void record(std::shared_ptr<MessageT> msg, bb_time_t tim = blackbox::get_bb_tim()){
        if(IS_ENABLE_RECORD)
            BlackBoxWriter<MessageT>::write(*msg, tim);
    }

private:
    Record() : BlackBoxWriter<MessageT>(){
    }

    /// @brief 初期化（内部用）
    /// @param bb blackbox::BlackBoxのshared_ptr
    /// @param record_name レコード名（"/record/namespace/record_name"になる）
    /// @param drop_count ドロップ数（0はドロップなし）
    void init(std::shared_ptr<BlackBox> bb, std::string record_name, size_t drop_count=0){
        std::string ns = bb->get_namespace();
        if(ns.size() != 1){
            ns += '/';
        }

        if(IS_ENABLE_RECORD)
            BlackBoxWriter<MessageT>::BlackBoxWriter_cons(bb, "/record" + ns + record_name, drop_count);
    }
};

}
