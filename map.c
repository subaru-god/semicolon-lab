#define TILE_EMPTY  0
#define TILE_WALL   1
#define TILE_EVENT  2

#define MAX_EVENT_DESCS 32
#define MAX_DESC_LEN    128

typedef struct {
    int width;
    int height;
    int** grid;
    char** event_id;
    int player_x;
    int player_y;
    char current_map_name[64];
    char event_descs[MAX_EVENT_DESCS][MAX_DESC_LEN];
    int event_desc_count;
} Map;

typedef struct {
    int id;
    int parent_id;
    int child_id;
    char name[64];
    int hp_heal;
    int mp_heal;
} MapItemData;

extern void echof(const char* format, ...);
extern int battle(int e_id);
extern int item_shop(int first_id, ...);
extern int weapon_shop(int first_id, ...);
extern int all_shop(int first_id, ...);
extern int can_item[];

static void set_cursor_pos(int x, int y) {
#ifdef _WIN32
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    printf("\033[%d;%dH", y + 1, x + 1);
#endif
}

static int read_map_key(void) {
#ifdef _WIN32
    return _getch();
#else
    struct termios original, raw;
    int key;

    if (tcgetattr(STDIN_FILENO, &original) == -1) {
        perror("tcgetattr");
        return getchar();
    }
    raw = original;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
        perror("tcsetattr");
        return getchar();
    }
    key = getchar();
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original) == -1) {
        perror("tcsetattr");
        return EOF;
    }
    return key;
#endif
}

static int is_event_symbol(char ch) {
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}

static int allocate_map(Map* map, int width, int height) {
    int y, i;
    map->width = width;
    map->height = height;

    map->grid = (int**)malloc(sizeof(int*) * height);
    map->event_id = (char**)malloc(sizeof(char*) * height);
    if (!map->grid || !map->event_id) return 0;

    for (y = 0; y < height; y++) {
        map->grid[y] = (int*)malloc(sizeof(int) * width);
        map->event_id[y] = (char*)malloc(sizeof(char) * width);
        if (!map->grid[y] || !map->event_id[y]) {
            for (i = 0; i < y; i++) {
                free(map->grid[i]); free(map->event_id[i]);
            }
            free(map->grid); free(map->event_id);
            return 0;
        }
        memset(map->grid[y], 0, sizeof(int) * width);
        memset(map->event_id[y], 0, sizeof(char) * width);
    }
    return 1;
}

static void free_map(Map* map) {
    int y;
    if (map->grid) {
        for (y = 0; y < map->height; y++) if (map->grid[y]) free(map->grid[y]);
        free(map->grid); map->grid = NULL;
    }
    if (map->event_id) {
        for (y = 0; y < map->height; y++) if (map->event_id[y]) free(map->event_id[y]);
        free(map->event_id); map->event_id = NULL;
    }
}

static int load_map_item_info(int item_id, MapItemData* out_item) {
    FILE* fp = fopen("アイテム.csv", "r");
    char line[256], * token;
    if (fp == NULL) return 0;
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        token = strtok(line, ",");
        if (token && atoi(token) == item_id) {
            out_item->id = item_id;
            token = strtok(NULL, ","); if (token) out_item->parent_id = atoi(token);
            token = strtok(NULL, ","); if (token) out_item->child_id = atoi(token);
            token = strtok(NULL, ","); if (token) strcpy(out_item->name, token);
            token = strtok(NULL, ","); if (token) out_item->hp_heal = atoi(token);
            token = strtok(NULL, ","); if (token) out_item->mp_heal = atoi(token);
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static void use_map_item(void) {
    MapItemData items[100];
    int count = 0, i, choice;
    cls();

    for (i = 1; i < 1000; i++) {
        if (can_item[i] >= 1) {
            MapItemData temp;
            if (load_map_item_info(i, &temp) && temp.parent_id == 2) {
                items[count++] = temp;
                if (count >= 100) break;
            }
        }
    }

    if (count == 0) {
        echof("\n移動中に使えるアイテムを持っていません！\n");
        cls();
        return;
    }

    printf("\n--- 移動用アイテム一覧 ---\n");
    for (i = 0; i < count; i++) {
        printf("%d. %s (所持数:%d) [回復 HP:%d / MP:%d]\n", i + 1, items[i].name, can_item[items[i].id], items[i].hp_heal, items[i].mp_heal);
    }
    printf("0. 戻る\n使用するアイテムを選択してください > ");

    if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); cls(); return; }
    while (getchar() != '\n');
    if (choice <= 0 || choice > count) { cls(); return; }

    choice--;
    echof("\n%s を使用した！ (HP:%d / MP:%d 回復)\n", items[choice].name, items[choice].hp_heal, items[choice].mp_heal);
    can_item[items[choice].id]--;
    cls();
}

void display_map(const Map* map) {
    int x, y, i;
    set_cursor_pos(0, 0);

    printf("==================== MAP ====================\n");
    for (y = 0; y < map->height; y++) {
        for (x = 0; x < map->width; x++) {
            if (x == map->player_x && y == map->player_y) printf("@");
            else {
                switch (map->grid[y][x]) {
                case TILE_WALL: printf("#"); break;
                case TILE_EVENT: printf("%c", map->event_id[y][x]); break;
                case TILE_EMPTY: default: printf(" "); break;
                }
            }
        }
        printf("\n");
    }
    printf("=============================================\n");

    if (map->event_desc_count > 0) {
        printf("--- イベント一覧 ---\n");
        for (i = 0; i < map->event_desc_count; i++) printf("%s\n", map->event_descs[i]);
        printf("--------------------\n");
    }
    printf("[W]:上  [S]:下  [A]:左  [D]:右  [Q]:アイテムを使う\n");
}

static void generate_random_map(Map* map, const char event_symbols[], int event_count, const char* event_descs[]) {
    int x, y, i, width = 20, height = 10, rx, ry;
    allocate_map(map, width, height);
    strcpy(map->current_map_name, "ランダムマップ");
    map->event_desc_count = 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            map->event_id[y][x] = '\0';
            if (x == 0 || x == width - 1 || y == 0 || y == height - 1) map->grid[y][x] = TILE_WALL;
            else map->grid[y][x] = (rand() % 100 < 15) ? TILE_WALL : TILE_EMPTY;
        }
    }

    map->player_x = 1;
    map->player_y = 1;
    map->grid[1][1] = TILE_EMPTY;

    for (i = 0; i < event_count; i++) {
        int attempts = 0;
        while (attempts < 100) {
            rx = rand() % (width - 2) + 1;
            ry = rand() % (height - 2) + 1;
            if ((rx != map->player_x || ry != map->player_y) && map->grid[ry][rx] == TILE_EMPTY) {
                map->grid[ry][rx] = TILE_EVENT;
                map->event_id[ry][rx] = event_symbols[i];
                if (event_descs[i] && map->event_desc_count < MAX_EVENT_DESCS) {
                    strncpy(map->event_descs[map->event_desc_count], event_descs[i], MAX_DESC_LEN - 1);
                    map->event_descs[map->event_desc_count][MAX_DESC_LEN - 1] = '\0';
                    map->event_desc_count++;
                }
                break;
            }
            attempts++;
        }
    }
}

int load_map_from_file(Map* map, const char* filename, int spawn_id) {
    FILE* fp = fopen(filename, "r");
    char line[512], spawn_char = (char)('0' + spawn_id);
    int max_w = 0, h = 0, len, x, y = 0, is_map_part = 1;

    if (fp == NULL) { printf("エラー: マップファイル '%s' が開けませんでした。\n", filename); return 0; }
    map->event_desc_count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        len = (int)strlen(line);
        if (len > 0 && (strchr(line, '#') || strchr(line, '.') || strchr(line, ' '))) {
            if (len > max_w) max_w = len;
            h++;
        }
    }

    if (max_w == 0 || h == 0 || !allocate_map(map, max_w, h)) { fclose(fp); return 0; }

    strcpy(map->current_map_name, filename);
    map->player_x = 1;
    map->player_y = 1;

    fseek(fp, 0, SEEK_SET);
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        len = (int)strlen(line);

        if (is_map_part && y < h && len > 0 && (strchr(line, '#') || strchr(line, '.') || strchr(line, ' '))) {
            for (x = 0; x < max_w; x++) {
                if (x < len) {
                    char ch = line[x];
                    map->event_id[y][x] = '\0';
                    if (ch == '#') map->grid[y][x] = TILE_WALL;
                    else if (is_event_symbol(ch)) { map->grid[y][x] = TILE_EVENT; map->event_id[y][x] = ch; }
                    else {
                        map->grid[y][x] = TILE_EMPTY;
                        if (ch == spawn_char) { map->player_x = x; map->player_y = y; }
                    }
                }
                else {
                    map->grid[y][x] = TILE_EMPTY;
                    map->event_id[y][x] = '\0';
                }
            }
            y++;
        }
        else {
            is_map_part = 0;
            if (len > 0 && map->event_desc_count < MAX_EVENT_DESCS) {
                strncpy(map->event_descs[map->event_desc_count], line, MAX_DESC_LEN - 1);
                map->event_descs[map->event_desc_count][MAX_DESC_LEN - 1] = '\0';
                map->event_desc_count++;
            }
        }
    }
    fclose(fp);
    return 1;
}

static int handle_event_point(Map* map, char symbol, const char* map_name) {
    char user_select;
    cls();
    switch (symbol) {
    case 'B':
        echof("お母さんと出会った！\n");
        do {
            printf("バトルの練習をしておく？ y/n：");
            user_select = (char)getchar();
            while (getchar() != '\n');
        } while (user_select != 'y' && user_select != 'n');

        if (user_select == 'y') battle(20);
        else echof("そう…");
        echof("頑張ってらっしゃい");
        break;
    case 'I':
        echof("アイテムショップへ入った！\n");
        if (strcmp(map_name, "サイショームラ_1.txt") == 0) item_shop(1, 2, 3, -1);
        break;
    case 'Q':
        echof("町を出た\n");
        return 0;
    case 'Y':
        echof("よろず屋へ入った！\n");
        if (strcmp(map_name, "サイショームラ_1.txt") == 0) all_shop(1, 2, 3, -1, 1, 2, 3, -1);
        break;
    case 'W':
        echof("武器屋へ入った！\n");
        if (strcmp(map_name, "サイショームラ_1.txt") == 0) weapon_shop(1, 2, 3, -1);
        break;
    default:
        echof("【%c】何も起こらなかった...\n", symbol);
        break;
    }
    cls();
    return 1;
}

int move_player(Map* map, char key, int has_enemy, const int enemy_ids[], int enemy_count) {
    int next_x = map->player_x, next_y = map->player_y;

    switch (key) {
    case 'w': case 'W': next_y--; break;
    case 's': case 'S': next_y++; break;
    case 'a': case 'A': next_x--; break;
    case 'd': case 'D': next_x++; break;
    case 'q': case 'Q': use_map_item(); return 1;
    default: return 1;
    }

    if (next_x < 0 || next_x >= map->width || next_y < 0 || next_y >= map->height) return 1;
    if (map->grid[next_y][next_x] == TILE_WALL) return 1;

    map->player_x = next_x;
    map->player_y = next_y;

    if (map->grid[next_y][next_x] == TILE_EVENT) {
        if (!handle_event_point(map, map->event_id[next_y][next_x], map->current_map_name)) return 0;
    }

    if (has_enemy && enemy_count > 0 && rand() % 100 < 5) {
        cls();
        echof("モンスターが現れた！\n");
        battle(enemy_ids[rand() % enemy_count]);
        cls();
    }
    return 1;
}

void start_map_mode(const char* filename, int spawn_id, int has_enemy, ...) {
    Map map;
    char key, event_symbols[32];
    const char* event_descs[32];
    int enemy_ids[64], enemy_count = 0, event_count = 0, eid;
    va_list args;

    va_start(args, has_enemy);
    if (has_enemy) {
        while ((eid = va_arg(args, int)) != -1) {
            enemy_ids[enemy_count++] = eid;
            if (enemy_count >= 64) break;
        }
    }

    cls();
    if (strcmp(filename, "random") == 0) {
        while (event_count < 32) {
            int sym = va_arg(args, int);
            if (sym == -1) break;
            event_symbols[event_count] = (char)sym;
            event_descs[event_count] = va_arg(args, char*);
            event_count++;
        }
        generate_random_map(&map, event_symbols, event_count, event_descs);
    }
    else {
        if (!load_map_from_file(&map, filename, spawn_id)) { va_end(args); getchar(); return; }
    }
    va_end(args);

    while (1) {
        display_map(&map);
        key = (char)read_map_key();
        if (!move_player(&map, key, has_enemy, enemy_ids, enemy_count)) break;
    }

    free_map(&map);
    cls();
}