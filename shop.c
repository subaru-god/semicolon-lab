typedef struct {
    int id, parent_id, child_id, hp_heal, mp_heal, price;
    char name[64];
} ShopItemData;

typedef struct {
    int id, parent_id, child_id, power_easy, power_normal, power_hard, price;
    char name[64];
} ShopWeaponData;

extern int money;
extern int can_item[];
extern int can_weapon[];
extern int level_of_story;
extern void echof(const char* format, ...);

static int load_shop_item_info(int item_id, ShopItemData* out_item) {
    FILE* fp = fopen("アイテム.csv", "r");
    char line[256], * token;
    if (fp == NULL) return 0;
    if (fgets(line, sizeof(line), fp) == NULL) { fclose(fp); return 0; }

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
            token = strtok(NULL, ","); out_item->price = (token && strlen(token) > 0) ? atoi(token) : 100;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static int load_shop_weapon_info(int weapon_id, ShopWeaponData* out_weapon) {
    FILE* fp = fopen("武器.csv", "r");
    char line[512], * token;
    int k;
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
            for (k = 0; k < 4; k++) token = strtok(NULL, ",");
            out_weapon->price = (token && strlen(token) > 0) ? atoi(token) : 200;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static void print_item_table(const ShopItemData items[], int count) {
    int i;
    char heal_buf[64];
    printf("======================================================================\n");
    printf("                         道具屋 (所持金: %d G)\n", money);
    printf("======================================================================\n");
    printf("  No. | %-20s | %-8s | %-6s | %-18s\n", "アイテム名", "価格", "所持数", "回復効果");
    printf("------+----------------------+----------+--------+--------------------\n");

    for (i = 0; i < count; i++) {
        heal_buf[0] = '\0';
        if (items[i].hp_heal > 0 && items[i].mp_heal > 0) sprintf(heal_buf, "HP:+%d MP:+%d", items[i].hp_heal, items[i].mp_heal);
        else if (items[i].hp_heal > 0) sprintf(heal_buf, "HP:+%d", items[i].hp_heal);
        else if (items[i].mp_heal > 0) sprintf(heal_buf, "MP:+%d", items[i].mp_heal);
        else strcpy(heal_buf, "なし");

        printf("  %2d  | %-20s | %6d G | %4d 個 | %-18s\n", i + 1, items[i].name, items[i].price, can_item[items[i].id], heal_buf);
    }
    printf("   0  | 店を出る\n");
    printf("======================================================================\n");
}

static void print_weapon_table(const ShopWeaponData weapons[], int count) {
    int i;
    int pwr;
    printf("======================================================================\n");
    printf("                         武器屋 (所持金: %d G)\n", money);
    printf("======================================================================\n");
    printf("  No. | %-20s | %-8s | %-6s | %-10s\n", "武器名", "価格", "威力", "状態");
    printf("------+----------------------+----------+--------+--------------------\n");

    for (i = 0; i < count; i++) {
        pwr = weapons[i].power_normal;
        if (level_of_story == 1) pwr = weapons[i].power_easy;
        else if (level_of_story == 3) pwr = weapons[i].power_hard;

        printf("  %2d  | %-20s | %6d G | %6d | %-10s\n", i + 1, weapons[i].name, weapons[i].price, pwr, can_weapon[weapons[i].id] ? "所持済み" : "未所持");
    }
    printf("   0  | 店を出る\n");
    printf("======================================================================\n");
}

int item_shop(int first_id, ...) {
    ShopItemData items[100];
    int count = 0, id, choice;
    va_list args;

    if (first_id != -1) {
        if (load_shop_item_info(first_id, &items[count])) count++;
        va_start(args, first_id);
        while ((id = va_arg(args, int)) != -1) {
            if (count < 100 && load_shop_item_info(id, &items[count])) count++;
        }
        va_end(args);
    }

    if (count == 0) { echof("売り出すアイテムがありません。\n"); return 0; }
    cls();
    echof("いらっしゃいませ！\nアイテムショップへようこそ");

    while (1) {
        cls();
        print_item_table(items, count);
        printf("購入するアイテムを選択してください > ");
        if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); continue; }
        while (getchar() != '\n');

        if (choice == 0) break;
        if (choice < 1 || choice > count) continue;

        choice--;
        if (money < items[choice].price) { echof("\nお金が足りません！\n"); cls(); }
        else {
            money -= items[choice].price;
            can_item[items[choice].id]++;
            echof("\n%s を購入しました！\n", items[choice].name);
            cls();
        }
    }
    cls();
    return 1;
}

int weapon_shop(int first_id, ...) {
    ShopWeaponData weapons[100];
    int count = 0, id, choice;
    va_list args;

    if (first_id != -1) {
        if (load_shop_weapon_info(first_id, &weapons[count])) count++;
        va_start(args, first_id);
        while ((id = va_arg(args, int)) != -1) {
            if (count < 100 && load_shop_weapon_info(id, &weapons[count])) count++;
        }
        va_end(args);
    }

    if (count == 0) { echof("売り出す武器がありません。\n"); return 0; }
    cls();
    echof("いらっしゃいませ！\n武器ショップへようこそ");

    while (1) {
        cls();
        print_weapon_table(weapons, count);
        printf("購入する武器を選択してください > ");
        if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); continue; }
        while (getchar() != '\n');

        if (choice == 0) break;
        if (choice < 1 || choice > count) continue;

        choice--;
        if (can_weapon[weapons[choice].id]) { echof("\n既に所持しています！\n"); cls(); }
        else if (money < weapons[choice].price) { echof("\nお金が足りません！\n"); cls(); }
        else {
            money -= weapons[choice].price;
            can_weapon[weapons[choice].id] = 1;
            echof("\n%s を購入しました！\n", weapons[choice].name);
            cls();
        }
    }
    cls();
    return 1;
}

int all_shop(int first_item_id, ...) {
    ShopItemData items[100];
    ShopWeaponData weapons[100];
    int item_count = 0, weapon_count = 0, id, choice, select_type;
    va_list args;

    va_start(args, first_item_id);
    if (first_item_id != -1) {
        if (load_shop_item_info(first_item_id, &items[item_count])) item_count++;
        while ((id = va_arg(args, int)) != -1) {
            if (item_count < 100 && load_shop_item_info(id, &items[item_count])) item_count++;
        }
    }
    while ((id = va_arg(args, int)) != -1) {
        if (weapon_count < 100 && load_shop_weapon_info(id, &weapons[weapon_count])) weapon_count++;
    }
    va_end(args);

    cls();
    echof("いらっしゃいませ！\nよろず屋へようこそ");

    while (1) {
        cls();
        printf("==================================================\n");
        printf("               よろず屋 (所持金: %d G)\n", money);
        printf("==================================================\n");
        printf("  1. アイテムを買う\n  2. 武器を買う\n  0. 店を出る\n");
        printf("==================================================\n選択してください > ");

        if (scanf("%d", &select_type) != 1) { while (getchar() != '\n'); continue; }
        while (getchar() != '\n');

        if (select_type == 0) break;

        if (select_type == 1) {
            while (1) {
                cls();
                print_item_table(items, item_count);
                printf("購入するアイテムを選択してください > ");
                if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); continue; }
                while (getchar() != '\n');
                if (choice == 0) break;
                if (choice < 1 || choice > item_count) continue;
                choice--;
                if (money < items[choice].price) echof("\nお金が足りません！\n");
                else {
                    money -= items[choice].price;
                    can_item[items[choice].id]++;
                    echof("\n%s を購入しました！\n", items[choice].name);
                }
            }
        }
        else if (select_type == 2) {
            while (1) {
                cls();
                print_weapon_table(weapons, weapon_count);
                printf("購入する武器を選択してください > ");
                if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); continue; }
                while (getchar() != '\n');
                if (choice == 0) break;
                if (choice < 1 || choice > weapon_count) continue;
                choice--;
                if (can_weapon[weapons[choice].id]) echof("\n既に所持しています！\n");
                else if (money < weapons[choice].price) echof("\nお金が足りません！\n");
                else {
                    money -= weapons[choice].price;
                    can_weapon[weapons[choice].id] = 1;
                    echof("\n%s を購入しました！\n", weapons[choice].name);
                }
            }
        }
    }
    cls();
    return 1;
}