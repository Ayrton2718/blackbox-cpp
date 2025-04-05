#pragma once

#include <string>
#include <sys/time.h>
#include <memory>
#include <chrono>
#include <vector>

#include <mcap/mcap.hpp>
#include "bb_time.hpp"

namespace blackbox
{

// mcapファイルの圧縮設定
enum class storage_profile_t {
    fastwrite,  // Configures the MCAP writer for the highest possible write throughput and lowest resource utilization. This preset does not calculate CRCs for integrity checking, and does not write a message index. Useful for resource-constrained robots.
    zstd_fast,  // Configures the MCAP writer to use chunk compression with zstd at the lowest compression ratio and disables CRC calculation, to achieve high throughput while conserving disk space.
    zstd_small  // Configures the MCAP writer to write 4MB chunks, compressed with zstd using its highest compression ratio. Calculates chunk CRCs, useful when using ros2 bag convert as a post-processing step.
};

/// @brief デバッグモードを指定する
enum class debug_mode_t{
    RELEASE,
    DEBUG,
};


class BlackBox
{
public:
    // メッセージの書き込みを行うクラス（blackbox::Loggerやblackbox::Recordなどで使用）
    template<typename MessageT>
    class BlackBoxWriter
    {
    public:
        BlackBoxWriter(){}

        void BlackBoxWriter_cons(BlackBox* handle, std::string topic_name, size_t drop_count=0)
        {
            auto res = handle->create(topic_name, MessageT::descriptor());
            if(res.first)
            {
                _handle = handle;
                _channel_id = res.second;
                _topic_name = topic_name;
                _counter = 0;
                _drop_count = (drop_count + 1);
            }
        }


        void write(std::unique_ptr<MessageT> msg, bb_time_t tim)
        {
            if(msg != NULL && _handle != NULL){
                if((_counter % _drop_count) == 0)
                {
                    // メッセージのシリアライズ
                    std::vector<std::byte> payload(msg->ByteSizeLong()); // uint8_t を使用
                    msg->SerializeToArray(static_cast<void*>(payload.data()), payload.size());
                    
                    mcap::Message mcap_msg;
                    mcap_msg.channelId = _channel_id;
                    mcap_msg.sequence = 0;
                    mcap_msg.publishTime = this->timespec_to_timestamp(tim);
                    mcap_msg.logTime = this->timespec_to_timestamp(get_bb_tim());
                    mcap_msg.data = payload.data(); // 修正: キャスト
                    mcap_msg.dataSize = payload.size();

                    _handle->write(mcap_msg);
                }
                msg.reset();
            }
            _counter++;
        }

    private:
        BlackBox*       _handle = nullptr;
        std::string     _topic_name;

        mcap::ChannelId _channel_id = 0;

        size_t      _counter;
        size_t      _drop_count;


        mcap::Timestamp timespec_to_timestamp(bb_time_t ts) {
            return mcap::Timestamp((uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec);
        }

    };


public:
    const debug_mode_t    _bb_debug_mode;

    /// @brief 
    /// @tparam MessageT 
    /// @param node rclcpp::Nodeのポインタ
    /// @param storage_preset_profile 
    /// @param max_cache_size


    /// @brief 
    /// @param node rclcpp::Nodeのポインタ
    /// @param debug_mode デバッグモードを指定する．主にログをコンソールに出力するかどうかを指定する．
    /// @param file_name ファイル名を指定する．（デフォルトは"blackbox"）
    /// @param storage_preset_profile mcapのstorage　profileを指定する．（デフォルトはstorage_profile_t::zstd_fast）
    /// @param max_cache_size キャッシュサイズを指定する．レコードするサイズによって最適な変更する．（デフォルトは1024*128）
    BlackBox(std::string ns, std::string name, debug_mode_t debug_mode, std::string file_name="blackbox", storage_profile_t storage_preset_profile=storage_profile_t::zstd_fast, uint64_t max_cache_size=1024*128);

    virtual ~BlackBox() noexcept
    {
        if(_writer != NULL)
        {
            _writer->close();
        }
    }

    std::string get_namespace(void)
    {
        return _ns;
    }

    std::string get_name(void)
    {
        return _name;
    }

private:
    std::string _ns;
    std::string _name;

    std::unordered_map<std::string, mcap::SchemaId> _schema_map;
    std::unordered_map<std::string, mcap::ChannelId> _channel_map;

    std::ofstream _out_file;
    std::unique_ptr<mcap::McapWriter> _writer = NULL;

    size_t _err_count = 0;


    std::pair<bool, mcap::ChannelId> create(std::string topic_name, const google::protobuf::Descriptor* descriptor);

    void write(mcap::Message msg)
    {
        if(_writer != NULL)
        {
            auto res = _writer->write(msg);
            if(!res.ok()){
                // エラー処理
                if(_err_count % 10)
                {                
                    std::cerr << "Error: " << res.message << std::endl;
                }
                _err_count++;
            }
        }
    }
};

template<typename MessageT>
using BlackBoxWriter = BlackBox::BlackBoxWriter<MessageT>;

}