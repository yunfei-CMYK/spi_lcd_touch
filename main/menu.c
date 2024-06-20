#include "menu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "example";

/********************************返回按钮回调函数***************************/
lv_obj_t *submenu;
lv_timer_t *my_lv_timer;
int icon_flag;

static void my_gesture_event_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_TOP)
    {
        if ((icon_flag == 1) || (icon_flag == 2) || (icon_flag == 4) || (icon_flag == 5))
        {
            lv_timer_del(my_lv_timer);
        }
        lv_obj_del(submenu);
        icon_flag = 0;
    }
}

/*********************************温湿度显示******************************/

lv_obj_t *temp_label;
lv_obj_t *humi_label;

int temp_value, humi_value; // 室内实时温湿度值

// 定时更新温湿度值
void thv_update_cb(lv_timer_t *timer)
{
    lv_label_set_text_fmt(temp_label, "Temperature: %d°C", temp_value);
    lv_label_set_text_fmt(humi_label, "Humidity: %d%%", humi_value);
}
void wenshidu_menu(void)
{
    // 重新创建一个面板对象
    static lv_style_t style_wenshidu_menu;
    lv_style_init(&style_wenshidu_menu);
    lv_style_set_bg_color(&style_wenshidu_menu, lv_color_hex(0x003a57));
    lv_style_set_width(&style_wenshidu_menu, 320);
    lv_style_set_height(&style_wenshidu_menu, 240);

    static lv_style_t style_font;
    lv_style_init(&style_font);
    lv_style_set_text_color(&style_font, lv_color_hex(0xFFFFFF));

    gxhtc3_get_tah();
    temp_value = round(temp);
    humi_value = round(humi);

    submenu = lv_obj_create(lv_scr_act());
    lv_obj_align(submenu, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(submenu, &style_wenshidu_menu, 0);

    // 创建温度显示标签
    temp_label = lv_label_create(submenu);
    lv_obj_add_style(temp_label, &style_font, 0);
    lv_label_set_text_fmt(temp_label, "Temperature: %d°C", temp_value);
    lv_obj_align(temp_label, LV_ALIGN_TOP_MID, 0, 20); // 例如，放在顶部中间

    // 创建湿度显示标签
    humi_label = lv_label_create(submenu);
    lv_obj_add_style(humi_label, &style_font, 0);
    lv_label_set_text_fmt(humi_label, "Humidity: %d%%", humi_value);
    lv_obj_align(humi_label, LV_ALIGN_TOP_MID, 0, 50); // 稍微往下一点放置

    icon_flag = 1; // 标记已经进入第一个应用
    xTaskCreate(get_th_task, "get_th_task", 4096, NULL, 5, NULL);

    my_lv_timer = lv_timer_create(thv_update_cb, 1000, NULL);

    lv_obj_add_event_cb(submenu, my_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(submenu, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(submenu, LV_OBJ_FLAG_CLICKABLE);
}

/******************************** 第2个图标 弹力球 应用程序******************************/
lv_obj_t *mat;       // 创建一个弹力球的垫子
lv_obj_t *ball;      // 创建一个弹力球
int mat_flag;        // 弹力球标志位
int ball_height = 0; // 弹力球的高度
int ball_dir = 0;    // 弹力球的方向
int mat_height = 0;  // 垫子的高度

// 弹力球各值更新程序
void game_update_cb(lv_timer_t *timer)
{
    if (strength != 0) // 发现有手指按下屏幕
    {
        if (strength < 31) // 限制手指按下时间最大为30
        {
            mat_height = 60 - strength;                             // 计算正方体的高度
            lv_obj_set_size(mat, 80, mat_height);                   // 调整正方体的高度
            lv_obj_align_to(ball, mat, LV_ALIGN_OUT_TOP_MID, 0, 0); // 让弹力球跟随垫子一起移动
            mat_flag = 1;                                           // 表示垫子已经缩小过
        }
    }
    else if (mat_flag == 1) // 如果垫子已经缩小过
    {
        lv_obj_set_size(mat, 80, 60);                           // 垫子回弹到原始值
        lv_obj_align_to(ball, mat, LV_ALIGN_OUT_TOP_MID, 0, 0); // 弹力球跟随垫子
        mat_flag = 2;                                           // 标记垫子已经回弹
    }
    else if (mat_flag == 2) // 垫子已经回弹 小球应该向上走了
    {
        if (ball_dir == 0) // 向上运动
        {
            if (ball_height < 150) // 限制弹力球上弹高度为150像素
            {
                ball_height = ball_height + 10;                                    // 每次上升10个像素
                lv_obj_align_to(ball, mat, LV_ALIGN_OUT_TOP_MID, 0, -ball_height); // 更新弹力球位置
                if (ball_height >= (0 + (60 - mat_height) * 5))                    // 根据力度计算小球最大高度
                {
                    ball_dir = 1; // 如果达到最大高度 更改小球运动方向
                }
            }
        }
        else // 向下运动
        {
            if (ball_height > 0) // 限制小球最低高度
            {
                ball_height = ball_height - 10;                                    // 每次降低10个像素高度
                lv_obj_align_to(ball, mat, LV_ALIGN_OUT_TOP_MID, 0, -ball_height); // 更新弹力球高度
                if (ball_height == 0)                                              // 如果弹力球落到了垫子上
                {
                    mat_flag = 0;   // 垫子状态恢复
                    ball_dir = 0;   // 小球方向恢复
                    mat_height = 0; // 垫子高度恢复
                }
            }
        }
    }
}
void game_menu(void)
{
    // 创建一个界面对象
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, 10);
    lv_style_set_bg_opa(&style, LV_OPA_COVER);
    lv_style_set_bg_color(&style, lv_color_hex(0xcccccc));
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_style_set_width(&style, 320);
    lv_style_set_height(&style, 240);

    submenu = lv_obj_create(lv_scr_act());
    lv_obj_add_style(submenu, &style, 0);

    // 创建一个垫子
    static lv_style_t mat_style;
    lv_style_init(&mat_style);
    lv_style_set_radius(&mat_style, 0);
    lv_style_set_border_width(&mat_style, 0);
    lv_style_set_pad_all(&mat_style, 0);
    lv_style_set_shadow_width(&mat_style, 10);
    lv_style_set_shadow_color(&mat_style, lv_color_black());
    lv_style_set_shadow_ofs_x(&mat_style, 10);
    lv_style_set_shadow_ofs_y(&mat_style, 10);

    mat = lv_obj_create(submenu);
    lv_obj_add_style(mat, &mat_style, 0);
    lv_obj_set_style_bg_color(mat, lv_color_hex(0x6B8E23), 0);
    lv_obj_align(mat, LV_ALIGN_BOTTOM_LEFT, 30, -30);
    lv_obj_set_size(mat, 80, 60);

    // 创建一个圆球
    ball = lv_led_create(submenu);
    lv_led_set_brightness(ball, 150);
    lv_led_set_color(ball, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_obj_align_to(ball, mat, LV_ALIGN_OUT_TOP_MID, 0, 0);

    // 创建一个lv_timer 用于更新圆球的坐标
    icon_flag = 2;                                           // 标记已经进入第2个图标
    my_lv_timer = lv_timer_create(game_update_cb, 50, NULL); //

    // 绘制退出提示符
    lv_obj_t *label = lv_label_create(submenu);
    lv_label_set_text(label, LV_SYMBOL_UP);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_RIGHT, -20, -20);

    // 添加向上滑动退出功能
    lv_obj_add_event_cb(submenu, my_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(submenu, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(submenu, LV_OBJ_FLAG_CLICKABLE);
}

void menu_event_handler(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    const char *label = lv_label_get_text(lv_obj_get_child(btn, 0));

    // 根据标签名或者按钮ID打开不同的子菜单
    if (strcmp(label, "wenshidu") == 0)
    {
        wenshidu_menu();
    }
    else if (strcmp(label, "game") == 0)
    {
        game_menu();
    }
    else if (strcmp(label, "Setting") == 0)
    {
        setting_menu();
    }
    else if (strcmp(label, "Netword") == 0)
    {
        network_menu();
    }
    else if (strcmp(label, "Application") == 0)
    {
        application_menu();
    }
    else if (strcmp(label, "Account") == 0)
    {
        account_menu();
    }
    else if (strcmp(label, "Time") == 0)
    {
        time_menu();
    }
    else if (strcmp(label, "other") == 0)
    {
        game_menu();
    }
    else if (strcmp(label, "Policy") == 0)
    {
        policy_menu();
    }
    else if (strcmp(label, "Update") == 0)
    {
        update_menu();
    }
}

void menu(void)
{
    lv_obj_del(start_src);

    static lv_style_t style_main_btn;
    lv_style_init(&style_main_btn);
    lv_style_set_border_color(&style_main_btn, lv_color_hex(0x003a57));

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a57), LV_PART_MAIN);

    static lv_style_t style_main_label;
    lv_style_init(&style_main_label);
    lv_style_set_text_color(&style_main_label, lv_color_hex(0xffffff));

    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "ESP32C3 Multi-Function Terminal");
    lv_obj_add_style(label, &style_main_label, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t *cont_row = lv_obj_create(lv_scr_act());
    lv_obj_set_style_bg_color(cont_row, lv_color_hex(0x003a57), LV_PART_MAIN);
    lv_obj_add_style(cont_row, &style_main_btn, LV_PART_MAIN);
    lv_obj_set_size(cont_row, 320, 100);
    lv_obj_align(cont_row, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_scrollbar_mode(cont_row, LV_SCROLLBAR_MODE_OFF);

    static lv_style_t style_main_font;
    lv_style_init(&style_main_font);
    lv_style_set_text_color(&style_main_font, lv_color_hex(0xffffff));
    const char *labels[] = {"wenshidu", "game", "Setting", "Netword", "Application", "Account", "Time", "other", "Policy", "Update"};

    uint32_t i;
    for (i = 0; i < sizeof(labels) / sizeof(labels[0]); i++)
    {
        lv_obj_t *obj;
        lv_obj_t *label;

        obj = lv_btn_create(cont_row);
        lv_obj_set_size(obj, 100, LV_PCT(100));
        label = lv_label_create(obj);
        lv_label_set_text_fmt(label, labels[i]);
        lv_obj_center(label);
        // 为按钮注册事件处理器
        lv_obj_add_event_cb(obj, menu_event_handler, LV_EVENT_CLICKED, NULL);
    }
}

/* multi-function menu  */
enum
{
    LV_MENU_ITEM_BUILDER_VARIANT_1,
    LV_MENU_ITEM_BUILDER_VARIANT_2
};
typedef uint8_t lv_menu_builder_variant_t;
static void back_event_handler(lv_event_t *e);
static void switch_handler(lv_event_t *e);
lv_obj_t *root_page;
static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt,
                             lv_menu_builder_variant_t builder_variant);
static lv_obj_t *create_slider(lv_obj_t *parent,
                               const char *icon, const char *txt, int32_t min, int32_t max, int32_t val);
static lv_obj_t *create_switch(lv_obj_t *parent,
                               const char *icon, const char *txt, bool chk);
void setting_menu(void)
{
    submenu = lv_menu_create(lv_scr_act());

    lv_color_t bg_color = lv_obj_get_style_bg_color(submenu, 0);
    if (lv_color_brightness(bg_color) > 127)
    {
        lv_obj_set_style_bg_color(submenu, lv_color_darken(lv_obj_get_style_bg_color(submenu, 0), 10), 0);
    }
    else
    {
        lv_obj_set_style_bg_color(submenu, lv_color_darken(lv_obj_get_style_bg_color(submenu, 0), 50), 0);
    }
    lv_menu_set_mode_root_back_btn(submenu, LV_MENU_ROOT_BACK_BTN_ENABLED);
    lv_obj_add_event_cb(submenu, back_event_handler, LV_EVENT_CLICKED, submenu);
    lv_obj_set_size(submenu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_center(submenu);

    lv_obj_t *cont;
    lv_obj_t *section;

    /*Create sub pages*/
    lv_obj_t *sub_mechanics_page = lv_menu_page_create(submenu, NULL);
    lv_obj_set_style_pad_hor(sub_mechanics_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(submenu), 0), 0);
    lv_menu_separator_create(sub_mechanics_page);
    section = lv_menu_section_create(sub_mechanics_page);
    create_slider(section, LV_SYMBOL_SETTINGS, "Velocity", 0, 150, 120);
    create_slider(section, LV_SYMBOL_SETTINGS, "Acceleration", 0, 150, 50);
    create_slider(section, LV_SYMBOL_SETTINGS, "Weight limit", 0, 150, 80);

    lv_obj_t *sub_sound_page = lv_menu_page_create(submenu, NULL);
    lv_obj_set_style_pad_hor(sub_sound_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(submenu), 0), 0);
    lv_menu_separator_create(sub_sound_page);
    section = lv_menu_section_create(sub_sound_page);
    create_switch(section, LV_SYMBOL_AUDIO, "Sound", false);

    lv_obj_t *sub_display_page = lv_menu_page_create(submenu, NULL);
    lv_obj_set_style_pad_hor(sub_display_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(submenu), 0), 0);
    lv_menu_separator_create(sub_display_page);
    section = lv_menu_section_create(sub_display_page);
    create_slider(section, LV_SYMBOL_SETTINGS, "Brightness", 0, 150, 100);

    lv_obj_t *sub_software_info_page = lv_menu_page_create(submenu, NULL);
    lv_obj_set_style_pad_hor(sub_software_info_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(submenu), 0), 0);
    section = lv_menu_section_create(sub_software_info_page);
    create_text(section, NULL, "Version 1.0", LV_MENU_ITEM_BUILDER_VARIANT_1);

    lv_obj_t *sub_legal_info_page = lv_menu_page_create(submenu, NULL);
    lv_obj_set_style_pad_hor(sub_legal_info_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(submenu), 0), 0);
    section = lv_menu_section_create(sub_legal_info_page);
    for (uint32_t i = 0; i < 15; i++)
    {
        create_text(section, NULL,
                    "This is a long long long long long long long long long text, if it is long enough it may scroll.",
                    LV_MENU_ITEM_BUILDER_VARIANT_1);
    }

    lv_obj_t *sub_about_page = lv_menu_page_create(submenu, NULL);
    lv_obj_set_style_pad_hor(sub_about_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(submenu), 0), 0);
    lv_menu_separator_create(sub_about_page);
    section = lv_menu_section_create(sub_about_page);
    cont = create_text(section, NULL, "Software information", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(submenu, cont, sub_software_info_page);
    cont = create_text(section, NULL, "Legal information", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(submenu, cont, sub_legal_info_page);
    cont = create_text(section, NULL, "Back", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(submenu, cont, root_page); // 添加返回到 root_page 的事件处理

    lv_obj_t *sub_menu_mode_page = lv_menu_page_create(submenu, NULL);
    lv_obj_set_style_pad_hor(sub_menu_mode_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(submenu), 0), 0);
    lv_menu_separator_create(sub_menu_mode_page);
    section = lv_menu_section_create(sub_menu_mode_page);
    cont = create_switch(section, LV_SYMBOL_AUDIO, "Sidebar enable", true);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), switch_handler, LV_EVENT_VALUE_CHANGED, submenu);

    /*Create a root page*/
    root_page = lv_menu_page_create(submenu, "Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(submenu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Mechanics", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(submenu, cont, sub_mechanics_page);
    cont = create_text(section, LV_SYMBOL_AUDIO, "Sound", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(submenu, cont, sub_sound_page);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Display", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(submenu, cont, sub_display_page);

    create_text(root_page, NULL, "Others", LV_MENU_ITEM_BUILDER_VARIANT_1);
    section = lv_menu_section_create(root_page);
    cont = create_text(section, NULL, "About", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(submenu, cont, sub_about_page);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "submenu mode", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(submenu, cont, sub_menu_mode_page);

    lv_menu_set_sidebar_page(submenu, root_page);

    lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(submenu), 0), 0), LV_EVENT_CLICKED, NULL);
}
static void back_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *menu = lv_event_get_user_data(e);

    if (lv_menu_back_btn_is_root(menu, obj))
    {
        lv_obj_del(submenu);
    }
    else
    {
        // lv_menu_go_back(menu);
    }
    //     lv_obj_t *submenu = lv_event_get_user_data(e);
    // // delete the submenu
    // lv_obj_del(submenu);
}

static void switch_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *menu = lv_event_get_user_data(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(obj, LV_STATE_CHECKED))
        {
            lv_menu_set_page(menu, NULL);
            lv_menu_set_sidebar_page(menu, root_page);
            lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
        }
        else
        {
            lv_menu_set_sidebar_page(menu, NULL);
            lv_menu_clear_history(menu); /* Clear history because we will be showing the root page later */
            lv_menu_set_page(menu, root_page);
        }
    }
}

static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt,
                             lv_menu_builder_variant_t builder_variant)
{
    lv_obj_t *obj = lv_menu_cont_create(parent);

    lv_obj_t *img = NULL;
    lv_obj_t *label = NULL;

    if (icon)
    {
        img = lv_img_create(obj);
        lv_img_set_src(img, icon);
    }

    if (txt)
    {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }

    if (builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt)
    {
        lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_swap(img, label);
    }

    return obj;
}

static lv_obj_t *create_slider(lv_obj_t *parent, const char *icon, const char *txt, int32_t min, int32_t max,
                               int32_t val)
{
    lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2);

    lv_obj_t *slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);

    if (icon == NULL)
    {
        lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    }

    return obj;
}

static lv_obj_t *create_switch(lv_obj_t *parent, const char *icon, const char *txt, bool chk)
{
    lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);

    lv_obj_t *sw = lv_switch_create(obj);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);

    return obj;
}

void network_menu(void)
{
}

void application_menu(void)
{
}

void account_menu(void)
{
}

void time_menu(void)
{
}

void other_menu(void)
{
}

void policy_menu(void)
{
}

void update_menu(void)
{
}