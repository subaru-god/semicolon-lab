char user_select;
char name[10000];
char call_name[10000];
int level_of_story;
int money;
int random_number;
int random_number2;
int cache;
int cache2;
int cache3[10000];
char cache_str[10000];
int result;
int ap;
int sum_ap;
int m_ap;
int m_sum_ap;
int current_weapon_id;
int can_magics[1000];
int can_weapon[1000];
int can_item[1000];
int player_hp = 100;
int player_rest_hp = 100;
int player_max_mp = 50;
int player_mp = 50;

#include "header.h"

#include "story.c"
#include "change.c"
#include "sys.c"
#include "battle.c"
#include "shop.c"
#include "map.c"

int main(int argc, char* argv[])
{

    int debug_mode = 0;

    if (argc == 2 && strcmp(argv[1], "1") == 0) {
        debug_mode = 1;
    }
    set_cursor_visible(1);
    if (debug_mode == 1) {
        echof("デバッグモードを有効にしました。");
		strcpy(name, "テスト");
		strcpy(call_name, "テスト");
		level_of_story = 1;
        can_item[1] = 1;
        start_map_mode("サイショームラ_1.txt", 0, 0);
        return 0;
    }
    configure_console_encoding();
    time(NULL);
    set_up();
    prologue();
    story_1();
    if (debug_mode == 1) {
        debug();
    }
    echof("ゲーム開始からここまで%d秒経過しました。", (int)time(NULL));
    echof("ここまでお疲れ様でした。");
    echof("ゲームを終了します。");
    return 0;
}