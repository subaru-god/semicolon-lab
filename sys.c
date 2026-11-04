#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "header.h"

#define AUDIO_SE_SLOTS 16

static ma_engine audio_engine;
static ma_sound bgm_sound;
static ma_sound se_sounds[AUDIO_SE_SLOTS];
static int se_ready[AUDIO_SE_SLOTS];
static int audio_ready;
static int bgm_ready;

static const char* audio_binary(const char* name) {
    static char path[512];
#ifdef _WIN32
    snprintf(path, sizeof(path), "runtime\\bin\\%s.exe", name);
#else
    snprintf(path, sizeof(path), "runtime/bin/%s", name);
#endif
    return path;
}

static int audio_start(void) {
    ma_result result;

    if (audio_ready) {
        return 1;
    }
    result = ma_engine_init(NULL, &audio_engine);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "miniaudio initialization failed: %s\n",
                ma_result_description(result));
        return 0;
    }
    audio_ready = 1;
    return 1;
}

static void audio_stop_sound(ma_sound* sound, int* initialized) {
    if (*initialized) {
        ma_sound_stop(sound);
        ma_sound_uninit(sound);
        *initialized = 0;
    }
}

static int run_open_jtalk(const char* text, const char* wav_path) {
    char text_path[512];
    char command[2048];
    FILE* file;
    int status;

#ifdef _WIN32
    snprintf(text_path, sizeof(text_path), "runtime\\speech.txt");
    if (access("runtime\\bin\\open_jtalk.exe", 0) != 0) {
        fprintf(stderr,
                "OpenJTalk executable was not found: "
                "runtime\\bin\\open_jtalk.exe\n");
        return 0;
    }
    snprintf(command, sizeof(command),
        "\"%s\" -x \"runtime\\dic\" -m \"runtime\\voice\\mei_normal.htsvoice\" "
        "-ow \"%s\" \"%s\"",
        audio_binary("open_jtalk"), wav_path, text_path);
#else
    snprintf(text_path, sizeof(text_path), "runtime/speech.txt");
    snprintf(command, sizeof(command),
        "LD_LIBRARY_PATH=\"runtime/usr/lib/x86_64-linux-gnu\" "
        "\"%s\" -x \"runtime/dic\" -m \"runtime/voice/mei_normal.htsvoice\" "
        "-ow \"%s\" \"%s\"",
        audio_binary("open_jtalk"), wav_path, text_path);
#endif

    file = fopen(text_path, "wb");
    if (file == NULL) {
        perror("speech file");
        return 0;
    }
    if (fputs(text, file) == EOF) {
        perror("speech file");
        fclose(file);
        return 0;
    }
    if (fputc('\n', file) == EOF) {
        perror("speech file");
        fclose(file);
        return 0;
    }
    fclose(file);

    status = system(command);
    if (status != 0) {
        fprintf(stderr, "OpenJTalk failed with status %d.\n", status);
        return 0;
    }
    return 1;
}

static void speak_text(const char* text) {
    const char* wav_path;
    ma_result result;

#ifdef _WIN32
    wav_path = "runtime\\speech.wav";
#else
    wav_path = "runtime/speech.wav";
#endif
    if (!run_open_jtalk(text, wav_path) || !audio_start()) {
        return;
    }
    result = ma_engine_play_sound(&audio_engine, wav_path, NULL);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Unable to play speech: %s\n",
                ma_result_description(result));
    }
}

void echof(const char* format, ...) {
    char text[1024];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);

    speak_text(text);
    fputs(text, stdout);
    fflush(stdout);
    getchar();
}

void play_bgm(const char* filename) {
    ma_result result;

    if (!audio_start()) {
        return;
    }
    audio_stop_sound(&bgm_sound, &bgm_ready);
    result = ma_sound_init_from_file(&audio_engine, filename,
                                     MA_SOUND_FLAG_STREAM, NULL, NULL,
                                     &bgm_sound);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Unable to load BGM '%s': %s\n",
                filename, ma_result_description(result));
        return;
    }
    bgm_ready = 1;
    ma_sound_set_looping(&bgm_sound, MA_TRUE);
    result = ma_sound_start(&bgm_sound);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Unable to start BGM '%s': %s\n",
                filename, ma_result_description(result));
    }
}

void play_se(const char* filename) {
    int index;
    ma_result result;

    if (!audio_start()) {
        return;
    }
    for (index = 0; index < AUDIO_SE_SLOTS; index++) {
        if (!se_ready[index] || !ma_sound_is_playing(&se_sounds[index])) {
            if (se_ready[index]) {
                ma_sound_uninit(&se_sounds[index]);
            }
            break;
        }
    }
    if (index == AUDIO_SE_SLOTS) {
        index = 0;
        if (se_ready[index]) {
            ma_sound_stop(&se_sounds[index]);
            ma_sound_uninit(&se_sounds[index]);
        }
    }
    result = ma_sound_init_from_file(&audio_engine, filename, 0, NULL, NULL,
                                     &se_sounds[index]);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Unable to load SE '%s': %s\n",
                filename, ma_result_description(result));
        return;
    }
    se_ready[index] = 1;
    result = ma_sound_start(&se_sounds[index]);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Unable to start SE '%s': %s\n",
                filename, ma_result_description(result));
        ma_sound_uninit(&se_sounds[index]);
        se_ready[index] = 0;
    }
}

void stop_bgm(void) {
    audio_stop_sound(&bgm_sound, &bgm_ready);
}

void audio_shutdown(void) {
    int index;

    if (!audio_ready) {
        return;
    }
    stop_bgm();
    for (index = 0; index < AUDIO_SE_SLOTS; index++) {
        if (se_ready[index]) {
            ma_sound_uninit(&se_sounds[index]);
            se_ready[index] = 0;
        }
    }
    ma_engine_uninit(&audio_engine);
    audio_ready = 0;
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
