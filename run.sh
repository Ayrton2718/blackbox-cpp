#!/bin/bash

# クリーンアップ関数を定義
cleanup() {
    echo "CTRL-C を検知しました。ログをアーカイブします..."
    blackbox_archive
    exit 1
}

# SIGINT（CTRL-C）を捕まえてクリーンアップ関数を呼ぶ
trap cleanup SIGINT

blackbox_create
./build/Release/example.out
blackbox_archive