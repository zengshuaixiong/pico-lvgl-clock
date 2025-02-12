/*****************************************************************************
* | File        :   LCD_Test.h
* | Author      :   Waveshare team
* | Function    :   test Demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2024-06-27
* | Info        :   
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/

#include "LCD_test.h"
  
int press_time = 0;

#include <math.h>
#include <time.h>
#define WIN_WIDTH  LV_HOR_RES
#define WIN_HEIGHT LV_VER_RES

// 定义表盘半径和中心点
#define CLOCK_RADIUS 110
#define CLOCK_CENTER_X 100
#define CLOCK_CENTER_Y 130

// 定义指针长度
#define HOUR_HAND_LENGTH   70
#define MINUTE_HAND_LENGTH 80
#define SECOND_HAND_LENGTH 90

// 定义指针颜色
#define HOUR_HAND_COLOR   lv_color_hex(0x0000FF)   // 蓝色
#define MINUTE_HAND_COLOR lv_color_hex(0x00FF00)   // 绿色
#define SECOND_HAND_COLOR lv_color_hex(0xFF0000)   // 红色

//数字显示文本的长度和字体和颜色
#define LABEL_COLOR   lv_color_hex(0x000000)
#define LABEL_FONT    lv_font_montserrat_18
#define LABEL_LENGTH  CLOCK_RADIUS-12

// 定义变量存储选中的数字
static int clock_hour = 0;
static int clock_minute = 0;
static int clock_second = 0;

// 全局变量：指针对象
static lv_obj_t* hour_hand;
static lv_obj_t* minute_hand;
static lv_obj_t* second_hand;

// 定义滚动表盘显示的数字
static lv_obj_t* roller_hour;
static lv_obj_t* roller_minute;
static lv_obj_t* roller_second;

static int buffer_set_time = false;

// 定义直线的起点和终点坐标
    static lv_point_t line_points_hour[] = {
        {105, 125},  // 起点 (x1, y1)
        {200, 100} // 终点 (x2, y2)
    };

        // 定义直线的起点和终点坐标
    static lv_point_t line_points_min[] = {
        {105, 125},  // 起点 (x1, y1)
        {150, 50} // 终点 (x2, y2)
    };

        // 定义直线的起点和终点坐标
    static lv_point_t line_points_sec[] = {
        {105, 125},  // 起点 (x1, y1)
        {40, 100} // 终点 (x2, y2)
    };

static datetime_t time_all;

//滑动动画回调函数
static void set_x_anim(void* obj, int32_t value) {
    lv_obj_set_x((lv_obj_t*)obj, value);
}

// 手势事件处理函数
static void event_handler(lv_event_t* e) {
    lv_obj_t* scr = lv_event_get_target(e); // 获取事件目标（屏幕）
    lv_indev_t* indev = lv_indev_get_act(); // 获取当前输入设备
    lv_point_t point;
    lv_indev_get_vect(indev, &point); // 获取滑动向量

    if (e->code == LV_EVENT_GESTURE) {
        lv_obj_t* win1 = lv_obj_get_child(scr, 0); // 获取第一个窗口
        lv_obj_t* win2 = lv_obj_get_child(scr, 1); // 获取第二个窗口

        if (point.x < -50) { // 向左滑动
            // 动画：win1 向左移出，win2 从右侧进入
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, win1);
            lv_anim_set_values(&a, lv_obj_get_x(win1), -WIN_WIDTH);
            lv_anim_set_exec_cb(&a, set_x_anim);
            lv_anim_set_time(&a, 300);
            lv_anim_start(&a);

            lv_anim_set_var(&a, win2);
            lv_anim_set_values(&a, lv_obj_get_x(win2), 0);
            lv_anim_start(&a);
            buffer_set_time = true;
        } else if (point.x > 50) { // 向右滑动
            // 动画：win1 从左侧进入，win2 向右移出
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, win1);
            lv_anim_set_values(&a, lv_obj_get_x(win1), 0);
            lv_anim_set_exec_cb(&a, set_x_anim);
            lv_anim_set_time(&a, 300);
            lv_anim_start(&a);

            lv_anim_set_var(&a, win2);
            lv_anim_set_values(&a, lv_obj_get_x(win2), WIN_WIDTH);
            lv_anim_start(&a);
            buffer_set_time = false;
        }
    }
}

// 计算指针端点坐标
static void calculate_hand_position(double angle, int length, lv_point_t* point) {
    double radians = angle * M_PI / 180.0; // 角度转弧度
    point->x = CLOCK_CENTER_X + (int)(length * sin(radians));
    point->y = CLOCK_CENTER_Y - (int)(length * cos(radians));
}

// 更新指针位置
static void update_clock_hands(int hour, int min, int sec) {
    // 计算角度
    double hour_angle = (hour % 12) * 30.0 + min * 0.5;   // 每小时 30 度，每分钟 0.5 度
    double min_angle = min * 6.0;                         // 每分钟 6 度
    double sec_angle = sec * 6.0;                         // 每秒 6 度

    // 计算指针端点坐标
    lv_point_t hour_point, min_point, sec_point;
    calculate_hand_position(hour_angle, HOUR_HAND_LENGTH, &hour_point);
    calculate_hand_position(min_angle, MINUTE_HAND_LENGTH, &min_point);
    calculate_hand_position(sec_angle, SECOND_HAND_LENGTH, &sec_point);

    line_points_hour[1] = hour_point;
    line_points_min[1] = min_point;
    line_points_sec[1] = sec_point;

    lv_line_set_points(hour_hand, line_points_hour, 2);
    lv_line_set_points(minute_hand, line_points_min, 2);
    lv_line_set_points(second_hand, line_points_sec, 2);

}


// 定时器回调函数
static void timer_callback(lv_timer_t* timer) {
    // 获取当前时间（模拟 RTC 读取）
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
	PCF85063A_Read_now(&time_all);
	update_clock_hands(time_all.hour,time_all.min,time_all.sec);
	
	if(buffer_set_time == false)
    {
      lv_roller_set_selected(roller_hour,time_all.hour,LV_ANIM_OFF);
      lv_roller_set_selected(roller_minute,time_all.min,LV_ANIM_OFF);
      
    }
	//update_clock_hands(time_all->hour,time_all->min,time_all->sec);
    // 更新指针位置
    //update_clock_hands(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    //update_clock_hands(6, 50, 30);
}

// 滚盘事件回调函数
static void roller_event_handler(lv_event_t* e) {
    lv_obj_t* roller = lv_event_get_target(e); // 获取触发事件的滚盘对象
    uint16_t selected = lv_roller_get_selected(roller); // 获取选中的索引
	buffer_set_time = true;
    // 根据滚盘的 ID 更新对应的变量
    if (lv_obj_get_index(roller) == 0) {
        clock_hour = selected;
    } else if (lv_obj_get_index(roller) == 1) {
        clock_minute = selected;
    } else if (lv_obj_get_index(roller) == 2) {
        clock_second = selected;
    } else if (lv_obj_get_index(roller) == 3) {

    }
    datetime_t time_temp222;
    time_temp222.hour = clock_hour;
    time_temp222.min = clock_minute;
    time_temp222.sec = clock_second;
    PCF85063A_Set_Time(time_temp222);
    update_clock_hands(clock_hour,clock_minute,clock_second);

}



/**
 * Create a button with a label and react on click event.
 */
void lv_example_get_started_1(void)
{
    //lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    //lv_obj_set_pos(btn, 100, 10);                            /*Set its position*/
    //lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
   
    //lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    //lv_label_set_text(label, "temp");                     /*Set the labels text*/
    //lv_obj_center(label); 
    
	// 获取默认屏幕
    lv_obj_t* scr = lv_scr_act();

    lv_obj_t* win1 = lv_obj_create(scr);
    lv_obj_set_size(win1, WIN_WIDTH, WIN_HEIGHT);
    lv_obj_set_style_bg_color(win1, lv_color_hex(0xFFFFFF), 0); // 设置背景颜色
    lv_obj_set_x(win1, 0); // 初始位置在屏幕左侧

    lv_obj_t* win2 = lv_obj_create(scr);
    lv_obj_set_size(win2, WIN_WIDTH, WIN_HEIGHT);
    lv_obj_set_style_bg_color(win2, lv_color_hex(0xFFFFFF), 0); // 设置背景颜色
    lv_obj_set_x(win2, WIN_WIDTH); // 初始位置在屏幕右侧

     // 创建小时滚盘
    roller_hour = lv_roller_create(win2);
    lv_roller_set_options(roller_hour, "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23", LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(roller_hour, 50, 150);
    lv_obj_align(roller_hour, LV_ALIGN_CENTER, -70, -0);
    lv_obj_add_event_cb(roller_hour, roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // 创建分钟滚盘
    roller_minute = lv_roller_create(win2);
    lv_roller_set_options(roller_minute, "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59", LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(roller_minute, 50, 150);
    lv_obj_align(roller_minute, LV_ALIGN_CENTER, 0, -0);
    lv_obj_add_event_cb(roller_minute, roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // 创建秒滚盘
    roller_second = lv_roller_create(win2);
    lv_roller_set_options(roller_second, "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59", LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(roller_second, 50, 150);
    lv_obj_align(roller_second, LV_ALIGN_CENTER, 70, -0);
    lv_obj_add_event_cb(roller_second, roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

     // 创建圆形钟表背景
    lv_obj_t * clock = lv_obj_create(win1);

    // 设置圆形的宽度和高度相等，确保它是一个圆形
    lv_obj_set_size(clock, CLOCK_RADIUS*2, CLOCK_RADIUS*2);  // 250x250 像素的钟表背景

    // 设置钟表的位置
    lv_obj_align(clock, LV_ALIGN_CENTER, 0, 0);  // 将钟表置于屏幕中心

    // 设置钟表背景样式
    static lv_style_t style;
    lv_style_init(&style);

    // 设置背景颜色为灰色
    lv_style_set_bg_color(&style, lv_color_hex(0x808080));

    // 设置圆形的圆角为半径大小，使其成为圆形
    lv_style_set_radius(&style, LV_RADIUS_CIRCLE);

    // 应用样式到钟表对象
    lv_obj_add_style(clock, &style, 0);

    // 创建一条直线
    hour_hand = lv_line_create(win1);
    // 设置直线的点坐标
    lv_line_set_points(hour_hand, line_points_hour, 2);
    // 设置直线的样式
    lv_obj_set_style_line_width(hour_hand, 2, 0);              // 线宽为 2
    lv_obj_set_style_line_color(hour_hand, lv_color_hex(0xFF0000), 0); // 红色
    lv_obj_set_style_line_opa(hour_hand, LV_OPA_COVER, 0);     // 完全不透明

    // 创建一条直线
    minute_hand = lv_line_create(win1);
    // 设置直线的点坐标
    lv_line_set_points(minute_hand, line_points_min, 2);
    // 设置直线的样式
    lv_obj_set_style_line_width(minute_hand, 3, 0);              // 线宽为 2
    lv_obj_set_style_line_color(minute_hand, lv_color_hex(0x00FF00), 0); // 红色
    lv_obj_set_style_line_opa(minute_hand, LV_OPA_COVER, 0);     // 完全不透明

    // 创建一条直线
    second_hand = lv_line_create(win1);
    // 设置直线的点坐标
    lv_line_set_points(second_hand, line_points_sec, 2);
    // 设置直线的样式
    lv_obj_set_style_line_width(second_hand, 4, 0);              // 线宽为 2
    lv_obj_set_style_line_color(second_hand, lv_color_hex(0x00FFFF), 0); // 红色
    lv_obj_set_style_line_opa(second_hand, LV_OPA_COVER, 0);     // 完全不透明

    // 创建标签并设置文本
    lv_obj_t* label12 = lv_label_create(win1);
    lv_label_set_text(label12, "12"); // 设置文本内容
    lv_obj_set_style_text_color(label12, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label12, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label12, LV_ALIGN_CENTER, 0, -(LABEL_LENGTH)); // 位置显示

    lv_obj_t* label3 = lv_label_create(win1);
    lv_label_set_text(label3, "3"); // 设置文本内容
    lv_obj_set_style_text_color(label3, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label3, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label3, LV_ALIGN_CENTER, LABEL_LENGTH, 0); // 位置显示

    lv_obj_t* label6 = lv_label_create(win1);
    lv_label_set_text(label6, "6"); // 设置文本内容
    lv_obj_set_style_text_color(label6, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label6, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label6, LV_ALIGN_CENTER, 0, LABEL_LENGTH); // 位置显示

    lv_obj_t* label9 = lv_label_create(win1);
    lv_label_set_text(label9, "9"); // 设置文本内容
    lv_obj_set_style_text_color(label9, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label9, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label9, LV_ALIGN_CENTER, -(LABEL_LENGTH), 0); // 位置显示

    lv_obj_t* label1 = lv_label_create(win1);
    lv_label_set_text(label1, "1"); // 设置文本内容
    lv_obj_set_style_text_color(label1, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label1, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label1, LV_ALIGN_CENTER, (LABEL_LENGTH)*sin(30.0 * M_PI / 180.0), -(LABEL_LENGTH)*cos(30.0 * M_PI / 180.0)); // 位置显示

    lv_obj_t* label2 = lv_label_create(win1);
    lv_label_set_text(label2, "2"); // 设置文本内容
    lv_obj_set_style_text_color(label2, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label2, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label2, LV_ALIGN_CENTER, (LABEL_LENGTH)*sin(60.0 * M_PI / 180.0), -(LABEL_LENGTH)*cos(60.0 * M_PI / 180.0)); // 位置显示

    lv_obj_t* label4 = lv_label_create(win1);
    lv_label_set_text(label4, "4"); // 设置文本内容
    lv_obj_set_style_text_color(label4, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label4, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label4, LV_ALIGN_CENTER, (LABEL_LENGTH)*sin(120.0 * M_PI / 180.0), -(LABEL_LENGTH)*cos(120.0 * M_PI / 180.0)); // 位置显示

    lv_obj_t* label5 = lv_label_create(win1);
    lv_label_set_text(label5, "5"); // 设置文本内容
    lv_obj_set_style_text_color(label5, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label5, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label5, LV_ALIGN_CENTER, (LABEL_LENGTH)*sin(150.0 * M_PI / 180.0), -(LABEL_LENGTH)*cos(150.0 * M_PI / 180.0)); // 位置显示

    lv_obj_t* label7 = lv_label_create(win1);
    lv_label_set_text(label7, "7"); // 设置文本内容
    lv_obj_set_style_text_color(label7, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label7, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label7, LV_ALIGN_CENTER, (LABEL_LENGTH)*sin(210.0 * M_PI / 180.0), -(LABEL_LENGTH)*cos(210.0 * M_PI / 180.0)); // 位置显示

    lv_obj_t* label8 = lv_label_create(win1);
    lv_label_set_text(label8, "8"); // 设置文本内容
    lv_obj_set_style_text_color(label8, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label8, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label8, LV_ALIGN_CENTER, (LABEL_LENGTH)*sin(240.0 * M_PI / 180.0), -(LABEL_LENGTH)*cos(240.0 * M_PI / 180.0)); // 位置显示

    lv_obj_t* label10 = lv_label_create(win1);
    lv_label_set_text(label10, "10"); // 设置文本内容
    lv_obj_set_style_text_color(label10, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label10, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label10, LV_ALIGN_CENTER, (LABEL_LENGTH)*sin(300.0 * M_PI / 180.0), -(LABEL_LENGTH)*cos(300.0 * M_PI / 180.0)); // 位置显示

    lv_obj_t* label11 = lv_label_create(win1);
    lv_label_set_text(label11, "11"); // 设置文本内容
    lv_obj_set_style_text_color(label11, LABEL_COLOR, 0); // 文本颜色
    lv_obj_set_style_text_font(label11, &LABEL_FONT, 0); // 字体大小
    lv_obj_align(label11, LV_ALIGN_CENTER, (LABEL_LENGTH)*sin(330.0 * M_PI / 180.0), -(LABEL_LENGTH)*cos(330.0 * M_PI / 180.0)); // 位置显示


    // 添加手势事件处理
    lv_obj_add_event_cb(scr, event_handler, LV_EVENT_GESTURE, NULL);



    update_clock_hands(11,30,45);
    // 创建定时器，每 100ms 更新一次
    lv_timer_create(timer_callback, 100, NULL);

   
}

int LCD_1in69_LVGL_Test(void)
{
    if (DEV_Module_Init() != 0)
    {
        return -1;
    } 

    printf("LCD_1in69_LCGL_test Demo\r\n");
    /*Init LCD*/
    LCD_1IN69_Init(VERTICAL);
    LCD_1IN69_Clear(WHITE);
    DEV_SET_PWM(60);
    /*Init RTC*/
    PCF85063A_Init();
    
    datetime_t time_temp;
    time_temp.hour = 13;
    time_temp.min = 23;
    time_temp.sec = 00;
    //PCF85063A_Set_Time(time_temp);
    
    /*Init touch screen*/ 
    CST816S_init(CST816S_ALL_Mode);
    /*Init IMU*/
    QMI8658_init();
    /*Init LVGL*/
    LVGL_Init();
    lv_example_get_started_1();
    //Widgets_Init();

    while(1)
    {
      lv_task_handler();
      DEV_Delay_ms(5); 
      if(DEV_Digital_Read(PWR_KEY_PIN) == 0)
      {
        press_time++;
        if(press_time > 300)//shutdown  
        {
          press_time = 0;
          DEV_Digital_Write(BAT_PWR_PIN, 0);
        }
      }
      else
      {
        press_time = 0;
      }
    }
    

    DEV_Module_Exit();
    return 0;
}
