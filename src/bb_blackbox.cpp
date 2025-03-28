#include "bb_blackbox.hpp"
#include "bb_file.hpp"

#include <google/protobuf/descriptor.pb.h>


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
    google::protobuf::FileDescriptorProto file_proto;
    file_descriptor->CopyTo(&file_proto);

    // FileDescriptorSet を作成して格納
    google::protobuf::FileDescriptorSet file_set;
    file_set.add_file()->CopyFrom(file_proto);

    // 依存しているファイルも追加
    std::set<std::string> processed_files;
    processed_files.insert(file_descriptor->name());

    for (int i = 0; i < file_descriptor->dependency_count(); i++)
    {
        const google::protobuf::FileDescriptor *dependency = file_descriptor->dependency(i);
        if (processed_files.find(dependency->name()) == processed_files.end())
        {
            google::protobuf::FileDescriptorProto dep_proto;
            dependency->CopyTo(&dep_proto);
            file_set.add_file()->CopyFrom(dep_proto);
            processed_files.insert(dependency->name());
        }
    }

    std::string binary_data;
    if (!file_set.SerializeToString(&binary_data))
    {
        throw std::runtime_error("Failed to serialize FileDescriptorSet");
    }

    // std::cout << "Binary data size: " << binary_data.size() << std::endl;
    std::cout << "Binary data" << file_set.DebugString() << std::endl;

    std::vector<std::byte> binary_vector(
        reinterpret_cast<const std::byte *>(binary_data.data()),
        reinterpret_cast<const std::byte *>(binary_data.data()) + binary_data.size());
    return binary_vector;
}


namespace blackbox
{

BlackBox::BlackBox(std::string ns, std::string name, debug_mode_t debug_mode, std::string file_name, storage_profile_t storage_preset_profile, uint64_t max_cache_size) : _bb_debug_mode(debug_mode)
{
    tl_file::init();
    tl_file::create_node(ns, name);
    std::string blackbox_path = tl_file::get_rosbag_path(ns, name, file_name);
    if (blackbox_path == "")
    {
        return;
    }

    _ns = ns;
    _name = name;

    _out_file.open(blackbox_path, std::ios::binary);
    if (!_out_file.is_open())
    {
        return;
    }

    // MCAP のオプション設定
    mcap::McapWriterOptions options("protobuf");
    options.noChunkCRC = true;
    options.compression = mcap::Compression::None;

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
        options.chunkSize = max_cache_size;
        break;
    }

    _writer = std::make_unique<mcap::McapWriter>();
    _writer->open(_out_file, options);
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

            _channel_map[topic_name] = topic.id;aaaaaaaaa
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

}