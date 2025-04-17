#include "blackbox/bb_blackbox.hpp"

#include <csignal>
#include <google/protobuf/descriptor.pb.h>
#include "blackbox/bb_file.hpp"

namespace blackbox
{

std::mutex BlackBox::_sig_mutex;
std::vector<std::shared_ptr<mcap::McapWriter>> BlackBox::_sig_queue;


static void ProcessDependencies(const google::protobuf::FileDescriptor* file_descriptor,
    google::protobuf::FileDescriptorSet* file_set,
    std::set<std::string>& processed_files) 
{
    if (!file_descriptor) return;
    if (processed_files.count(file_descriptor->name())) return;

    processed_files.insert(file_descriptor->name());

    google::protobuf::FileDescriptorProto file_proto;
    file_descriptor->CopyTo(&file_proto);
    file_set->add_file()->CopyFrom(file_proto);

    for (int i = 0; i < file_descriptor->dependency_count(); ++i) {
        ProcessDependencies(file_descriptor->dependency(i), file_set, processed_files);
    }
}

static std::vector<std::byte> GenerateDescriptorBinary(const google::protobuf::Descriptor *descriptor)
{
    if (!descriptor)
    {
        throw std::runtime_error("Descriptor is null");
    }

    // FileDescriptor を取得
    const google::protobuf::FileDescriptor *file_descriptor = descriptor->file();
    if (!file_descriptor)
    {
        throw std::runtime_error("Failed to get FileDescriptor");
    }

    // FileDescriptorProto に変換
    google::protobuf::FileDescriptorSet file_set;
    std::set<std::string> processed_files;

    ProcessDependencies(file_descriptor, &file_set, processed_files);

    std::string binary_data;
    if (!file_set.SerializeToString(&binary_data))
    {
        throw std::runtime_error("Failed to serialize FileDescriptorSet");
    }

#ifdef BLACKBOX_SCHEMA_DEBUG
    // デバッグ用にスキーマを表示
    std::cout << "=============" << descriptor->full_name() << "===========" << std::endl;
    std::cout << "Schema: " << file_set.DebugString() << std::endl;
    std::cout << "===================================" << std::endl;
#endif

    std::vector<std::byte> binary_vector(
        reinterpret_cast<const std::byte *>(binary_data.data()),
        reinterpret_cast<const std::byte *>(binary_data.data()) + binary_data.size());
    return binary_vector;
}


BlackBox::BlackBox(std::string ns, std::string name, debug_mode_t debug_mode, std::string file_name, storage_profile_t storage_preset_profile, uint64_t max_cache_size) : _bb_debug_mode(debug_mode)
{
    if (!ns.empty() && ns[0] != '/') {
        ns = "/" + ns; // 先頭にスラッシュを追加
    }
    
    tl_file::init();
    tl_file::create_node(ns, name);
    std::string blackbox_path = tl_file::get_rosbag_path(ns, name, file_name);
    if (blackbox_path == "")
    {
        return;
    }

    _ns = ns;
    _name = name;

    blackbox_path = blackbox_path + ".mcap";
    _out_file.open(blackbox_path, std::ios::binary);
    if (!_out_file.is_open())
    {
        return;
    }

    // MCAP のオプション設定
    mcap::McapWriterOptions options("protobuf");
    options.noChunkCRC = true;
    options.compression = mcap::Compression::None;
    options.chunkSize = max_cache_size;

    switch (storage_preset_profile)
    {
    case storage_profile_t::fastwrite:
        options.noChunking = true;
        options.noSummaryCRC = true;
        break;
    case storage_profile_t::zstd_fast:
        options.compression = mcap::Compression::Zstd;
        options.compressionLevel = mcap::CompressionLevel::Fastest;
        options.noChunkCRC = true;
        break;
    case storage_profile_t::zstd_small:
        options.compression = mcap::Compression::Zstd;
        options.compressionLevel = mcap::CompressionLevel::Slowest;
        break;
    }

    _writer = std::make_shared<mcap::McapWriter>();
    _writer->open(_out_file, options);

    std::cout << "Crate BlackBox bag file: " << blackbox_path << std::endl;

    std::lock_guard<std::mutex> lock(_sig_mutex);
    _sig_queue.push_back(_writer);
    signal(SIGINT, BlackBox::handler);
}


std::pair<bool, mcap::ChannelId> BlackBox::create(std::string topic_name, const google::protobuf::Descriptor *descriptor)
{
    if (_writer != NULL)
    {
        std::string schema_name = descriptor->full_name();

        mcap::SchemaId schema_id = 0;
        if(_schema_map.find(schema_name) == _schema_map.end())
        {
            // Protobuf スキーマのバイナリ (`FileDescriptorSet`)
            std::vector<std::byte> descriptorData = GenerateDescriptorBinary(descriptor);

            mcap::Schema myStringSchema(
                descriptor->full_name(), // 完全修飾名
                "protobuf",              // エンコーディング
                descriptorData           // バイナリデータをそのまま渡す
            );
            _writer->addSchema(myStringSchema);

            _schema_map[schema_name] = myStringSchema.id;
            schema_id = myStringSchema.id;
        }
        else
        {
            schema_id = _schema_map[schema_name];
        }

        mcap::ChannelId channel_id = 0;
        if(_channel_map.find(topic_name) == _channel_map.end())
        {
            mcap::Channel topic(topic_name, "protobuf", schema_id);
            _writer->addChannel(topic);

            _channel_map[topic_name] = topic.id;
            channel_id = topic.id;
        }
        else
        {
            channel_id = _channel_map[topic_name];
        }

        return std::make_pair(true, channel_id);
    }
    return std::make_pair(false, 0);
}

void BlackBox::handler(int sig)
{
    std::cerr << "SIGINT received, exiting..." << std::endl;

    for (auto &sig_instance : _sig_queue)
    {
        if (sig_instance != nullptr)
        {
            std::cerr << "Closing blackbox bag file..." << std::endl;
            sig_instance->close();
            sig_instance->terminate();
            sig_instance.reset();
        }
    }

    _sig_queue.clear();

    exit(sig);
}

}