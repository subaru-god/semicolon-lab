#include "header.h"

// 合成音声を流す関数 (getchar()がある)
#ifdef _WIN32
static HANDLE echof_process = NULL;
#endif

static void echof_v(const char* format, va_list args, int wait_for_input) {
#ifdef _WIN32
    char text_buffer[1024];
    char clean_buffer[2048];
    char command_buffer[4096];
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;
    DWORD process_state;
    int i, j = 0;

    // 1. フォーマット文字列の組み立て
    vsprintf(text_buffer, format, args);

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
    /* Avoid killing all PowerShell processes; it adds startup latency. */

    // 4. PowerShellを呼び出して読み上げ（-Command ではなくスクリプトブロック渡しで文字化け・途切りを防止）
// 話速($s.Rate) と 音量($s.Volume) を設定する例
	if (echof_process != NULL) {
		process_state = 0;
		if (GetExitCodeProcess(echof_process, &process_state) && process_state == STILL_ACTIVE) {
			TerminateProcess(echof_process, 0);
		}
		CloseHandle(echof_process);
		echof_process = NULL;
	}

	sprintf(command_buffer,
		"powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "
		"\"Add-Type -AssemblyName System.Speech; "
		"$s = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
		"$s.Rate = 3; $s.Volume = 100; $s.Speak('%s');\"",
		clean_buffer);

	memset(&startup_info, 0, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);
	memset(&process_info, 0, sizeof(process_info));

	if (CreateProcessA(
			NULL,
			command_buffer,
			NULL,
			NULL,
			FALSE,
			CREATE_NO_WINDOW,
			NULL,
			NULL,
			&startup_info,
			&process_info)) {
		echof_process = process_info.hProcess;
		CloseHandle(process_info.hThread);
	}
	else {
		log_error("AUDIO", "failed to start speech command: error %lu", GetLastError());
	}

    // 画面には元のテキスト（改行あり）を表示
    printf("%s", text_buffer);

    // エンターキー待機
    if (wait_for_input) {
        getchar();
    }
#else
    vprintf(format, args);
    if (wait_for_input) {
        getchar();
    }
#endif
}

void echof(const char* format, ...) {
    va_list args;
    va_start(args, format);
    echof_v(format, args, 1);
    va_end(args);
}

void echof_skip(const char* format, ...) {
    va_list args;
    va_start(args, format);
    echof_v(format, args, 0);
    va_end(args);
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

    if (mciSendString("setaudio my_bgm volume to 300", NULL, 0, NULL) != 0) {
        log_error("AUDIO", "failed to set BGM volume: %s", filename);
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

int seek_level(void) {
    int EXP_A = 10; // レベルアップに必要な経験値の増加量
    int EXP_B = 0;   // レベル1に必要な経験値
    int EXP_K = 3;   // レベルアップに必要な経験値の増加率
	int max_level = 100; // 最大レベル
    int atk_min = 5;
	int atk_max = 1000;
	int p = 2; // 累乗成長の指数 
    double ratio;

    double calculated_level;

    if (player_exp < EXP_B) {
        return 1;
    }



    // 逆算式: L = ((E - B) / A) ^ (1 / k) + 1
    calculated_level = pow(((double)player_exp - EXP_B) / EXP_A, 1.0 / EXP_K) + 1.0;

    if (player_level != (int)calculated_level) {
        player_level = (int)calculated_level;
        echof("レベルが %d に上がった！\n", player_level);



        // 進行度の割合 (0.0 ～ 1.0) を計算 ※doubleにキャストして整数除算を防ぐ
        ratio = (double)(player_level - 1) / (double)(max_level - 1);

        // 累乗成長式の計算
        ap = atk_min + (atk_max - atk_min) * pow(ratio, p);
        if (current_weapon_id > 0) {
            change(current_weapon_id);
        }
        else {
            sum_ap = ap;
        }

		echof("攻撃力が %d に上がった！\n", ap);

        player_mp = player_max_mp;
        player_rest_hp = player_max_hp;
        echof("HPとMPが全回復しました。\n");

    }

    // 小数点以下を切り捨てて整数（レベル）にする
    return (int)calculated_level;
}

// atk^2 / (atk + def) の計算