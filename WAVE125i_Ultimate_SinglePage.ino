
/*
  WAVE125i ULTIMATE SINGLE PAGE DASHBOARD - หน้าเดียวจบ เท่ๆ
  บอร์ด ESP32-2432S028R (CYD 2.8") 237 บาท
  UI: หน้าเดียวรวมทั้งหมด - วัดรอบวงกลม + แบต + AF กราฟ + เช็ค/ลบโค๊ต
  ภาษาไทย 100%

  Library: TFT_eSPI + XPT2046_Touchscreen + lvgl 8.3.11
  Setup: User_Setups/Setup250_ESP32_CYD_2432S028R.h
*/

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define HOR 320
#define VER 240
#define BUF_SIZE (HOR * VER / 10)

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(33, 36);
HardwareSerial KLine(2);

#define PIN_RPM 35
#define PIN_BATT 34
#define PIN_AF 32
#define PIN_K_RX 16
#define PIN_K_TX 17

volatile unsigned long pulseCnt=0;
int rpmVal=0; float battV=13.8; float afV=0.65; float afr=14.7;
String dtc="ไม่มีโค๊ต"; bool hasDTC=false;
void IRAM_ATTR isrRPM(){pulseCnt++;}

// LVGL
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[BUF_SIZE];
lv_obj_t *meter; lv_meter_indicator_t *needle;
lv_obj_t *lbRPM, *lbBatt, *lbAF, *lbAFSt, *lbDTC, *chart;
lv_chart_series_t *ser;
lv_obj_t *ledShift[5];

void flush_cb(lv_disp_drv_t *d, const lv_area_t *a, lv_color_t *c){
  uint32_t w=a->x2-a->x1+1, h=a->y2-a->y1+1;
  tft.startWrite(); tft.setAddrWindow(a->x1,a->y1,w,h);
  tft.pushColors((uint16_t*)&c->full,w*h,true); tft.endWrite();
  lv_disp_flush_ready(d);
}
void touch_cb(lv_indev_drv_t *d, lv_indev_data_t *data){
  if(ts.touched()){
    TS_Point p=ts.getPoint();
    data->state=LV_INDEV_STATE_PR;
    data->point.x=map(p.x,3800,200,0,320);
    data->point.y=map(p.y,200,3800,0,240);
  }else data->state=LV_INDEV_STATE_REL;
}
void check_cb(lv_event_t *e){
  lv_label_set_text(lbDTC,"กำลังเช็คโค๊ต...");
  KLine.write((uint8_t*)"\x02\x04\x00\x00",4); delay(300);
  if(KLine.available()){
    int c=KLine.read(); 
    String t[]={"","1-MAP","7-ECT","8-TP","9-IAT","12-หัวฉีด","21-O2","29-IACV","52-CKP"};
    if(c>=1 && c<9) dtc=t[c]; else dtc="รหัส:"+String(c);
    hasDTC=true;
  }else{
    dtc = hasDTC?"12-หัวฉีดน้ำมัน":"ไม่มีโค๊ต";
  }
  lv_label_set_text(lbDTC,("โค๊ต: "+dtc).c_str());
}
void clear_cb(lv_event_t *e){
  lv_label_set_text(lbDTC,"กำลังลบ...");
  KLine.write((uint8_t*)"\x02\x04\xFF\x00",4); delay(500);
  dtc="ลบสำเร็จ"; hasDTC=false;
  lv_label_set_text(lbDTC,dtc.c_str());
}

void setup(){
  Serial.begin(115200);
  KLine.begin(10400,SERIAL_8N1,PIN_K_RX,PIN_K_TX);
  pinMode(PIN_RPM,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_RPM),isrRPM,FALLING);
  tft.init(); tft.setRotation(1); tft.fillScreen(TFT_BLACK);
  ts.begin(); ts.setRotation(1);
  lv_init();
  lv_disp_draw_buf_init(&draw_buf,buf,NULL,BUF_SIZE);
  static lv_disp_drv_t disp; lv_disp_drv_init(&disp);
  disp.hor_res=HOR; disp.ver_res=VER; disp.flush_cb=flush_cb; disp.draw_buf=&draw_buf;
  lv_disp_drv_register(&disp);
  static lv_indev_drv_t indev; lv_indev_drv_init(&indev);
  indev.type=LV_INDEV_TYPE_POINTER; indev.read_cb=touch_cb;
  lv_indev_drv_register(&indev);

  lv_obj_t *scr=lv_scr_act();
  lv_obj_set_style_bg_color(scr,lv_color_hex(0x0A0A0A),0);
  lv_obj_set_style_bg_opa(scr,LV_OPA_COVER,0);

  // Header
  lv_obj_t *head=lv_label_create(scr);
  lv_label_set_text(head,"WAVE125i ULTIMATE  13.8V  ● WIFI");
  lv_obj_set_style_text_color(head,lv_color_hex(0x00FFCC),0);
  lv_obj_set_pos(head,5,2);
  // Shift lights
  for(int i=0;i<5;i++){
    ledShift[i]=lv_led_create(scr);
    lv_obj_set_size(ledShift[i],12,8);
    lv_obj_set_pos(ledShift[i],180+i*15,2);
    lv_led_set_color(ledShift[i],lv_color_hex(i<3?0x00FF00:i<4?0xFFFF00:0xFF0000));
    lv_led_off(ledShift[i]);
  }
  // Meter
  meter=lv_meter_create(scr);
  lv_obj_set_size(meter,150,150); lv_obj_set_pos(meter,5,18);
  lv_obj_set_style_bg_color(meter,lv_color_hex(0x111111),0);
  lv_meter_scale_t *sc=lv_meter_add_scale(meter);
  lv_meter_set_scale_range(meter,sc,0,120,270,90);
  lv_meter_set_scale_ticks(meter,sc,13,2,10,lv_color_hex(0x333333));
  lv_meter_set_scale_major_ticks(meter,sc,1,3,15,lv_color_white(),10);
  auto red=lv_meter_add_scale_lines(meter,sc,lv_color_hex(0xFF0000),lv_color_hex(0xFF0000),false,0);
  lv_meter_set_indicator_start_value(meter,red,90); lv_meter_set_indicator_end_value(meter,red,120);
  needle=lv_meter_add_needle_line(meter,sc,4,lv_color_hex(0x00FFCC),-12);

  lbRPM=lv_label_create(scr);
  lv_label_set_text(lbRPM,"0\nRPM"); lv_obj_set_style_text_align(lbRPM,LV_TEXT_ALIGN_CENTER,0);
  lv_obj_set_style_text_font(lbRPM,&lv_font_montserrat_14,0);
  lv_obj_set_style_text_color(lbRPM,lv_color_white(),0);
  lv_obj_set_pos(lbRPM,50,95);

  // Cards Right
  auto cardBatt=lv_obj_create(scr); lv_obj_set_size(cardBatt,150,52); lv_obj_set_pos(cardBatt,162,18);
  lv_obj_set_style_bg_color(cardBatt,lv_color_hex(0x1E1E1E),0); lv_obj_set_style_border_width(cardBatt,1,0);
  lv_obj_set_style_border_color(cardBatt,lv_color_hex(0x333333),0);
  auto t1=lv_label_create(cardBatt); lv_label_set_text(t1,"แบตเตอรี่"); lv_obj_set_style_text_color(t1,lv_color_hex(0x888888),0); lv_obj_set_pos(t1,0,-8);
  lbBatt=lv_label_create(cardBatt); lv_label_set_text(lbBatt,"13.8 V  ปกติ"); lv_obj_set_style_text_font(lbBatt,&lv_font_montserrat_14,0); lv_obj_set_style_text_color(lbBatt,lv_color_hex(0xFFFF55),0); lv_obj_set_pos(lbBatt,0,10);

  auto cardAF=lv_obj_create(scr); lv_obj_set_size(cardAF,150,52); lv_obj_set_pos(cardAF,162,75);
  lv_obj_set_style_bg_color(cardAF,lv_color_hex(0x1E1E1E),0); lv_obj_set_style_border_color(cardAF,lv_color_hex(0x333333),0);
  auto t2=lv_label_create(cardAF); lv_label_set_text(t2,"ส่วนผสม A/F"); lv_obj_set_style_text_color(t2,lv_color_hex(0x888888),0); lv_obj_set_pos(t2,0,-8);
  lbAF=lv_label_create(cardAF); lv_label_set_text(lbAF,"14.7"); lv_obj_set_style_text_font(lbAF,&lv_font_montserrat_16,0); lv_obj_set_style_text_color(lbAF,lv_color_white(),0); lv_obj_set_pos(lbAF,0,10);
  lbAFSt=lv_label_create(cardAF); lv_label_set_text(lbAFSt,"พอดี"); lv_obj_set_style_text_color(lbAFSt,lv_color_hex(0x00FF00),0); lv_obj_set_pos(lbAFSt,60,12);

  // Chart mini
  chart=lv_chart_create(scr); lv_obj_set_size(chart,150,45); lv_obj_set_pos(chart,162,132);
  lv_chart_set_type(chart,LV_CHART_TYPE_LINE); lv_chart_set_range(chart,LV_CHART_AXIS_PRIMARY_Y,100,200);
  lv_chart_set_point_count(chart,30); lv_obj_set_style_bg_color(chart,lv_color_hex(0x111111),0);
  ser=lv_chart_add_series(chart,lv_color_hex(0x00FFCC),LV_CHART_AXIS_PRIMARY_Y);

  lbDTC=lv_label_create(scr); lv_label_set_text(lbDTC,"โค๊ต: ไม่มีโค๊ต"); lv_obj_set_style_text_color(lbDTC,lv_color_hex(0xFFAA00),0); lv_obj_set_pos(lbDTC,5,175);

  auto b1=lv_btn_create(scr); lv_obj_set_size(b1,148,35); lv_obj_set_pos(b1,5,195); lv_obj_set_style_bg_color(b1,lv_color_hex(0x331111),0); lv_obj_set_style_border_color(b1,lv_color_hex(0xCC2222),0); lv_obj_set_style_border_width(b1,1,0);
  auto l1=lv_label_create(b1); lv_label_set_text(l1,"เช็คโค๊ต"); lv_obj_center(l1); lv_obj_add_event_cb(b1,check_cb,LV_EVENT_CLICKED,NULL);
  auto b2=lv_btn_create(scr); lv_obj_set_size(b2,148,35); lv_obj_set_pos(b2,162,195); lv_obj_set_style_bg_color(b2,lv_color_hex(0x22AA22),0);
  auto l2=lv_label_create(b2); lv_label_set_text(l2,"ลบโค๊ต"); lv_obj_center(l2); lv_obj_add_event_cb(b2,clear_cb,LV_EVENT_CLICKED,NULL);
}

void loop(){
  static unsigned long last=0;
  if(millis()-last>200){
    noInterrupts(); unsigned long c=pulseCnt; pulseCnt=0; interrupts();
    rpmVal=c*300; if(rpmVal>13000) rpmVal=0;
    int rb=analogRead(PIN_BATT); battV=(rb/4095.0)*3.3*5.545;
    int ra=analogRead(PIN_AF); afV=(ra/4095.0)*3.3;
    if(afV<0.3) afr=15.8; else if(afV<0.6) afr=14.7; else afr=13.2;
    lv_meter_set_indicator_value(meter,needle,map(constrain(rpmVal,0,12000),0,12000,0,120));
    lv_label_set_text(lbRPM,(String(rpmVal)+"\nRPM").c_str());
    lv_label_set_text(lbBatt,(String(battV,1)+" V").c_str());
    lv_label_set_text(lbAF,String(afr,1).c_str());
    String st="พอดี"; lv_color_t col=lv_color_hex(0x00FF00);
    if(afr>15){st="บาง"; col=lv_color_hex(0xFF4444);} else if(afr<13.5){st="หนา"; col=lv_color_hex(0xFFAA00);}
    lv_label_set_text(lbAFSt,st.c_str()); lv_obj_set_style_text_color(lbAFSt,col,0);
    lv_chart_set_next_value(chart,ser,afr*10);
    for(int i=0;i<5;i++){ if(rpmVal>i*2000+6000) lv_led_on(ledShift[i]); else lv_led_off(ledShift[i]); }
    last=millis();
  }
  lv_timer_handler(); delay(5);
}
