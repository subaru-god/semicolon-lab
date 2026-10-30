#pragma once
#ifndef SEMICOLON_LAB_HEADER_H
#define SEMICOLON_LAB_HEADER_H

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
    #include <io.h>
    #include <conio.h>
    #define access _access
#elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    #include <unistd.h>
    #include <termios.h>
    #define Sleep(ms) usleep((ms) * 1000)
#else
    #error "Unsupported platform"
#endif

#if defined(_WIN32) && defined(_MSC_VER)
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

static void configure_console_encoding(void) {
#ifdef _WIN32
    SetConsoleCP(932);
    SetConsoleOutputCP(932);
#endif
}

extern char user_select;
extern char name[];
extern char call_name[];
extern int level_of_story;
extern int money;
extern int random_number;
extern int random_number2;
extern int cache;
extern int cache2;
extern int cache3[];
extern char cache_str[];
extern int result;
extern int ap;
extern int sum_ap;
extern int m_ap;
extern int m_sum_ap;
extern int current_weapon_id;
extern int can_magics[];
extern int can_weapon[];
extern int can_item[];
extern int player_hp;
extern int player_rest_hp;

void echof(const char* format, ...);
void play_bgm(const char* filename);
void play_se(const char* filename);
void log_music_load(const char* type, const char* filename,
                    const char* program, const char* command,
                    int system_status);
void stop_bgm(void);
void cls(void);
int battle(int e_id);
int item_shop(int first_id, ...);
int weapon_shop(int first_id, ...);
int all_shop(int first_id, ...);
void start_map_mode(const char* filename, int spawn_id, int has_enemy, ...);
void change(void);
void set_up(void);
void prologue(void);
int story_1(void);
int debug(void);

#endif
