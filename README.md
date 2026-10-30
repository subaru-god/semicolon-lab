# semicolon-lab

## 音声とBGM

合成音声は同梱した `espeak-ng`、BGMは同梱した `mpg123` をコマンドで起動します。
Linuxでは次のコマンドで実行ファイルと依存パッケージを `runtime/` に展開してください。

```sh
./linux_setup.sh
```

Windowsでは `runtime/bin/espeak-ng.exe` と `runtime/bin/mpg123.exe`（および必要なDLL）を
配置してください。実行ファイルが `runtime/` にない場合は、PATH上のコマンドへフォールバックします。

### 効果音

効果音は `play_se()` で再生できます。再生処理は非同期なので、BGMや合成音声を
停止せずに重ねて再生できます。

```c
play_se("決定音.mp3");
```

### 音声読み込みログ

音声の読み込み時は `load_music.c` が対象ファイル、実行ファイル、作業ディレクトリ、
実行コマンド、`mpg123` の出力、終了状態を標準出力へ出します。音声が流れない場合は、
このログでファイルの存在と音声出力デバイスのエラーを確認してください。

Codespacesなど音声デバイスがない環境で単体実行すると、音声を鳴らさず
MP3をデコードできるか確認します。引数を省略すると`set_up.mp3`を検査します。

```sh
gcc load_music.c -o test
./test set_up.mp3
```
