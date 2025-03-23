#define MCAP_IMPLEMENTATION
#include <mcap/writer.hpp>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <google/protobuf/descriptor.pb.h>
#include "foxglove/Log.pb.h"

// 現在のタイムスタンプを取得
mcap::Timestamp now() {
  return mcap::Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(
                             std::chrono::system_clock::now().time_since_epoch())
                             .count());
}

std::vector<std::byte> GenerateDescriptorBinary(const google::protobuf::Descriptor* descriptor) {
    if (!descriptor) {
        throw std::runtime_error("Descriptor is null");
    }
  
    // FileDescriptor を取得
    const google::protobuf::FileDescriptor* file_descriptor = descriptor->file();
    if (!file_descriptor) {
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
  
    for (int i = 0; i < file_descriptor->dependency_count(); i++) {
        const google::protobuf::FileDescriptor* dependency = file_descriptor->dependency(i);
        if (processed_files.find(dependency->name()) == processed_files.end()) {
            google::protobuf::FileDescriptorProto dep_proto;
            dependency->CopyTo(&dep_proto);
            file_set.add_file()->CopyFrom(dep_proto);
            processed_files.insert(dependency->name());
        }
    }
  
    std::string binary_data;
    if (!file_set.SerializeToString(&binary_data)) {
        throw std::runtime_error("Failed to serialize FileDescriptorSet");
    }

    // std::cout << "Binary data size: " << binary_data.size() << std::endl;
    std::cout << "Binary data" << file_set.DebugString() << std::endl;
  
    std::vector<std::byte> binary_vector(
        reinterpret_cast<const std::byte*>(binary_data.data()),
        reinterpret_cast<const std::byte*>(binary_data.data()) + binary_data.size()
    );
    return binary_vector;
  }
  
int main() {
    mcap::McapWriter writer;

    // MCAP のオプション設定
    auto options = mcap::McapWriterOptions("protobuf");
    options.compression = mcap::Compression::Zstd; // Zstd 圧縮を使用

    std::ofstream out("output.mcap", std::ios::binary);
    writer.open(out, options);

    // Protobuf スキーマのバイナリ (`FileDescriptorSet`)
    std::vector<std::byte> descriptorData = GenerateDescriptorBinary(foxglove::Log::descriptor());

    // MCAP にスキーマを登録
    mcap::Schema myStringSchema(
        "foxglove.Log",  // 完全修飾名
        "protobuf",          // エンコーディング
        descriptorData       // バイナリデータをそのまま渡す
    );
    writer.addSchema(myStringSchema);

    // チャンネルの作成
    mcap::Channel topic("/chatter", "protobuf", myStringSchema.id);
    writer.addChannel(topic);

    // Protobuf メッセージの作成
    foxglove::Log message;
    message.set_level(foxglove::Log::INFO);
    message.set_name("example");
    message.set_message("Hello, world!");
    auto stamp = new google::protobuf::Timestamp();
    stamp->set_seconds(1234567890);
    stamp->set_nanos(123456789);
    message.set_allocated_timestamp(stamp);
    message.set_file("example.cpp");
    message.set_line(42);
    

    // メッセージのシリアライズ
    std::vector<std::byte> payload(message.ByteSizeLong()); // uint8_t を使用
    message.SerializeToArray(static_cast<void*>(payload.data()), payload.size());

    // MCAP メッセージを作成して書き込み
    mcap::Message msg;
    msg.channelId = topic.id;
    msg.sequence = 0;
    msg.publishTime = now();
    msg.logTime = msg.publishTime;
    msg.data = payload.data(); // 修正: キャスト
    msg.dataSize = payload.size();

    const auto res = writer.write(msg);
    if (!res.ok()) {
        std::cerr << "Failed to write message: " << res.message << "\n";
        writer.terminate();
        out.close();
        std::ignore = std::remove("output.mcap");
        return 1;
    }

    writer.close();

    std::cout << "Successfully wrote Protobuf message to output.mcap\n";
    return 0;
}
