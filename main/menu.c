#include "menu.h"
#include "math.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "example";
extern lv_obj_t *start_src;
lv_obj_t *submenu;
lv_timer_t *my_lv_timer;
int icon_flag;

/********************************返回按钮回调函数***************************/
void back_to_main_menu(lv_event_t *e)
{
    // lv_obj_t *submenu = lv_event_get_user_data(e);
    // // delete the submenu
    // lv_obj_del(submenu);

    if (icon_flag == 1)
    {
        lv_timer_del(my_lv_timer);
    }
    lv_obj_del(submenu);
}

/*********************************温湿度显示******************************/

lv_obj_t *temp_meter;
lv_obj_t *humi_meter;
lv_obj_t *temp_label;
lv_obj_t *humi_label;
lv_meter_indicator_t *temp_indic;
lv_meter_indicator_t *humi_indic;
int temp_value, humi_value; // 室内实时温湿度值

void get_th_task(void *args)
{
    esp_err_t ret;
    int time_cnt = 0, date_cnt = 0;
    float temp_sum = 0.0, humi_sum = 0.0;

    while (1)
    {
        ret = gxhtc3_get_tah(); // 获取一次温湿度
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "GXHTC3 READ TAH ERROR.");
        }
        else
        {                               // 如果成功获取数据
            temp_sum = temp_sum + temp; // 温度累计和
            humi_sum = humi_sum + humi; // 湿度累计和
            date_cnt++;                 // 记录累计次数
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // 延时100毫秒
        time_cnt++;                           // 每100毫秒+1
        if (time_cnt > 10)                    // 1秒钟到
        {
            // 取平均数 且把结果四舍五入为整数
            temp_value = round(temp_sum / date_cnt);
            humi_value = round(humi_sum / date_cnt);
            // 各标志位清零
            time_cnt = 0;
            date_cnt = 0;
            temp_sum = 0;
            humi_sum = 0;
            // 标记温湿度有新数值

            // th_update_flag = 1;
        }
        if (icon_flag == 0)
        {
            break;
        }
    }
    vTaskDelete(NULL);
}

// 定时更新温湿度值
void thv_update_cb(lv_timer_t *timer)
{
    lv_meter_set_indicator_end_value(temp_meter, temp_indic, temp_value);
    lv_meter_set_indicator_end_value(humi_meter, humi_indic, humi_value);
    lv_label_set_text_fmt(temp_label, "%d℃", temp_value);
    lv_label_set_text_fmt(humi_label, "%d%%", humi_value);
}
void setting_menu(void)
{
    // 重新创建一个面板对象
    static lv_style_t style_setting_menu;
    lv_style_init(&style_setting_menu);
    lv_style_set_bg_color(&style_setting_menu, lv_color_hex(0x003a57));

    static lv_style_t style_font;
    lv_style_init(&style_font);
    lv_style_set_text_color(&style_font, lv_color_hex(0xFFFFFF));

    submenu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(submenu, 320, 240);
    lv_obj_align(submenu, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(submenu, &style_setting_menu, 0);

    gxhtc3_get_tah();
    temp_value = round(temp);
    humi_value = round(humi);
    // 创建温度显示标签
    lv_obj_t *temp_number = lv_label_create(submenu);
    lv_obj_add_style(temp_number, &style_font, 0);
    lv_label_set_text_fmt(temp_number, "Temperature: %d°C", temp_value);
    lv_obj_align(temp_number, LV_ALIGN_TOP_MID, 0, 20); // 例如，放在顶部中间

    // 创建湿度显示标签
    lv_obj_t *humi_number = lv_label_create(submenu);
    lv_obj_add_style(humi_number, &style_font, 0);
    lv_label_set_text_fmt(humi_number, "Humidity: %d%%", humi_value);
    lv_obj_align(humi_number, LV_ALIGN_TOP_MID, 0, 50); // 稍微往下一点放置

    icon_flag = 1; // 标记已经进入第一个应用
    xTaskCreate(get_th_task, "get_th_task", 4096, NULL, 5, NULL);

    my_lv_timer = lv_timer_create(thv_update_cb, 1000, NULL);

    // 创建返回按钮
    lv_obj_t *back_btn = lv_btn_create(submenu);
    lv_obj_set_size(back_btn, 100, 50);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_RIGHT, 0, -20);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);

    // 为返回按钮注册事件处理器
    lv_obj_add_event_cb(back_btn, back_to_main_menu, LV_EVENT_CLICKED, submenu);
}

static lv_obj_t *label;
static void slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);

    /*Refresh the text*/
    lv_label_set_text_fmt(label, "%" LV_PRId32, lv_slider_get_value(slider));
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
}
void system_menu(void)
{
    static lv_style_t style_submenu;
    lv_style_init(&style_submenu);
    lv_style_set_bg_color(&style_submenu, lv_color_hex(0x003a57));

    static lv_style_t style_sublabel;
    lv_style_init(&style_sublabel);
    lv_style_set_text_color(&style_sublabel, lv_color_hex(0xFFFFFF));

    submenu = lv_obj_create(lv_scr_act());
    lv_obj_add_style(submenu, &style_submenu, LV_PART_MAIN);
    lv_obj_set_size(submenu, 320, 240);
    lv_obj_align(submenu, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *label = lv_label_create(submenu);
    lv_label_set_text(label, "system");
    lv_obj_add_style(label, &style_sublabel, LV_PART_MAIN);

    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

    /*Create a slider in the center of the display*/
    lv_obj_t *slider = lv_slider_create(submenu);
    lv_obj_set_width(slider, 200);                                              /*Set the width*/
    lv_obj_center(slider);                                                      /*Align to the center of the parent (screen)*/
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL); /*Assign an event function*/

    /*Create a label above the slider*/
    label = lv_label_create(submenu);
    lv_label_set_text(label, "0");
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/

    // 创建返回按钮
    lv_obj_t *back_btn = lv_btn_create(submenu);
    lv_obj_set_size(back_btn, 100, 50);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_RIGHT, 0, -20);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);

    // 为返回按钮注册事件处理器
    lv_obj_add_event_cb(back_btn, back_to_main_menu, LV_EVENT_CLICKED, submenu);
}

void menu_event_handler(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    const char *label = lv_label_get_text(lv_obj_get_child(btn, 0));

    // 根据标签名或者按钮ID打开不同的子菜单
    if (strcmp(label, "Setting") == 0)
    {
        setting_menu();
    }
    else if (strcmp(label, "System") == 0)
    {
        system_menu();
    }
    else if (strcmp(label, "Bluetooth") == 0)
    {
        bluetooth_menu();
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
    else if (strcmp(label, "Game") == 0)
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
    const char *labels[] = {"Setting", "System", "Bluetooth", "Netword", "Application", "Account", "Time", "Game", "Policy", "Update"};

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

void bluetooth_menu(void)
{
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

void game_menu(void)
{
}

void policy_menu(void)
{
}

void update_menu(void)
{
}