#pragma once
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#if defined(__unix__) || defined(__APPLE__)
    #include <iconv.h>
#endif

#ifdef _WIN32
    #include <windows.h>
    #include <mmsystem.h>
    #include <conio.h>
#elif  defined(__unix__) || defined(__APPLE__)
    // Unix系ではtermios.hで端末を操作し、usleepで待機する。
    #include <unistd.h>
    #include <termios.h>
    #include <unistd.h>
    #define Sleep(ms) usleep((ms) * 1000)
#else
    // 対応していないプラットフォーム。
    #error "Unsupported platform"
#endif

#if defined(_WIN32) && defined(_MSC_VER)
    // MSVCでビルドする場合はWindowsのマルチメディアライブラリをリンクする。
    #pragma comment(lib, "winmm.lib")
#endif



static FILE* project_fopen(const char* path, const char* mode) {
#if defined(__unix__) || defined(__APPLE__)
    iconv_t converter = iconv_open("UTF-8", "CP932");
    char utf8_path[4096];
    char* input = (char*)path;
    char* output = utf8_path;
    size_t input_left = strlen(path);
    size_t output_left = sizeof(utf8_path) - 1;

    if (converter == (iconv_t)-1) {
        perror("iconv_open");
        return NULL;
    }
    if (iconv(converter, &input, &input_left, &output, &output_left) == (size_t)-1) {
        int saved_errno = errno;
        iconv_close(converter);
        errno = saved_errno;
        perror("CP932 path conversion");
        return NULL;
    }
    *output = '\0';
    iconv_close(converter);
    return fopen(utf8_path, mode);
#else
    return fopen(path, mode);
#endif
}

#define fopen project_fopen

extern char user_select;     // ユーザーの選択。
extern char name[];          // プレイヤーの名前。
extern char call_name[];     // プレイヤーの呼び名。
extern int level_of_story;   // 選択したストーリーの難易度。
extern int money;            // プレイヤーの所持金。
extern int random_number;    // 汎用乱数。
extern int random_number2;   // 予備の乱数。
extern int cache;            // 汎用整数キャッシュ。
extern int cache2;           // 予備の整数キャッシュ。
extern int cache3[];         // 整数キャッシュ配列。
extern char cache_str[];     // 汎用文字列キャッシュ。
extern int result;           // 汎用の結果格納用変数。
extern int ap;               // アクションポイント。
extern int sum_ap;           // アクションポイントの合計。
extern int m_ap;             // 現在の最大アクションポイント。
extern int m_sum_ap;         // 最大アクションポイントの合計。
extern int current_weapon_id;// 現在装備している武器のID。
extern int can_magics[];     // 習得済み魔法のフラグ。
extern int can_weapon[];     // 使用可能な武器のフラグ。
extern int can_item[];       // 使用可能なアイテムのフラグ。
extern int player_hp;        // プレイヤーの最大HP。
extern int player_rest_hp;   // プレイヤーの残りHP。

// 書式付きの文字列を表示する。
void echof(const char* format, ...);
// BGMを再生する。
void play_bgm(const char* filename);
// BGMを停止する。
void stop_bgm(void);
// 端末の表示を消去する。
void cls(void);
// 指定した敵との戦闘を実行する。
int battle(int e_id);
// アイテムショップを開く。
int item_shop(int first_id, ...);
// 武器ショップを開く。
int weapon_shop(int first_id, ...);
// 総合ショップを開く。
int all_shop(int first_id, ...);
// マップモードを開始する。
void start_map_mode(const char* filename, int spawn_id, int has_enemy, ...);
// プレイヤーの装備を変更する。
void change(void);
// ゲームデータを初期化する。
void set_up(void);
// プロローグを実行する。
void prologue(void);
// メインストーリーを実行する。
int story_1(void);
// デバッグメニューを実行する。
int debug(void);

#include "change.c"
#include "sys.c"
#include "story.c"
#include "battle.c"
#include "shop.c"
#include "map.c"