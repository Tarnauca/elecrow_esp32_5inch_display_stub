#include <lvgl.h>
#include "lv_demo_widgets.h"

#include "ui.h"

#include <SPI.h>

#define TFT_BL 2
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
class LGFX : public lgfx::LGFX_Device
{
public:

  lgfx::Bus_RGB     _bus_instance;
  lgfx::Panel_RGB   _panel_instance;

  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;
      
      cfg.pin_d0  = GPIO_NUM_8; // B0
      cfg.pin_d1  = GPIO_NUM_3;  // B1
      cfg.pin_d2  = GPIO_NUM_46;  // B2
      cfg.pin_d3  = GPIO_NUM_9;  // B3
      cfg.pin_d4  = GPIO_NUM_1;  // B4
      
      cfg.pin_d5  = GPIO_NUM_5;  // G0
      cfg.pin_d6  = GPIO_NUM_6; // G1
      cfg.pin_d7  = GPIO_NUM_7;  // G2
      cfg.pin_d8  = GPIO_NUM_15;  // G3
      cfg.pin_d9  = GPIO_NUM_16; // G4
      cfg.pin_d10 = GPIO_NUM_4;  // G5
      
      cfg.pin_d11 = GPIO_NUM_45; // R0
      cfg.pin_d12 = GPIO_NUM_48; // R1
      cfg.pin_d13 = GPIO_NUM_47; // R2
      cfg.pin_d14 = GPIO_NUM_21; // R3
      cfg.pin_d15 = GPIO_NUM_14; // R4

      cfg.pin_henable = GPIO_NUM_40;
      cfg.pin_vsync   = GPIO_NUM_41;
      cfg.pin_hsync   = GPIO_NUM_39;
      cfg.pin_pclk    = GPIO_NUM_0;
      cfg.freq_write  = 15000000;

      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 8;
      cfg.hsync_pulse_width = 4;
      cfg.hsync_back_porch  = 43;
      
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 8;
      cfg.vsync_pulse_width = 4;
      cfg.vsync_back_porch  = 12;

      cfg.pclk_active_neg   = 1;
      cfg.de_idle_high      = 0;
      cfg.pclk_idle_high    = 0;

      _bus_instance.config(cfg);
    }
            {
      auto cfg = _panel_instance.config();
      cfg.memory_width  = 800;
      cfg.memory_height = 480;
      cfg.panel_width  = 800;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      _panel_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);
    setPanel(&_panel_instance);

  }
};

LGFX lcd;

#define BUTTON_PIN    38

SPIClass& spi = SPI;
uint16_t touchCalibration_x0 = 300, touchCalibration_x1 = 3600, touchCalibration_y0 = 300, touchCalibration_y1 = 3600;
uint8_t  touchCalibration_rotate = 1, touchCalibration_invert_x = 2, touchCalibration_invert_y = 0;

/*******************************************************************************
   Please config the touch panel in touch.h
 ******************************************************************************/
#include "touch.h"
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t disp_draw_buf[800 * 480 / 10];
//static lv_color_t disp_draw_buf;
static lv_disp_drv_t disp_drv;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  

  //lcd.fillScreen(TFT_WHITE);
#if (LV_COLOR_16_SWAP != 0)
 lcd.pushImageDMA(area->x1, area->y1, w, h,(lgfx::rgb565_t*)&color_p->full);
#else
  lcd.pushImageDMA(area->x1, area->y1, w, h,(lgfx::rgb565_t*)&color_p->full);//
#endif

  lv_disp_flush_ready(disp);

}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (touch_has_signal())
  {
    if (touch_touched())
    {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
      Serial.print( "Data x :" );
      Serial.println( touch_last_x );

      Serial.print( "Data y :" );
      Serial.println( touch_last_y );
    }
    else if (touch_released())
    {
      data->state = LV_INDEV_STATE_REL;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
  delay(15);
}

uint16_t calData[5] = { 190, 3679, 382, 3335, 0 };

void setup()
{
  //  pinMode(TFT_BL, OUTPUT);
  //  digitalWrite(TFT_BL, LOW);
  Serial.begin(15200);
  Serial.println("LVGL Widgets Demo");

 
  // Init Display
  lcd.begin();
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextSize(2);
  delay(200);

  //  lcd.fillScreen(TFT_REDRED);
  //  delay(500);
  //  lcd.fillScreen(TFT_REDGREEN);
  //  delay(500);
  //  lcd.fillScreen(TFT_REDBLUE);
  //  delay(500);
  //  lcd.fillScreen(TFT_BLACK);
  //  delay(500);

  lv_init();

  delay(100);
  touch_init();

  screenWidth = lcd.width();
  screenHeight = lcd.height();

  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight / 10);
  //  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, 480 * 272 / 10);
  /* Initialize the display */
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the (dummy) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif


  //touch_calibrate();

  ui_init();//开机UI界面

  lcd.fillScreen(TFT_BLACK);
  lv_demo_widgets();//主UI界面
  Serial.println( "Setup done" );
}

void loop()
{
    lv_timer_handler();
    delay(5);
}


#define Z_THRESHOLD 350 // Touch pressure threshold for validating touches
#define _RAWERR 20 // Deadband error allowed in successive position samples
void begin_touch_read_write(void) {
  digitalWrite(38, HIGH); // Just in case it has been left low
  spi.setFrequency(600000);
  digitalWrite(38, LOW);
}

void end_touch_read_write(void) {
  digitalWrite(38, HIGH); // Just in case it has been left low
  spi.setFrequency(600000);

}

uint16_t getTouchRawZ(void) {

  begin_touch_read_write();

  // Z sample request
  int16_t tz = 0xFFF;
  spi.transfer(0xb0);               // Start new Z1 conversion
  tz += spi.transfer16(0xc0) >> 3;  // Read Z1 and start Z2 conversion
  tz -= spi.transfer16(0x00) >> 3;  // Read Z2

  end_touch_read_write();

  return (uint16_t)tz;
}

uint8_t getTouchRaw(uint16_t *x, uint16_t *y) {
  uint16_t tmp;

  begin_touch_read_write();

  // Start YP sample request for x position, read 4 times and keep last sample
  spi.transfer(0xd0);                    // Start new YP conversion
  spi.transfer(0);                       // Read first 8 bits
  spi.transfer(0xd0);                    // Read last 8 bits and start new YP conversion
  spi.transfer(0);                       // Read first 8 bits
  spi.transfer(0xd0);                    // Read last 8 bits and start new YP conversion
  spi.transfer(0);                       // Read first 8 bits
  spi.transfer(0xd0);                    // Read last 8 bits and start new YP conversion

  tmp = spi.transfer(0);                   // Read first 8 bits
  tmp = tmp << 5;
  tmp |= 0x1f & (spi.transfer(0x90) >> 3); // Read last 8 bits and start new XP conversion

  *x = tmp;

  // Start XP sample request for y position, read 4 times and keep last sample
  spi.transfer(0);                       // Read first 8 bits
  spi.transfer(0x90);                    // Read last 8 bits and start new XP conversion
  spi.transfer(0);                       // Read first 8 bits
  spi.transfer(0x90);                    // Read last 8 bits and start new XP conversion
  spi.transfer(0);                       // Read first 8 bits
  spi.transfer(0x90);                    // Read last 8 bits and start new XP conversion

  tmp = spi.transfer(0);                 // Read first 8 bits
  tmp = tmp << 5;
  tmp |= 0x1f & (spi.transfer(0) >> 3);  // Read last 8 bits

  *y = tmp;

  end_touch_read_write();

  return true;
}

uint8_t validTouch(uint16_t *x, uint16_t *y, uint16_t threshold) {
  uint16_t x_tmp, y_tmp, x_tmp2, y_tmp2;

  // Wait until pressure stops increasing to debounce pressure
  uint16_t z1 = 1;
  uint16_t z2 = 0;
  while (z1 > z2)
  {
    z2 = z1;
    z1 = getTouchRawZ();
    delay(1);
    Serial.print("z1:");
    Serial.println(z1);
  }


  if (z1 <= threshold) return false;

  getTouchRaw(&x_tmp, &y_tmp);


  delay(1); // Small delay to the next sample
  if (getTouchRawZ() <= threshold) return false;

  delay(2); // Small delay to the next sample
  getTouchRaw(&x_tmp2, &y_tmp2);


  if (abs(x_tmp - x_tmp2) > _RAWERR) return false;
  if (abs(y_tmp - y_tmp2) > _RAWERR) return false;

  *x = x_tmp;
  *y = y_tmp;

  return true;
}

void calibrateTouch(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size) {
  int16_t values[] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint16_t x_tmp, y_tmp;
  uint16_t _width = 800;
  uint16_t _height = 480;

  for (uint8_t i = 0; i < 4; i++) {
    lcd.fillRect(0, 0, size + 1, size + 1, color_bg);
    lcd.fillRect(0, _height - size - 1, size + 1, size + 1, color_bg);
    lcd.fillRect(_width - size - 1, 0, size + 1, size + 1, color_bg);
    lcd.fillRect(_width - size - 1, _height - size - 1, size + 1, size + 1, color_bg);

    if (i == 5) break; // used to clear the arrows

    switch (i) {
      case 0: // up left
        lcd.drawLine(0, 0, 0, size, color_fg);
        lcd.drawLine(0, 0, size, 0, color_fg);
        lcd.drawLine(0, 0, size , size, color_fg);
        break;
      case 1: // bot left
        lcd.drawLine(0, _height - size - 1, 0, _height - 1, color_fg);
        lcd.drawLine(0, _height - 1, size, _height - 1, color_fg);
        lcd.drawLine(size, _height - size - 1, 0, _height - 1 , color_fg);
        break;
      case 2: // up right
        lcd.drawLine(_width - size - 1, 0, _width - 1, 0, color_fg);
        lcd.drawLine(_width - size - 1, size, _width - 1, 0, color_fg);
        lcd.drawLine(_width - 1, size, _width - 1, 0, color_fg);
        break;
      case 3: // bot right
        lcd.drawLine(_width - size - 1, _height - size - 1, _width - 1, _height - 1, color_fg);
        lcd.drawLine(_width - 1, _height - 1 - size, _width - 1, _height - 1, color_fg);
        lcd.drawLine(_width - 1 - size, _height - 1, _width - 1, _height - 1, color_fg);
        break;
    }

    // user has to get the chance to release
    if (i > 0) delay(1000);

    for (uint8_t j = 0; j < 8; j++) {
      while (touch_has_signal())
      {
        if (touch_touched())
        {
          Serial.print( "Data x :" );
          Serial.println( touch_last_x );
          Serial.print( "Data y :" );
          Serial.println( touch_last_y );
          break;
        }
      }
    }
  }
}

void touch_calibrate()//屏幕校准
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  Serial.println("屏幕校准");

  //校准
  //  lcd.fillScreen(TFT_BLACK);
  //  lcd.setCursor(20, 0);
  //  Serial.println("setCursor");
  //  lcd.setTextFont(2);
  //  Serial.println("setTextFont");
  //  lcd.setTextSize(1);
  //  Serial.println("setTextSize");
  //  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  //  lcd.println("按指示触摸角落");
  Serial.println("按指示触摸角落");
  //  lcd.setTextFont(1);
  //  lcd.println();
  //      lcd.setCursor(175, 100);
  //      lcd.printf("Touch Adjust");
  //  Serial.println("setTextFont(1)");
  lv_timer_handler();
  calibrateTouch(calData, TFT_RED, TFT_BLACK, 17);
  Serial.println("calibrateTouch(calData, TFT_RED, TFT_BLACK, 15)");
  Serial.println(); Serial.println();
  Serial.println("//在setup()中使用此校准代码:");
  Serial.print("uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  //  Serial.print("  tft.setTouch(calData);");
  //  Serial.println(); Serial.println();
  //  lcd.fillScreen(TFT_BLACK);
  //  lcd.println("XZ OK!");
  //  lcd.println("Calibration code sent to Serial port.");


}

void setTouch(uint16_t *parameters) {
  touchCalibration_x0 = parameters[0];
  touchCalibration_x1 = parameters[1];
  touchCalibration_y0 = parameters[2];
  touchCalibration_y1 = parameters[3];

  if (touchCalibration_x0 == 0) touchCalibration_x0 = 1;
  if (touchCalibration_x1 == 0) touchCalibration_x1 = 1;
  if (touchCalibration_y0 == 0) touchCalibration_y0 = 1;
  if (touchCalibration_y1 == 0) touchCalibration_y1 = 1;

  touchCalibration_rotate = parameters[4] & 0x01;
  touchCalibration_invert_x = parameters[4] & 0x02;
  touchCalibration_invert_y = parameters[4] & 0x04;
}
