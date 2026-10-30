#include "header.h"

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <sys/stat.h>
#endif

static void music_log_line(const char* label, const char* value) {
    fprintf(stdout, "[music] %-16s %s\n", label, value != NULL ? value : "(null)");
}

static void music_log_number(const char* label, long value) {
    fprintf(stdout, "[music] %-16s %ld\n", label, value);
}

static const char* music_program(void) {
    static char path[512];
#ifdef _WIN32
    snprintf(path, sizeof(path), "runtime\\bin\\mpg123.exe");
#else
    snprintf(path, sizeof(path), "runtime/bin/mpg123");
#endif
    return access(path, 0) == 0 ? path : "mpg123";
}

void log_music_load(const char* type, const char* filename,
                    const char* program, const char* command,
                    int system_status) {
    char cwd[4096];
    char status[32];
    FILE* music_file;

    music_log_line("event", "load");
    music_log_line("type", type);
    music_log_line("file", filename);
    music_log_line("program", program);

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        music_log_line("cwd", cwd);
    } else {
        music_log_line("cwd", "(unavailable)");
    }

    music_file = fopen(filename, "rb");
    music_log_line("file_access", music_file != NULL ? "ok" : "failed");
    if (music_file != NULL) {
        fclose(music_file);
    } else {
        music_log_line("file_error", strerror(errno));
    }

    music_log_line("command", command);
    if (system_status < 0) {
        snprintf(status, sizeof(status), "system error (%s)", strerror(errno));
    } else {
        snprintf(status, sizeof(status), "%d", system_status);
    }
    music_log_line("system_status", status);
    music_log_number("errno", errno);
    fflush(stdout);
}

#ifndef SEMICOLON_LOAD_MUSIC_EMBEDDED
int main(int argc, char* argv[]) {
    const char* filename = argc >= 2 ? argv[1] : "set_up.mp3";
    const char* program = music_program();
    char command[2048];
    int status;

#ifdef _WIN32
    snprintf(command, sizeof(command), "\"%s\" -q -w NUL \"%s\"",
             program, filename);
#else
    snprintf(command, sizeof(command),
             "LD_LIBRARY_PATH=\"/usr/lib/x86_64-linux-gnu:"
             "runtime/usr/lib/x86_64-linux-gnu\" "
             "MPG123_MODDIR=\"runtime/usr/lib/x86_64-linux-gnu/mpg123\" "
             "\"%s\" -q -w /dev/null \"%s\"",
             program, filename);
#endif
    log_music_load("decode", filename, program, command, 0);
    status = system(command);
    log_music_load("decode", filename, program, command, status);
    return status == -1 ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
