#pragma once
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <conio.h>
#elif  defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <termios.h>
#else
#error "Unsupported platform"
#endif

#if defined(_WIN32) && defined(_MSC_VER)
#pragma comment(lib, "winmm.lib")
#endif

#include "change.c"
#include "sys.c"
#include "story.c"
#include "battle.c"
#include "shop.c"
#include "map.c"

extern char user_select;     //ユーザーの選択を格納する変数
extern char name[];          //プレイヤーの名前を格納する変数
extern char call_name[];     //プレイヤーの呼ばれ方を格納する変数
extern int level_of_story;   //プレイヤーが選択した難易度を格納する変数
extern int money;            //プレイヤーの所持金を格納する変数
extern int random;           //乱数を格納する変数
extern int random2;          //乱数を格納する変数
extern int cache;            //一時的な値を格納する変数
extern int cache2;           //一時的な値を格納する変数
extern int cache3[]; 	  	 //一時的な値を格納する配列
extern char cache_str[];     //一時的な文字列を格納する配列
extern int result;           //戦闘の結果を格納する変数
extern int ap;               //プレイヤーの攻撃力を格納する変数
extern int sum_ap;           //プレイヤーの合計攻撃力を格納する変数
extern int m_ap;             //プレイヤーの魔法攻撃力を格納する変数
extern int m_sum_ap;         //プレイヤーの合計魔法攻撃力を格納する変数
extern int current_weapon_id;//プレイヤーが装備している武器のIDを格納する変数
extern int can_magics[];     //プレイヤーが使用可能な魔法のIDを格納する配列
extern int can_weapon[];     //プレイヤーが所持している武器のIDを格納する配列
extern int can_item[];       //プレイヤーが所持しているアイテムのIDを格納する配列
extern int player_hp;        //プレイヤーの現在のHPを格納する変数
extern int player_rest_hp;   //プレイヤーの残りHPを格納する変数

//printfのように文字列を出力し、同時に音声で読み上げる関数(getchar()あり)
void echof(const char* format, ...);
//BGMを再生する関数
void play_bgm(const char* filename);
//BGMを停止する関数
void stop_bgm(void);
//画面をクリアする関数
void cls(void);
//敵IDを指定して戦闘を開始する関数
int battle(int e_id);
//アイテムショップを開く関数(カンマ区切りで品物を指定(-1で終了))
int item_shop(int first_id, ...);
//武器ショップを開く関数(カンマ区切りで品物を指定(-1で終了))
int weapon_shop(int first_id, ...);
//よろず屋を開く関数(カンマ区切りで品物を指定(-1でアイテムを区切る))
int all_shop(int first_id, ...);
//マップモードを開始する関数(カンマ区切りで敵IDを指定(-1で終了))
void start_map_mode(const char* filename, int spawn_id, int has_enemy, ...);
//装備変更を行う関数
void change(void);

//ゲーム開始時の初期設定を行う関数
void set_up(void);
//ゲームのプロローグを表示する関数
void prologue(void);
//ゲームのエピローグを表示する関数
int story_1(void);
//ゲームのデバッグ情報を表示する関数
int debug(void);