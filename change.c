#include "header.h"

/* heder.h で宣言されているのでコメントアウトして動作確認
extern int can_weapon[];
extern int level_of_story;
extern int sum_ap;
extern int current_weapon_id;
extern int ap;
extern void echof(const char* format, ...);
*/

typedef struct {
    int id;
    int parent_id;
    int child_id;
    char name[64];
    int power_easy;
    int power_normal;
    int power_hard;
    char remark[128];
} WeaponData;

/* 引数: weapon_id = CSVの武器ID、out_weapon = 読み込み先 */
static int load_weapon_info(int weapon_id, WeaponData* out_weapon) {
    FILE* fp = fopen("武器.csv", "r");
    char line[256];
    char* token;

    if (fp == NULL) return 0;
    if (fgets(line, sizeof(line), fp) == NULL) { fclose(fp); return 0; }

    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        token = strtok(line, ",");
        if (token && atoi(token) == weapon_id) {
            out_weapon->id = weapon_id;
            token = strtok(NULL, ","); if (token) out_weapon->parent_id = atoi(token);
            token = strtok(NULL, ","); if (token) out_weapon->child_id = atoi(token);
            token = strtok(NULL, ","); if (token) strcpy(out_weapon->name, token);
            token = strtok(NULL, ","); if (token) out_weapon->power_easy = atoi(token);
            token = strtok(NULL, ","); if (token) out_weapon->power_normal = atoi(token);
            token = strtok(NULL, ","); if (token) out_weapon->power_hard = atoi(token);
            token = strtok(NULL, ",");
            if (token) {
                strncpy(out_weapon->remark, token, sizeof(out_weapon->remark) - 1);
                out_weapon->remark[sizeof(out_weapon->remark) - 1] = '\0';
            }
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void change(void) {
    WeaponData available_weapons[100];
    int count = 0, i, choice, selected_power;

    for (i = 1; i < 1000; i++) {
        if (can_weapon[i] == 1 && load_weapon_info(i, &available_weapons[count])) {
            if (++count >= 100) break;
        }
    }
    cls();
    if (count == 0) {
        echof("\n変更できる装備を持っていません！\n");
        cls();
        return;
    }

    printf("\n--- 装備選択 ---\n");
    for (i = 0; i < count; i++) {
        int pwr = available_weapons[i].power_normal;
        if (level_of_story == 1) pwr = available_weapons[i].power_easy;
        else if (level_of_story == 3) pwr = available_weapons[i].power_hard;

        printf("%d. %s (威力: %d)\n", i + 1, available_weapons[i].name, pwr);
    }
    printf("0. 戻る\n装備を選択してください > ");

    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        cls();
        return;
    }
    while (getchar() != '\n');

    if (choice <= 0 || choice > count) {
        cls();
        return;
    }

    choice--;
    current_weapon_id = available_weapons[choice].id;

    if (level_of_story == 1) selected_power = available_weapons[choice].power_easy;
    else if (level_of_story == 2) selected_power = available_weapons[choice].power_normal;
    else if (level_of_story == 3) selected_power = available_weapons[choice].power_hard;
    else selected_power = available_weapons[choice].power_normal;

    sum_ap = selected_power + ap;
    echof("\n%s を装備しました！（攻撃力: %d）\n", available_weapons[choice].name, sum_ap);
    cls();
}