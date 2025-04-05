# BlackBox-Cpp

**BlackBox-Cpp** は、もともと ROS2 用に開発されたロギングライブラリ [BlackBox](https://github.com/Ayrton2718/blackbox.git) を ROS2 に依存しない形に改良したものです。  
このライブラリを使うことで、ROS2 を使用しなくても **Mcap-Protobuf 形式** の rosbag ファイルを作成できます。

BlackBox-Cpp を活用すれば、さまざまな種類のログをひとつの rosbag ファイルにまとめて保存できます。たとえば：
- 実行時のコンソールログ
- アプリケーションが生成する独自メッセージ
- 可視化のためのログデータ

作成した rosbag ファイルは、クロスプラットフォーム可視化ツールである [Foxglove Studio](https://foxglove.dev/) を使って解析・可視化できます。

[example.cpp](example/example.cpp)
![blackbox-cpp_foxglove_sample](docs/blackbox-cpp_foxglove_sample.webp)

## Foxglove Studio について

**Foxglove Studio** は、`.mcap` ファイルを直接開いて解析できる多機能ツールです。  
**Windows**、**Mac**、さらにスマートフォンの **Web ブラウザ** からも利用できます。

また、Foxglove Studio は日本語に対応しており、設定メニューから言語を日本語に変更できます。  
さらに、再生時のフレームレートも設定画面で調整可能です。

詳しくは以下のリンクからアプリケーションをダウンロードしてください：  
[Foxglove Studio のインストール](https://foxglove.dev/download)

## How To Install
```bash
sudo apt install -y ansible
```

ros2ワークスペースのsrc内
```bash
git clone https://github.com/Ayrton2718/blackbox.git
cd blackbox
ansible-playbook --ask-become-pass ansible/dev.yml
```

## How To Use
このライブラリは、`BlackBox` のインスタンスを作成して使用します。  
サンプルでは、次のように `BlackBox` を生成しています。

```cpp
// BlackBoxNode インスタンス作成
blackbox::BlackBox bb_node("namespace", "node_name", blackbox::debug_mode_t::DEBUG);
```

### Logger
実行時のメッセージは Logger を使って記録します。  
ログには、レベル（重要度）とタグを設定できます。  
記録されたログは `/tagger/namespace/node_name` に格納され、**Foxglove Studio の Log パネル**で確認できます。

[Foxglove Log パネルのドキュメント](https://docs.foxglove.dev/docs/visualization/panels/log)

例：

```cpp
blackbox::Logger info;
info.init(&bb_node, blackbox::log_type_t::INFO, "position");

blackbox::Logger error;
error.init(&bb_node, blackbox::log_type_t::ERR, "over_position");

TAGGER(&info, "In box: Position (%.2f, %.2f, %.2f)", x, y, z);
TAGGER(&error, "Out of box: Position (%.2f, %.2f, %.2f)", x, y, z);
```

> **ポイント：** `BlackBox` のデバッグモードを設定することで、コンソール出力の有無を切り替えることができます。

```cpp
// デバッグモード：コンソール出力 + 記録
blackbox::BlackBox(blackbox::debug_mode_t::DEBUG, "node_name", "namespace");

// リリースモード：記録のみ
blackbox::BlackBox(blackbox::debug_mode_t::RELEASE, "node_name", "namespace");
```

ログレベル：
```cpp
enum log_type_t{
    ERR,            // 常にレコード、Debug modeはSTDOUT
    WARN,            // 常にレコード、Debug modeはSTDOUT
    INFO,            // 常にレコード、 Debug modeはSTDOUT
    DEBUG,          // Debug modeのときだけレコード + STDOUT
};
```

### Recorder

数値データなどのメッセージは **レコーダー** を使って記録します。  
記録したデータは **Foxglove Studio の Plot パネル** で可視化したり、`.mcap` ファイルとして保存し後から解析することができます。

データは `/record/namespace/node_name` に保存されます。

例：

```cpp
blackbox::Record<foxglove::FrameTransform> frame_record;
frame_record.init(&bb_node, "tf");

auto frame_msg = create_frame_transform_msg(x, y, z);
frame_record.record(std::move(frame_msg));
```

> **ポイント：** 実行中は **Foxglove Bridge** を使ってリアルタイムで確認、終了後は `.mcap` ファイルを Foxglove Studio で解析できます。


## Example

このサンプルアプリケーションでは次の処理を行います：
- 円運動をシミュレーション
- 現在位置をログ出力
- 各種データを記録
  - `FrameTransform`：座標変換
  - `SceneUpdate`：環境情報
  - `LaserScan`：レーザースキャン
  - `MultiArrayDouble` / `MultiArrayBool`：診断データ

実行後、自動的に `.mcap` 形式でデータが保存されます。

完全なサンプルコードはこちら：  
[example.cpp](example/example.cpp)


## Management Log Files
直近のログファイルは`/tmp/blackbox_log`に保存されます。  
過去のログは`/var/tmp/blackbox_archive`に保存されます。  
`/tmp/blackbox_log`内には、ノードごとに分かれてMcapファイルが保存されます。

### blackbox_create
ログファイルの保存場所`/tmp/blackbox_log`を作成します。  
プログラムの実行前に必ずこのコマンドを実行する必要があります。

launchファイルにこのような行を追加すれば、自動的にログファイルの保存場所を作成することができます。

```python
import subprocess
subprocess.run(["blackbox_create"])
```

### blackbox_archive
直近のログファイル`/tmp/blackbox_log`を`/var/tmp/blackbox_archive`に移動させます。  
`BLACKBOX`という名前の外部ディスクがあると、そちらに保存されるようになります。

こちらもlaunchファイルに追加することで、自動的にログファイルがアーカイブされるようになります。  
注意点としては、ログファイルの大きさによっては、launchの起動が遅くなる可能性があります。  
対策として、ros2をsystemdで起動するようにし、サービス終了時にアーカイブするようにすることを推奨します。

```python
import subprocess
subprocess.run(["blackbox_archive"])
subprocess.run(["blackbox_create"])
```

### blackbox_merge
`/tmp/blackbox_log`にある、ノードごとに分かれているMcapを一つにまとめることができます。  
一つになったmcapファイルは、`merged.mcap`として出力されます。

```bash
# 最後に実行したものに対して行う
blackbox_merge '/tmp/blackbox_log' 
# 過去のBlackBoxに対して
blackbox_merge '/var/tmp/blackbox_archive/blackbox_2024-03-24_21-24-14' 
# 現在のディレクトリに対して
blackbox_merge .
```