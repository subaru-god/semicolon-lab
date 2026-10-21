typedef struct {
    int id;
    char name[32];
    int weak_id;
    int strong_id;
} ElementRelation;

typedef struct {
    int id;
    int parent_id;
    int child_id;
    char name[64];
    int hp_heal;
    int mp_heal;
} ItemData;

typedef struct {
    int id;
    int parent_id;
    int child_id;
    char name[64];
    int mp_cost;
    float power_rate;
    int buff_turns;
    int is_buff;
} SkillData;

typedef struct {
    int id;
    char name[64];
    int hp;
    int mp;
    int element_id;
    int skill_ids[4];
    float skill_probs[4];
} EnemyData;

typedef struct {
    int id;
    int parent_id;
    int child_id;
    char name[64];
    int power_easy;
    int power_normal;
    int power_hard;
    int boost_elem_id;
    float elem_rate;
    char effective_target[64];
    float effective_rate;
} WeaponEffectData;

typedef struct {
    float atk_rate;
    int atk_turns;
    float def_rate;
    int def_turns;
    float mag_rate;
    int mag_turns;
} BuffStatus;

extern char name[];
extern int can_magics[];
extern int can_item[];
extern int sum_ap;
extern int level_of_story;
extern int current_weapon_id;
extern int player_hp;
extern int player_rest_hp;
extern void echof(const char* format, ...);
extern void change(void);

static int load_element_relation(int elem_id, ElementRelation* out_elem) {
    FILE* fp = fopen("属性対応.csv", "r");
    char line[256];
    char* token;

    if (!fp) return 0;
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        token = strtok(line, ",");
        if (token && atoi(token) == elem_id) {
            out_elem->id = elem_id;
            token = strtok(NULL, ","); if (token) strcpy(out_elem->name, token);
            token = strtok(NULL, ","); out_elem->weak_id = (token && strlen(token) > 0) ? atoi(token) : 0;
            token = strtok(NULL, ","); out_elem->strong_id = (token && strlen(token) > 0) ? atoi(token) : 0;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static float get_element_multiplier(int atk_elem_id, int def_elem_id) {
    ElementRelation atk_rel;
    if (atk_elem_id <= 0 || def_elem_id <= 0) return 1.0f;

    if ((atk_elem_id == 6 && def_elem_id == 7) || (atk_elem_id == 7 && def_elem_id == 6)) {
        return 1.5f;
    }

    if (load_element_relation(atk_elem_id, &atk_rel)) {
        if (atk_rel.strong_id == def_elem_id) {
            return 1.5f;
        }
    }
    return 1.0f;
}

static int load_item_info(int item_id, ItemData* out_item) {
    FILE* fp = fopen("アイテム.csv", "r");
    char line[256], * token;
    if (!fp) return 0;
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        token = strtok(line, ",");
        if (token && atoi(token) == item_id) {
            out_item->id = item_id;
            token = strtok(NULL, ","); if (token) out_item->parent_id = atoi(token);
            token = strtok(NULL, ","); if (token) out_item->child_id = atoi(token);
            token = strtok(NULL, ","); if (token) strcpy(out_item->name, token);
            token = strtok(NULL, ","); if (token) out_item->hp_heal = atoi(token);
            token = strtok(NULL, ","); if (token) out_item->mp_heal = atoi(token);
            fclose(fp); return 1;
        }
    }
    fclose(fp); return 0;
}

static int load_weapon_effect(int weapon_id, WeaponEffectData* out_wep) {
    FILE* fp = fopen("武器.csv", "r");
    char line[512], * token;
    if (weapon_id <= 0 || !fp) return 0;
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        token = strtok(line, ",");
        if (token && atoi(token) == weapon_id) {
            out_wep->id = weapon_id;
            token = strtok(NULL, ","); if (token) out_wep->parent_id = atoi(token);
            token = strtok(NULL, ","); if (token) out_wep->child_id = atoi(token);
            token = strtok(NULL, ","); if (token) strcpy(out_wep->name, token);
            token = strtok(NULL, ","); if (token) out_wep->power_easy = atoi(token);
            token = strtok(NULL, ","); if (token) out_wep->power_normal = atoi(token);
            token = strtok(NULL, ","); if (token) out_wep->power_hard = atoi(token);
            token = strtok(NULL, ","); out_wep->boost_elem_id = (token && strlen(token) > 0) ? atoi(token) : 0;
            token = strtok(NULL, ","); out_wep->elem_rate = (token && strlen(token) > 0) ? (float)atof(token) : 1.0f;
            token = strtok(NULL, ",");
            if (token && strlen(token) > 0) strcpy(out_wep->effective_target, token);
            else out_wep->effective_target[0] = '\0';
            token = strtok(NULL, ","); out_wep->effective_rate = (token && strlen(token) > 0) ? (float)atof(token) : 1.0f;

            fclose(fp); return 1;
        }
    }
    fclose(fp); return 0;
}

static int load_enemy_info(int target_id, EnemyData* out_enemy) {
    FILE* fp = fopen("敵.csv", "r");
    char line[512], * token;
    int i;
    if (!fp) return 0;
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        token = strtok(line, ",");
        if (token && atoi(token) == target_id) {
            out_enemy->id = target_id;
            token = strtok(NULL, ","); if (token) strcpy(out_enemy->name, token);
            token = strtok(NULL, ","); if (token) out_enemy->hp = atoi(token);
            token = strtok(NULL, ","); if (token) out_enemy->mp = atoi(token);
            token = strtok(NULL, ","); if (token) out_enemy->element_id = atoi(token);
            for (i = 0; i < 4; i++) {
                token = strtok(NULL, ","); out_enemy->skill_ids[i] = token ? atoi(token) : 0;
                token = strtok(NULL, ","); out_enemy->skill_probs[i] = token ? (float)atof(token) : 0.0f;
            }
            fclose(fp); return 1;
        }
    }
    fclose(fp); return 0;
}

static void parse_power_and_turns(const char* str, float* out_power, int* out_turns, int parent_id) {
    char buf[64];
    char* comma;

    *out_power = 0.0f;
    *out_turns = 0;
    if (!str || strlen(str) == 0) return;

    strncpy(buf, str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    if (buf[0] == '"') memmove(buf, buf + 1, strlen(buf));
    if (buf[strlen(buf) - 1] == '"') buf[strlen(buf) - 1] = '\0';

    comma = strchr(buf, ',');
    if (comma) {
        *comma = '\0';
        *out_power = (float)atof(buf);
        *out_turns = atoi(comma + 1);
    }
    else {
        *out_power = (float)atof(buf);
        *out_turns = 0;
    }

    if (parent_id >= 9 && parent_id <= 12 && *out_turns == 0) {
        *out_turns = 3;
    }
}

static int load_skill_info(int skill_id, SkillData* out_skill) {
    FILE* fp = fopen("技一覧.csv", "r");
    char line[512], * token;
    char power_str[64] = "";
    if (!fp) return 0;
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        token = strtok(line, ",");
        if (token && atoi(token) == skill_id) {
            out_skill->id = skill_id;
            token = strtok(NULL, ","); if (token) out_skill->parent_id = atoi(token);
            token = strtok(NULL, ","); if (token) out_skill->child_id = atoi(token);
            token = strtok(NULL, ","); if (token) strcpy(out_skill->name, token);
            token = strtok(NULL, ","); if (token) out_skill->mp_cost = atoi(token);
            token = strtok(NULL, ","); if (token) strcpy(power_str, token);

            parse_power_and_turns(power_str, &out_skill->power_rate, &out_skill->buff_turns, out_skill->parent_id);
            out_skill->is_buff = (out_skill->parent_id >= 9 && out_skill->parent_id <= 12);

            fclose(fp); return 1;
        }
    }
    fclose(fp); return 0;
}

static void update_buff_status(BuffStatus* buff, const char* entity_name) {
    if (buff->atk_turns > 0 && --buff->atk_turns == 0) {
        buff->atk_rate = 1.0f;
        echof("%s の攻撃力アップ効果が切れた。\n", entity_name);
    }
    if (buff->def_turns > 0 && --buff->def_turns == 0) {
        buff->def_rate = 1.0f;
        echof("%s の防御力アップ効果が切れた。\n", entity_name);
    }
    if (buff->mag_turns > 0 && --buff->mag_turns == 0) {
        buff->mag_rate = 1.0f;
        echof("%s の魔法効果アップ効果が切れた。\n", entity_name);
    }
}

static void apply_buff(BuffStatus* buff, int parent_id, float rate, int turns, const char* entity_name) {
    switch (parent_id) {
    case 9:
        buff->atk_rate = rate; buff->atk_turns = turns;
        echof("%s の攻撃力が %.1f 倍に上がった！ (%dターン持続)\n", entity_name, rate, turns);
        break;
    case 10:
        buff->def_rate = rate; buff->def_turns = turns;
        echof("%s の防御力が %.1f 倍に上がった！ (%dターン持続)\n", entity_name, rate, turns);
        break;
    case 11:
        buff->atk_rate = rate; buff->atk_turns = turns;
        buff->def_rate = rate; buff->def_turns = turns;
        echof("%s の攻撃力と防御力が %.1f 倍に上がった！ (%dターン持続)\n", entity_name, rate, turns);
        break;
    case 12:
        buff->mag_rate = rate; buff->mag_turns = turns;
        echof("%s の魔法効果が %.1f 倍に上がった！ (%dターン持続)\n", entity_name, rate, turns);
        break;
    }
}

static int select_magic_phase(int* e_rest_hp, const char* e_name, int* player_mp, int enemy_element_id, BuffStatus* p_buff, BuffStatus* e_buff) {
    SkillData learned_skills[100];
    WeaponEffectData wep;
    int count = 0, i, choice, damage, random;
    float mult = 1.0f;

    for (i = 1; i < 1000; i++) {
        if (can_magics[i] == 1 && load_skill_info(i, &learned_skills[count])) {
            if (++count >= 100) break;
        }
    }
    if (count == 0) { echof("\n魔法・特技を覚えていません。\n"); return 0; }

    printf("\n--- 魔法・特技選択 ---\n");
    for (i = 0; i < count; i++) {
        if (learned_skills[i].is_buff) {
            printf("%d. %s (必要MP: %d / 効果: ×%.2f %dターン)\n", i + 1, learned_skills[i].name, learned_skills[i].mp_cost, learned_skills[i].power_rate, learned_skills[i].buff_turns);
        }
        else {
            printf("%d. %s (必要MP: %d / 威力: %.0f)\n", i + 1, learned_skills[i].name, learned_skills[i].mp_cost, learned_skills[i].power_rate);
        }
    }
    printf("0. やめる\n選択 > ");
    if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); return 0; }
    while (getchar() != '\n');
    if (choice <= 0 || choice > count) return 0;
    choice--;

    if (*player_mp < learned_skills[choice].mp_cost) {
        echof("\nMPが不足しています。\n"); return 0;
    }
    *player_mp -= learned_skills[choice].mp_cost;

    if (learned_skills[choice].is_buff) {
        echof("\n%sの%s！\n", name, learned_skills[choice].name);
        apply_buff(p_buff, learned_skills[choice].parent_id, learned_skills[choice].power_rate, learned_skills[choice].buff_turns, name);
        return 1;
    }

    random = rand() % 30 + 70;
    damage = (int)(learned_skills[choice].power_rate * random / 100.0f * p_buff->mag_rate);
    mult *= get_element_multiplier(learned_skills[choice].parent_id, enemy_element_id);

    if (load_weapon_effect(current_weapon_id, &wep)) {
        if ((wep.boost_elem_id == -1 && learned_skills[choice].parent_id != 0) || (wep.boost_elem_id > 0 && wep.boost_elem_id == learned_skills[choice].parent_id)) {
            mult *= wep.elem_rate;
        }
        if (strlen(wep.effective_target) > 0 && strstr(e_name, wep.effective_target) != NULL) {
            mult *= wep.effective_rate;
        }
    }

    damage = (int)(damage * mult);
    if (e_buff->def_rate > 1.0f) damage = (int)(damage / e_buff->def_rate);

    echof("\n%sの%s！\n%sに %d のダメージ！(倍率: %.2f倍)\n", name, learned_skills[choice].name, e_name, damage, mult);
    *e_rest_hp -= damage;
    return 1;
}

static int select_item_phase_battle(int* player_mp, int max_mp) {
    ItemData items[100];
    int count = 0, i, choice;

    for (i = 1; i < 1000; i++) {
        if (can_item[i] >= 1) {
            ItemData temp;
            if (load_item_info(i, &temp) && temp.parent_id == 1) {
                items[count++] = temp;
                if (count >= 100) break;
            }
        }
    }
    if (count == 0) { echof("\n戦闘中に使えるアイテムを持っていません！\n"); return 0; }

    printf("\n--- 戦闘用アイテム一覧 ---\n");
    for (i = 0; i < count; i++) {
        printf("%d. %s (所持数:%d) [回復 HP:%d / MP:%d]\n", i + 1, items[i].name, can_item[items[i].id], items[i].hp_heal, items[i].mp_heal);
    }
    printf("0. 戻る\nアイテムを選択 > ");
    if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); return 0; }
    while (getchar() != '\n');
    if (choice <= 0 || choice > count) return 0;
    choice--;

    if (items[choice].hp_heal > 0) {
        player_rest_hp += items[choice].hp_heal;
        if (player_rest_hp > player_hp) player_rest_hp = player_hp;
    }
    if (items[choice].mp_heal > 0) {
        *player_mp += items[choice].mp_heal;
        if (*player_mp > max_mp) *player_mp = max_mp;
    }

    echof("\n%s を使用した！ (HP:%d / MP:%d 回復)\n", items[choice].name, items[choice].hp_heal, items[choice].mp_heal);
    can_item[items[choice].id]--;
    return 1;
}

static void enemy_turn_action(const EnemyData* enemy, BuffStatus* e_buff, BuffStatus* p_buff) {
    float r = (float)rand() / RAND_MAX;
    float cumulative = 0.0f;
    int selected_skill_id = 0, i, damage;
    SkillData e_skill;

    for (i = 0; i < 4; i++) {
        if (enemy->skill_ids[i] > 0 && enemy->skill_probs[i] > 0.0f) {
            cumulative += enemy->skill_probs[i];
            if (r <= cumulative) {
                selected_skill_id = enemy->skill_ids[i];
                break;
            }
        }
    }

    if (selected_skill_id > 0 && load_skill_info(selected_skill_id, &e_skill)) {
        if (e_skill.is_buff) {
            echof("%sは %s を唱えた！\n", enemy->name, e_skill.name);
            apply_buff(e_buff, e_skill.parent_id, e_skill.power_rate, e_skill.buff_turns, enemy->name);
        }
        else {
            damage = (int)(e_skill.power_rate * (rand() % 30 + 70) / 100.0f * e_buff->mag_rate);
            damage = (int)(damage * get_element_multiplier(e_skill.parent_id, 8));
            if (p_buff->def_rate > 1.0f) damage = (int)(damage / p_buff->def_rate);

            echof("%sは %s を放った！ %sは %d のダメージを受けた！\n", enemy->name, e_skill.name, name, damage);
            player_rest_hp -= damage;
        }
    }
    else {
        damage = (int)((rand() % 10 + 5) * e_buff->atk_rate);
        if (p_buff->def_rate > 1.0f) damage = (int)(damage / p_buff->def_rate);
        echof("%sの攻撃！ %sは %d のダメージを受けた！\n", enemy->name, name, damage);
        player_rest_hp -= damage;
    }
}

int battle(int e_id) {
    EnemyData enemy;
    int e_hp, e_rest_hp, max_mp = 50, player_mp = 50;
    char select;
    int rnd1, rnd2, damage, turn_acted;
    BuffStatus p_buff = { 1.0f, 0, 1.0f, 0, 1.0f, 0 };
    BuffStatus e_buff = { 1.0f, 0, 1.0f, 0, 1.0f, 0 };

    srand((unsigned int)time(NULL));

    if (!load_enemy_info(e_id, &enemy)) {
        strcpy(enemy.name, "スライム");
        enemy.hp = 30;
        enemy.element_id = 1;
        enemy.skill_ids[0] = 0; enemy.skill_probs[0] = 1.0f;
    }
    e_hp = enemy.hp;
    e_rest_hp = e_hp;

    cls();
    echof("==================== \n %s が現れた!\n==================== \n\n", enemy.name);

    while (e_rest_hp > 0 && player_rest_hp > 0) {
        cls();
        printf("\n----------------------------------------\n");
        printf("%s HP %d/%d  VS  %s HP %d/%d (MP:%d/%d)\n", enemy.name, e_rest_hp, e_hp, name, player_rest_hp, player_hp, player_mp, max_mp);

        if (p_buff.atk_turns > 0 || p_buff.def_turns > 0 || p_buff.mag_turns > 0) {
            printf("[自分バフ] ");
            if (p_buff.atk_turns > 0) printf("攻×%.1f(%dターン) ", p_buff.atk_rate, p_buff.atk_turns);
            if (p_buff.def_turns > 0) printf("防×%.1f(%dターン) ", p_buff.def_rate, p_buff.def_turns);
            if (p_buff.mag_turns > 0) printf("魔×%.1f(%dターン) ", p_buff.mag_rate, p_buff.mag_turns);
            printf("\n");
        }
        if (e_buff.atk_turns > 0 || e_buff.def_turns > 0 || e_buff.mag_turns > 0) {
            printf("[敵バフ] ");
            if (e_buff.atk_turns > 0) printf("攻×%.1f(%dターン) ", e_buff.atk_rate, e_buff.atk_turns);
            if (e_buff.def_turns > 0) printf("防×%.1f(%dターン) ", e_buff.def_rate, e_buff.def_turns);
            if (e_buff.mag_turns > 0) printf("魔×%.1f(%dターン) ", e_buff.mag_rate, e_buff.mag_turns);
            printf("\n");
        }
        printf("----------------------------------------\n");

        do {
            printf("何をする？\n１.通常攻撃\n２.魔法・特技\n３.アイテム\n４.装備変更\n５.逃げる\n入力 > ");
            select = (char)getchar();
            while (getchar() != '\n');
        } while (select < '1' || select > '5');

        turn_acted = 0;

        switch (select) {
        case '1':
            rnd1 = rand() % 100 + 1;
            rnd2 = rand() % 30 + 70;
            damage = (int)((sum_ap * rnd2 / 100.0f) * p_buff.atk_rate);

            if (rnd1 >= 78) {
                printf("\nクリティカル攻撃！！\n");
                damage *= 2;
            }
            else {
                printf("\n通常攻撃！\n");
            }

            {
                WeaponEffectData wep;
                if (load_weapon_effect(current_weapon_id, &wep) && strlen(wep.effective_target) > 0 && strstr(enemy.name, wep.effective_target) != NULL) {
                    damage = (int)(damage * wep.effective_rate);
                    printf("【特効発動！】");
                }
            }

            if (e_buff.def_rate > 1.0f) damage = (int)(damage / e_buff.def_rate);

            echof("%sに %d ダメージを与えた！\n", enemy.name, damage);
            e_rest_hp -= damage;
            turn_acted = 1;
            break;

        case '2':
            turn_acted = select_magic_phase(&e_rest_hp, enemy.name, &player_mp, enemy.element_id, &p_buff, &e_buff);
            break;

        case '3':
            turn_acted = select_item_phase_battle(&player_mp, max_mp);
            break;

        case '4':
            change();
            break;

        case '5':
            echof("\n%sは逃げ出した！\n", name);
            cls();
            return 0;
        }

        if (turn_acted && e_rest_hp > 0) {
            enemy_turn_action(&enemy, &e_buff, &p_buff);
            update_buff_status(&p_buff, name);
            update_buff_status(&e_buff, enemy.name);
        }
    }

    if (e_rest_hp <= 0) {
        echof("\n%sを倒した！勝利！\n", enemy.name);
        cls();
        return 1;
    }
    else {
        echof("\n%sは倒れてしまった...\n", name);
        cls();
        return 0;
    }
}