#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* heder.h で宣言されているのでコメントアウトして動作確認
extern char name[];
extern char call_name[];
extern char user_select;
extern int level_of_story;
extern int money;
extern int result;
extern int ap;
extern int sum_ap;
extern int player_hp;
extern int player_rest_hp;
extern int can_item[];
extern int can_weapon[];
extern int can_magics[];
extern int cache;
extern int cache2;
extern int cache3[];
extern char cache_str[];
extern void echof(const char* format, ...);
extern void play_bgm(const char* filename);
extern void stop_bgm(void);
extern int battle(int e_id);
extern void change(void);
extern int item_shop(int first_id, ...);
extern int weapon_shop(int first_id, ...);
extern int all_shop(int first_id, ...);
extern void start_map_mode(const char* filename, int spawn_id, int has_enemy, ...);
*/

void set_up(void) {
    cls();
    play_bgm("set_up.mp3");
    echof("【Re:魔王討伐は簡単だった ～そんなことはない！～ 】へ ようこそ！（進むにはEnterキーを押してください）");
    echof("\nまずは名前を決めます。");

    do {
        printf("名前を入力してください：");
        scanf(" %1000s", name);
        while (getchar() != '\n');
        printf("%sでよろしいですね？y/n：", name);
        scanf(" %c", &user_select);
        while (getchar() != '\n');
    } while (user_select != 'y');

    echof("\n\n次に呼ばれ方を決めます。\n");
    do {
        printf("何て呼べばいいですか：");
        scanf(" %s", call_name);
        while (getchar() != '\n');
        printf("%sでよろしいですね？y/n：", call_name);
        scanf(" %c", &user_select);
        while (getchar() != '\n');
    } while (user_select != 'y');

    echof("\n\n最後に難易度を決めます\n");
    do {
        do {
            printf("難易度を1～3で選択してください（1がEasy,3がHardモードです）：");
            scanf(" %1d", &level_of_story);
            while (getchar() != '\n');
        } while (level_of_story < 1 || level_of_story > 3);

        printf("%dでよろしいですね？y/n：", level_of_story);
        scanf(" %c", &user_select);
        while (getchar() != '\n');
    } while (user_select != 'y');
}

void prologue(void) {
    cls();
    do {
        echof("...%s ..%s..! 起きて%s!!!!\n", name, name, name);
        printf("起きますか？y/n：");
        scanf(" %c", &user_select);
        while (getchar() != '\n');
    } while (user_select != 'y');

    echof("明日はあなたが魔王討伐に向けて旅立つ日でしょう。お母さん心配なの。");
    echof("だからあなたの腕試しをしておきたいの。");
    ap = 20;
    sum_ap = ap;
    can_item[1] = 1;
    stop_bgm();
    play_bgm("battle.mp3");
    result = battle(20);
    stop_bgm();
    play_bgm("set_up.mp3");

    if (result == 1) {
        echof("お母さんももう衰えたわね。これで安心して送り出せるわ。明日は早いからもう寝なさい。");
    }
    else {
        echof("私も勇者と結婚したほど強いってものよ。あなたもすぐ強くなれるわ。寝る子は育つ！明日は早いからもう寝なさい。");
    }

    do {
        printf("寝ますか？y/n：");
        scanf(" %c", &user_select);
        while (getchar() != '\n');
    } while (user_select != 'y');
}

int story_1(void) {
    cls();
    echof("おはよう！%s", call_name);
    echof("ついに旅立つ日が来たわね。これはこの村の人全員からの気持ちよ。");
    echof("\n300Gを受け取りました");
    money += 300;
    echof("G(ゴールド)はお金です。世界中のあらゆる場所で使えます。");
    echof("\n木の棒を受け取りました。");
    can_weapon[1] = 1;
    echof("木の棒は装備品です。装備してみましょう");
    change();
    echof("\nアイスの魔導書を受け取りました。");
    can_magics[1] = 1;
    echof("2.魔法 ＞1.アイス を選択して使えます。早速使ってみましょう");
    play_bgm("battle.mp3");
    battle(20);
    echof("使い方がわかったので町へ出てみてみましょう");
    can_item[1] = 1;
    start_map_mode("サイショームラ.txt", 0, -1);
    return 0;
}

int debug(void) {
    int i, j;
    echof("ここからは開発者モードです。");
    money = 10000000;
    item_shop(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, -1);
    weapon_shop(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, -1);
    all_shop(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1);

    player_hp = 10000;
    player_rest_hp = player_hp;
    for (i = 1; i < 18; i++) can_weapon[i] = 1;
    for (i = 1; i < 40; i++) can_magics[i] = 1;
    for (i = 1; i < 16; i++) can_item[i] = 20;

    echof("開発者モードにつき全ての武器、魔法、アイテムを所持しました。");
    for (;;) {
        printf("敵IDを入力してください、終了するには0を入力してください。：");
        scanf("%d", &i);
        if (i == 0) break;
        play_bgm("battle.mp3");
        while (getchar() != '\n');
        battle(i);
    }

    echof("マップのデバッグを行います。\n\n");
    while (1) {
        printf("マップファイル名を入力してください、終了するにはexitを入力してください。：");
        scanf(" %255s", cache_str);
        if (cache_str[0] == '\0') continue;
        if (strcmp(cache_str, "exit") == 0) break;

        while (1) {
            printf("敵の有無を選択してください（0:敵なし、1:敵あり）：");
            scanf("%d", &cache);
            while (getchar() != '\n');
            if (cache == 0 || cache == 1) break;
        }

        if (cache == 1) {
            i = 0;
            memset(cache3, 0, sizeof(cache3));
            while (1) {
                printf("現在登録中のID：");
                for (j = 0; j < i; j++) printf("%d ", cache3[j]);
                printf("\n");

                printf("敵IDを入力してください（終了するには-1を入力）：");
                if (scanf(" %d", &cache3[i]) != 1) { while (getchar() != '\n'); continue; }
                while (getchar() != '\n');

                if (cache3[i] == -1) break;
                if (i < 64) i++;
                else { printf("これ以上登録できません。\n"); break; }
            }
            start_map_mode(cache_str, 0, 1, cache3[0], cache3[1], cache3[2], cache3[3], cache3[4], cache3[5], cache3[6], cache3[7], cache3[8], cache3[9], -1);
        }
        else {
            start_map_mode(cache_str, 0, 0);
        }
    }
    return 0;
}