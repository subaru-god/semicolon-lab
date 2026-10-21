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
    // Linux terminal support
    #include <unistd.h>
    #include <termios.h>
    #include <unistd.h>
    // Linux Sleep compatibility
    #define Sleep(ms) usleep((ms) * 1000)
#else
    // legacy comment removed during CP932 normalization
    #error "Unsupported platform"
#endif

#if defined(_WIN32) && defined(_MSC_VER)
    // legacy comment removed during CP932 normalization
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

extern char user_select;     // legacy comment removed during CP932 normalization
extern char name[];          // legacy comment removed during CP932 normalization
extern char call_name[];     // legacy comment removed during CP932 normalization
extern int level_of_story;   // legacy comment removed during CP932 normalization
extern int money;            // legacy comment removed during CP932 normalization
extern int random_number;           // legacy comment removed during CP932 normalization
extern int random_number2;          // legacy comment removed during CP932 normalization
extern int cache;            // legacy comment removed during CP932 normalization
extern int cache2;           // legacy comment removed during CP932 normalization
extern int cache3[]; 	  	 // legacy comment removed during CP932 normalization
extern char cache_str[];     // legacy comment removed during CP932 normalization
extern int result;           // legacy comment removed during CP932 normalization
extern int ap;               // legacy comment removed during CP932 normalization
extern int sum_ap;           // legacy comment removed during CP932 normalization
extern int m_ap;             // legacy comment removed during CP932 normalization
extern int m_sum_ap;         // legacy comment removed during CP932 normalization
extern int current_weapon_id;// legacy comment removed during CP932 normalization
extern int can_magics[];     // legacy comment removed during CP932 normalization
extern int can_weapon[];     // legacy comment removed during CP932 normalization
extern int can_item[];       // legacy comment removed during CP932 normalization
extern int player_hp;        // legacy comment removed during CP932 normalization
extern int player_rest_hp;   // legacy comment removed during CP932 normalization

// legacy comment removed during CP932 normalization
void echof(const char* format, ...);
// legacy comment removed during CP932 normalization
void play_bgm(const char* filename);
// legacy comment removed during CP932 normalization
void stop_bgm(void);
// legacy comment removed during CP932 normalization
void cls(void);
// legacy comment removed during CP932 normalization
int battle(int e_id);
// legacy comment removed during CP932 normalization
int item_shop(int first_id, ...);
// legacy comment removed during CP932 normalization
int weapon_shop(int first_id, ...);
// legacy comment removed during CP932 normalization
int all_shop(int first_id, ...);
// legacy comment removed during CP932 normalization
void start_map_mode(const char* filename, int spawn_id, int has_enemy, ...);
// legacy comment removed during CP932 normalization
void change(void);

// legacy comment removed during CP932 normalization
void set_up(void);
// legacy comment removed during CP932 normalization
void prologue(void);
// legacy comment removed during CP932 normalization
int story_1(void);
// legacy comment removed during CP932 normalization
int debug(void);

#include "change.c"
#include "sys.c"
#include "story.c"
#include "battle.c"
#include "shop.c"
#include "map.c"