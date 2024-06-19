#ifndef _MENU_H_
#define _MENU_H_
#include "lvgl.h"
#include "../managed_components/lvgl__lvgl/src/core/lv_event.h"
#include "gxhtc3.h"
#include "math.h"

#define MENUWORK BIT1

extern float temp;
extern float humi;
extern int temp_value, humi_value;
extern int icon_flag;
extern lv_obj_t *start_src;

extern void get_th_task(void *args);

void lv_gui_start(void);

void startbtn_event_cb(lv_event_t *e);

void back_to_main_menu(lv_event_t *e);

void menu_event_handler(lv_event_t *e);

void menu(void);

/* sub menu*/
void setting_menu(void);
void system_menu(void);
void bluetooth_menu(void);
void network_menu(void);
void application_menu(void);
void account_menu(void);
void time_menu(void);
void game_menu(void);
void policy_menu(void);
void update_menu(void);

#endif