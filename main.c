#ifndef _BORLANDC_
#pragma comment(lib, "winmm.lib")
#endif

char user_select;
char name[10000];
char call_name[10000];
int level_of_story;
int money;
int random;
int random2;
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

#include "header.h"

int main(void) {
    time(NULL);
    set_up();
    prologue();
    story_1();
    debug();

    return 0;
}