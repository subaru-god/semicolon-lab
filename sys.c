#include "header.h"

static void print_and_wait(const char* text) {
    fputs(text, stdout);
    fflush(stdout);
    getchar();
}

static const char* audio_program(const char* name) {
    static char bundled[512];
#ifdef _WIN32
    snprintf(bundled, sizeof(bundled), "runtime\\bin\\%s.exe", name);
#else
    snprintf(bundled, sizeof(bundled), "runtime/bin/%s", name);
#endif
    if (access(bundled, 0) == 0) {
        return bundled;
    }
    return name;
}

static void write_speech_file(const char* text, char* path, size_t path_size) {
#ifdef _WIN32
    if (access("runtime", 0) == 0) {
        snprintf(path, path_size, "runtime\\speech.txt");
    } else {
        snprintf(path, path_size, ".semicolon-speech.txt");
    }
#else
    if (access("runtime", 0) == 0) {
        snprintf(path, path_size, "runtime/speech.txt");
    } else {
        snprintf(path, path_size, ".semicolon-speech.txt");
    }
#endif
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        perror("speech file");
        return;
    }
    fputs(text, file);
    fclose(file);
}

static void speak_text(const char* text) {
    char speech_file[512];
    char command[2048];
    write_speech_file(text, speech_file, sizeof(speech_file));
#ifdef _WIN32
    snprintf(command, sizeof(command),
        "start \"\" /b \"%s\" -v ja -f \"%s\" >nul 2>&1",
        audio_program("espeak-ng"), speech_file);
#else
    snprintf(command, sizeof(command),
        "ESPEAK_DATA_PATH=\"runtime/usr/share/espeak-ng-data\" "
        "LD_LIBRARY_PATH=\"runtime/lib:runtime/usr/lib:runtime/lib/x86_64-linux-gnu:"
        "runtime/usr/lib/x86_64-linux-gnu\" \"%s\" -v ja -f \"%s\" >/dev/null 2>&1 &",
        audio_program("espeak-ng"), speech_file);
#endif
    if (system(command) == -1) {
        fputs("Failed to start espeak-ng.\n", stderr);
    }
}

void echof(const char* format, ...) {
    char text[1024];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);

    speak_text(text);
    print_and_wait(text);
}

void play_bgm(const char* filename) {
#ifdef _WIN32
    char command[512];
    const char* program = audio_program("mpg123");

    stop_bgm();
    snprintf(command, sizeof(command),
        "start \"\" /b \"%s\" --loop -1 \"%s\"",
        program, filename);
    log_music_load("BGM", filename, program, command, 0);
    int status = system(command);
    log_music_load("BGM", filename, program, command, status);
    if (status == -1) {
        fputs("Failed to start mpg123.\n", stderr);
    }
#else
    char command[1024];
    const char* program = audio_program("mpg123");

    stop_bgm();
    snprintf(command, sizeof(command),
        "LD_LIBRARY_PATH=\"/usr/lib/x86_64-linux-gnu:"
        "runtime/usr/lib/x86_64-linux-gnu\" "
        "MPG123_MODDIR=\"runtime/usr/lib/x86_64-linux-gnu/mpg123\" "
        "\"%s\" --loop -1 \"%s\" "
        "& echo $! > runtime/mpg123.pid",
        program, filename);
    log_music_load("BGM", filename, program, command, 0);
    int status = system(command);
    log_music_load("BGM", filename, program, command, status);
    if (status == -1) {
        fputs("Failed to start mpg123.\n", stderr);
    }
#endif
}

void play_se(const char* filename) {
#ifdef _WIN32
    char command[512];
    const char* program = audio_program("mpg123");

    snprintf(command, sizeof(command),
        "start \"\" /b \"%s\" \"%s\"",
        program, filename);
    log_music_load("SE", filename, program, command, 0);
    int status = system(command);
    log_music_load("SE", filename, program, command, status);
    if (status == -1) {
        fputs("Failed to start mpg123.\n", stderr);
    }
#else
    char command[1024];
    const char* program = audio_program("mpg123");

    snprintf(command, sizeof(command),
        "LD_LIBRARY_PATH=\"/usr/lib/x86_64-linux-gnu:"
        "runtime/usr/lib/x86_64-linux-gnu\" "
        "MPG123_MODDIR=\"runtime/usr/lib/x86_64-linux-gnu/mpg123\" "
        "\"%s\" \"%s\" "
        "&",
        program, filename);
    log_music_load("SE", filename, program, command, 0);
    int status = system(command);
    log_music_load("SE", filename, program, command, status);
    if (status == -1) {
        fputs("Failed to start mpg123.\n", stderr);
    }
#endif
}

void stop_bgm(void) {
#ifdef _WIN32
    system("taskkill /f /im mpg123.exe >nul 2>&1");
#else
    system("if [ -f runtime/mpg123.pid ]; then kill \"$(cat runtime/mpg123.pid)\" "
           ">/dev/null 2>&1 || true; rm -f runtime/mpg123.pid; fi");
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
