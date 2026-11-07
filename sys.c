#include "header.h"

// 合成音声を流す関数 (getchar()がある)
void echof(const char* format, ...) {
#ifdef _WIN32
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
        "start \"\" /b powershell -NoProfile -ExecutionPolicy Bypass -Command "
        "\"Add-Type -AssemblyName System.Speech; "
        "$s = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
        "$s.Rate = 3; "     // 話速: -10 ～ 10 (例: 2 で少し早口)
        "$s.Volume = 200; "  // 音量: 0 ～ 100
        "$s.Speak('%s');\" >nul 2>&1",
        clean_buffer);

    if (system(command_buffer) != 0) {
        log_error("AUDIO", "failed to start speech command");
    }
    // 画面には元のテキスト（改行あり）を表示
    printf("%s", text_buffer);
	Sleep(3000); // 音声が途切れないように少し待機

    // エンターキー待機
    getchar();
#else
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    getchar();
#endif
}


/* BGM再生開始 (ファイル名指定) */
void play_bgm(const char* filename) {
#ifdef _WIN32
    char cmd[512];

    /* 既に鳴っているBGMがあれば停止して閉じる */
    mciSendString("stop my_bgm", NULL, 0, NULL);
    mciSendString("close my_bgm", NULL, 0, NULL);

    /* BGMファイルを開く (alias: my_bgm として登録) */
    sprintf(cmd, "open \"%s\" type mpegvideo alias my_bgm", filename);
    if (mciSendString(cmd, NULL, 0, NULL) != 0) {
        log_error("AUDIO", "failed to open BGM: %s", filename);
    }

    /* ループ再生を開始 */
    if (mciSendString("play my_bgm repeat", NULL, 0, NULL) != 0) {
        log_error("AUDIO", "failed to play BGM: %s", filename);
    }
#else
    (void)filename;
#endif
}
#define SE_CHANNEL_COUNT 16

static int se_open[SE_CHANNEL_COUNT] = { 0 };
static int se_next_channel = 0;

static int is_wav_file(const char* filename) {
    const char* extension = strrchr(filename, '.');

    if (extension == NULL) {
        return 0;
    }

#if defined(__BORLANDC__)
    return stricmp(extension, ".wav") == 0;
#elif defined(_MSC_VER)
    return _stricmp(extension, ".wav") == 0;
#else
    return strcmp(extension, ".wav") == 0;
#endif
}

/* SEを一度だけ再生する */
void play_se(const char* filename) {
#ifdef _WIN32
    char alias[32];
    char command[1024];
    char status[32];
    int channel;
    int i;

    if (filename == NULL || filename[0] == '\0') {
        log_error("AUDIO", "SE filename is empty");
        return;
    }

    /*
     * 使用可能なチャンネルを探す。
     * 再生終了済みのSEは、次回再生時に自動的にクローズする。
     */
    channel = -1;

    for (i = 0; i < SE_CHANNEL_COUNT; i++) {
        int candidate = (se_next_channel + i) % SE_CHANNEL_COUNT;

        sprintf(alias, "se_%d", candidate);

        if (!se_open[candidate]) {
            channel = candidate;
            break;
        }

        memset(status, 0, sizeof(status));

        sprintf(command, "status %s mode", alias);

        if (mciSendStringA(
            command,
            status,
            sizeof(status),
            NULL) != 0 ||
            strcmp(status, "playing") != 0) {
            if (mciSendStringA(
                command,
                status,
                sizeof(status),
                NULL) != 0 ||
                strcmp(status, "playing") != 0) {
                sprintf(command, "close %s", alias);
                mciSendStringA(command, NULL, 0, NULL);

                se_open[candidate] = 0;
                channel = candidate;
                break;
            }
            sprintf(command, "close %s", alias);
            mciSendStringA(command, NULL, 0, NULL);

            se_open[candidate] = 0;
            channel = candidate;
            break;
        }
    }

    /*
     * すべてのチャンネルが再生中の場合は、新しいSEを再生しない。
     * SE_CHANNEL_COUNTを増やすと同時再生数を増やせる。
     */
    if (channel < 0) {
        log_error("AUDIO", "no available SE channel for %s", filename);
        return;
    }

    se_next_channel = (channel + 1) % SE_CHANNEL_COUNT;

    sprintf(alias, "se_%d", channel);

    if (is_wav_file(filename)) {
        sprintf(
            command,
            "open \"%s\" type waveaudio alias %s",
            filename,
            alias);
    }
    else {
        sprintf(
            command,
            "open \"%s\" type mpegvideo alias %s",
            filename,
            alias);
    }

    if (mciSendStringA(command, NULL, 0, NULL) != 0) {
        log_error("AUDIO", "failed to open SE: %s", filename);
        se_open[channel] = 0;
        return;
    }

    sprintf(command, "play %s", alias);

    if (mciSendStringA(command, NULL, 0, NULL) != 0) {
        log_error("AUDIO", "failed to play SE: %s", filename);
        sprintf(command, "close %s", alias);
        mciSendStringA(command, NULL, 0, NULL);
        se_open[channel] = 0;
        return;
    }

    se_open[channel] = 1;
#else
    (void)filename;
#endif
}

/* 再生中のSEをすべて停止する */
void stop_se_all(void) {
#ifdef _WIN32
    char alias[32];
    char command[64];
    int i;

    for (i = 0; i < SE_CHANNEL_COUNT; i++) {
        sprintf(alias, "se_%d", i);

        sprintf(command, "stop %s", alias);
        mciSendStringA(command, NULL, 0, NULL);

        sprintf(command, "close %s", alias);
        mciSendStringA(command, NULL, 0, NULL);

        se_open[i] = 0;
    }
#endif
}

/* BGM停止 */
void stop_bgm(void) {
#ifdef _WIN32
    mciSendString("stop my_bgm", NULL, 0, NULL);
    mciSendString("close my_bgm", NULL, 0, NULL);
#endif
}

void cls(void) {
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    DWORD written;
    COORD origin = { 0, 0 };

    if (GetConsoleScreenBufferInfo(output, &info)) {
        DWORD cells = (DWORD)info.dwSize.X * (DWORD)info.dwSize.Y;
        FillConsoleOutputCharacterA(output, ' ', cells, origin, &written);
        FillConsoleOutputAttribute(output, info.wAttributes, cells, origin, &written);
        SetConsoleCursorPosition(output, origin);
        return;
    }
#endif
    printf("\033[2J\033[H");
}