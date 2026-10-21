
// 合成音声を流す関数 (getchar()がある)
void echof(const char* format, ...) {
    char text_buffer[1024];
    char clean_buffer[1024];
    char command_buffer[2048];
    va_list args;
    int i, j = 0;

    // 1. フォーマット文字列の組み立て
    va_start(args, format);
    vsprintf(text_buffer, format, args);
    va_end(args);

    // 2. PowerShellエラーの原因となる「改行」や「シングルクォート」を除去・整形
    for (i = 0; text_buffer[i] != '\0'; i++) {
        if (text_buffer[i] == '\r' || text_buffer[i] == '\n') {
            continue; // 改行文字は音声コマンドから除外
        }
        if (text_buffer[i] == '\'') {
            clean_buffer[j++] = '\''; // シングルクォートのエスケープ
        }
        clean_buffer[j++] = text_buffer[i];
    }
    clean_buffer[j] = '\0';

    // 3. 既存のPowerShell音声再生プロセスを終了（音声の上書き）
    system("taskkill /f /im powershell.exe >nul 2>&1");

    // 4. PowerShellを呼び出して読み上げ（-Command ではなくスクリプトブロック渡しで文字化け・途切りを防止）
// 話速($s.Rate) と 音量($s.Volume) を設定する例
    sprintf(command_buffer,
        "start /b powershell -NoProfile -ExecutionPolicy Bypass -Command "
        "\"Add-Type -AssemblyName System.Speech; "
        "$s = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
        "$s.Rate = 3; "     // 話速: -10 ～ 10 (例: 2 で少し早口)
        "$s.Volume = 100; "  // 音量: 0 ～ 100
        "$s.Speak('%s');\" >nul 2>&1",
        clean_buffer);

    system(command_buffer);
    Sleep(600);
    // 画面には元のテキスト（改行あり）を表示
    printf("%s", text_buffer);

    // エンターキー待機
    getchar();
}


/* BGM再生開始 (ファイル名指定) */
void play_bgm(const char* filename) {
    char cmd[512];

    /* 既に鳴っているBGMがあれば停止して閉じる */
    mciSendString("stop my_bgm", NULL, 0, NULL);
    mciSendString("close my_bgm", NULL, 0, NULL);

    /* BGMファイルを開く (alias: my_bgm として登録) */
    sprintf(cmd, "open \"%s\" type mpegvideo alias my_bgm", filename);
    mciSendString(cmd, NULL, 0, NULL);

    /* ループ再生を開始 */
    mciSendString("play my_bgm repeat", NULL, 0, NULL);
}

/* BGM停止 */
void stop_bgm(void) {
    mciSendString("stop my_bgm", NULL, 0, NULL);
    mciSendString("close my_bgm", NULL, 0, NULL);
}

void cls(void) {
    printf("\033[2J\033[H");
}