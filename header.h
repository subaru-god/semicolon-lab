#pragma once
#ifndef SEMICOLON_LAB_HEADER_H
#define SEMICOLON_LAB_HEADER_H

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>

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
#ifdef _WIN32
    wchar_t wide_path[4096];
    wchar_t wide_mode[32];

    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1,
                            wide_path, sizeof(wide_path) / sizeof(wide_path[0])) == 0 ||
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, mode, -1,
                            wide_mode, sizeof(wide_mode) / sizeof(wide_mode[0])) == 0) {
        fprintf(stderr, "UTF-8 path conversion failed for '%s'.\n", path);
        return NULL;
    }
    return _wfopen(wide_path, wide_mode);
#else
    return fopen(path, mode);
#endif
}

#define fopen project_fopen

static void configure_console_encoding(void) {
#ifdef _WIN32
    wchar_t executable_path[4096];
    DWORD path_length;
    wchar_t* separator;

    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    path_length = GetModuleFileNameW(NULL, executable_path,
                                     sizeof(executable_path) / sizeof(executable_path[0]));
    if (path_length == 0 || path_length >= sizeof(executable_path) / sizeof(executable_path[0])) {
        fprintf(stderr, "Unable to determine executable directory.\n");
        return;
    }
    separator = wcsrchr(executable_path, L'\\');
    if (separator != NULL) {
        *separator = L'\0';
        {
            wchar_t runtime_path[4096];
            int written = swprintf(runtime_path,
                                    sizeof(runtime_path) / sizeof(runtime_path[0]),
                                    L"%ls\\runtime", executable_path);
            DWORD attributes = written > 0 ? GetFileAttributesW(runtime_path)
                                           : INVALID_FILE_ATTRIBUTES;

            if (attributes != INVALID_FILE_ATTRIBUTES &&
                (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 &&
                !SetCurrentDirectoryW(executable_path)) {
                fprintf(stderr, "Unable to set executable directory as current directory.\n");
            }
        }
    }
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
void stop_bgm(void);
void audio_shutdown(void);
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
