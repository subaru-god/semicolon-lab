# semicolon-lab

## 音声機能

合成音声はOpenJTalkの`mei_normal`音響モデルでWAVを生成し、miniaudioで再生します。
BGMと効果音もminiaudioでMP3を再生します。Windows/Linux共通の音声処理です。

初回はLinuxで次のコマンドを実行してください。OpenJTalk本体、NAIST辞書、
`mei_normal.htsvoice`を`runtime/`へ配置します。

```sh
./setup_audio.sh
```

Windowsでは次を配置してください。

```text
runtime/bin/open_jtalk.exe
runtime/dic/                 # OpenJTalk NAIST辞書
runtime/voice/mei_normal.htsvoice
```

`miniaudio.h`はリポジトリに同梱しています。`play_bgm()`はループ再生、
`play_se()`は最大16音まで重ねて再生します。

Linuxで手動ビルドする場合は、miniaudioが使用する数学・動的ロード・スレッド
ライブラリをリンクしてください。

```sh
gcc main.c -o main -lm -ldl -lpthread
```
