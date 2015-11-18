// ----------------------------------------
// P1X MICRO PLATFORM
// Krzysztof Jankowski <kj@p1x.in>
//
// (c)2015 P1X
// http://p1x.in
//
// Repo:
// https://github.com/w84death/p1x-micro-platform
//
// Components:
// Arduino Micro
// OLED 128x64
// BUZZER
// 2x BUTTONS
// ANALOG JOISTICK
//
// ----------------------------------------

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);


#define BUZZ 5
#define BUTTON_A 7
#define BUTTON_B 8
#define AXIS_X 9
#define AXIS_Y 10
#define AXIS_TRESHOLD 24
#define AXIS_X_CALIBRATION 18
#define AXIS_Y_CALIBRATION 0


#define STATE_READY 1
#define STATE_LOG 2
#define STATE_MENU 4
#define STATE_GAME 8
int GAME_STATE = 0;

#define SPRITE_WIDTH  16
#define SPRITE_HEIGHT 16

static const unsigned char PROGMEM logo_p1x[] =
{ B11111100, B10010001,
  B10000101, B10010001,
  B10000100, B10010001,
  B10000100, B10010001,
  B10000100, B10010001,
  B10000100, B10010001,
  B10000100, B10001010,
  B10000100, B10000100,
  B10000100, B10001010,
  B11111100, B10010001,
  B10000000, B10010001,
  B10000000, B10010001,
  B10000000, B10010001,
  B10000000, B10010001,
  B10000000, B10010001,
  B10000000, B10010001,
  B10000000, B10010001,
  B10000000, B10010001, };

char* game_strings[] = {
  "MICRO PLATFORM",
  "PRESS ANY BUTTON..",
  "LOG: ",
  "[A] ",
  "[B] ",
  "[X] ",
  "[Y] ",
};

void hello(void){
  display.clearDisplay();
  display.drawBitmap(55, 23,  logo_p1x, SPRITE_WIDTH, SPRITE_HEIGHT, 1);
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(26,50);
  display.print(game_strings[0]);
  display.display();
}

void game_message(int string_id, boolean clear_screen = 0){
  if (clear_screen){
    display.clearDisplay();
  }
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print(game_strings[1]);
  display.println(game_strings[string_id]);
  display.display();
}

void game_log(boolean a, boolean b, int x, int y){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(game_strings[2]);
  display.print(game_strings[3]);
  display.println(a);
  display.print(game_strings[2]);
  display.print(game_strings[4]);
  display.println(b);
  display.print(game_strings[2]);
  display.print(game_strings[5]);
  display.println(x - 512);
  display.print(game_strings[2]);
  display.print(game_strings[6]);
  display.println(y - 512);
  display.display();
}

void intro_melody(void){
  int delay_value = 80;
  for (long i = 0; i < 160; i++) {
    buzz_once(delay_value, 1);
    delay_value = delay_value + 22;
  }
}

void buzz_once(int delay_value, int cycles){
    for (int i = 0; i < cycles; i++){
    digitalWrite(BUZZ, HIGH);
    delayMicroseconds(delay_value);
    digitalWrite(BUZZ, LOW);
    delayMicroseconds(delay_value);
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(BUZZ, OUTPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(AXIS_X, INPUT);
  pinMode(AXIS_Y, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);

  display.display();
  hello();
  intro_melody();
  game_message(1, true);
  GAME_STATE = STATE_READY;
}

void loop() {
  boolean read_a = !digitalRead(BUTTON_A);
  boolean read_b = !digitalRead(BUTTON_B);
  int read_x = analogRead(AXIS_X) + AXIS_X_CALIBRATION;
  int read_y = analogRead(AXIS_Y) + AXIS_Y_CALIBRATION;

  if (GAME_STATE == STATE_READY){
    if (read_a or read_b){
      GAME_STATE = STATE_LOG;
    }
  }

  if (GAME_STATE == STATE_LOG){
    if (read_a) {
      buzz_once(180, 4);
    }
    if (read_b) {
      buzz_once(220, 4);
    }
    if (abs(read_x - 512) > AXIS_TRESHOLD){
      buzz_once(read_x, 2);
    }
    if (abs(read_y - 512) > AXIS_TRESHOLD){
      buzz_once(read_y, 2);
    }
    game_log(read_a, read_b, read_x, read_y);
  }
}
