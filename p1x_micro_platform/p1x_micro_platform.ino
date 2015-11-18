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
// 2x Buzzer (stereo)
// 2x Buttons
// Analog Joystick
// (optional) power bank / battery supply
//
// ----------------------------------------

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#define MAX_X 127
#define MAX_Y 63

#define BUZZ_LEFT 5
#define BUZZ_RIGHT 6
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

#define LOGO_SIZE 16
#define SPRITE_SIZE 8
#define MAP_BOUNDS 6

byte player_x = 64;
byte player_y = 32;

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
  B10000000, B10010001 };

static const unsigned char PROGMEM sprite_clear[] =
{ B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111};
  
static const unsigned char PROGMEM sprite_player[] =
{ B00000000,
  B00110000,
  B01011000,
  B01111100,
  B10111010,
  B10011001,
  B00100100,
  B01101100};

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
  display.drawBitmap(55, 23,  logo_p1x, LOGO_SIZE, LOGO_SIZE, 1);
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(26,50);
  display.print(game_strings[0]);
  display.display();
  intro_melody();
  delay(1500);
}

void game_message(int string_id, boolean clear_screen = false){
  if (clear_screen) display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
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

// ----------------------------------------------- AUDIO --
void buzz_stereo(int delay_value, int cycles, byte channel = 3){
  for (int i = 0; i < cycles; i++){
    if(channel == 1 or channel == 3) digitalWrite(BUZZ_LEFT, HIGH);
    if(channel == 2 or channel == 3) digitalWrite(BUZZ_RIGHT, HIGH);
    delayMicroseconds(delay_value);
    digitalWrite(BUZZ_LEFT, LOW);
    digitalWrite(BUZZ_RIGHT, LOW);
  }
}

void intro_melody(void){
  int delay_value = 80;
  for (long i = 0; i < 160; i++) {
    buzz_stereo(delay_value, 1, 1);
    if(i%2 == 0) buzz_stereo(delay_value, 1, 2);
    delay_value = delay_value + 22;
  }
}

// ----------------------------------------------- GAME LOGIC --

void game_change_state(byte state){
  display.clearDisplay();
  GAME_STATE = state;
  buzz_stereo(512, 4, 3);
  delay(300);
}

// ----------------------------------------------- PLAYER --

void game_move_player(byte x, byte y){
  player_x = player_x + x;
  player_y = player_y + y;
  
  if( player_x > MAX_X - SPRITE_SIZE - MAP_BOUNDS ) player_x = MAP_BOUNDS;
  if( player_x < MAP_BOUNDS ) player_x = MAX_X - SPRITE_SIZE - MAP_BOUNDS;
  
  if( player_y > MAX_Y - SPRITE_SIZE - MAP_BOUNDS ) player_y = MAP_BOUNDS;
  if( player_y < MAP_BOUNDS ) player_y = MAX_Y - SPRITE_SIZE - MAP_BOUNDS;
}

// ----------------------------------------------- DRAW --

void game_draw_player(){
  display.drawBitmap(player_x, player_y, sprite_player, SPRITE_SIZE, SPRITE_SIZE, 1);
}

void game_draw_map(){
  display.drawRect(0, 0, MAX_X, MAX_Y, 1);
  display.drawRect(2, 2, MAX_X-4, MAX_Y-4, 1);
}
// ----------------------------------------------- SETUP --
void setup() {
  Serial.begin(9600);
  pinMode(BUZZ_LEFT, OUTPUT);
  pinMode(BUZZ_RIGHT, OUTPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(AXIS_X, INPUT);
  pinMode(AXIS_Y, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.display();
  
  // DRAW LOGO
  hello();
  
  // PRESS ANY KEY
  game_change_state(STATE_READY);
  game_message(1, true);
}

// ----------------------------------------------- MAIN LOOP --
boolean read_a(){
  return !digitalRead(BUTTON_A);
}

boolean read_b(){
  return !digitalRead(BUTTON_B);
}

void loop() {
  short int read_x = analogRead(AXIS_X) + AXIS_X_CALIBRATION;
  short int read_y = analogRead(AXIS_Y) + AXIS_Y_CALIBRATION;
 
  // STATE - PRESS ANY KEY
  // -------------------------------------
  if (GAME_STATE == STATE_READY){
    if (read_a() or read_b()) game_change_state(STATE_GAME);
  }

  // STATE - GAME LOG
  // -------------------------------------
  if (GAME_STATE == STATE_LOG){
    if (read_a() and !read_b()) buzz_stereo(180, 4, 1);
    if (read_b() and !read_a()) buzz_stereo(220, 4, 2);
    if (abs(read_x - 512) > AXIS_TRESHOLD) buzz_stereo(read_x, 2, 1);
    if (abs(read_y - 512) > AXIS_TRESHOLD) buzz_stereo(read_y, 2, 2);   
    game_log(read_a(), read_b(), read_x, read_y);
    if (read_a() and read_b()) game_change_state(STATE_GAME);
  }
  
  // STATE - MENU
  // -------------------------------------
  if (GAME_STATE == STATE_MENU){

  }
  
  // STATE - GAME
  // -------------------------------------
  if (GAME_STATE == STATE_GAME){
    if (read_a() and read_b()) game_change_state(STATE_LOG);
    
    if(abs(read_x - 512) > AXIS_TRESHOLD){
      if(read_x < 512) game_move_player(-1,0);
      if(read_x > 512) game_move_player(1,0);
    }
    if(abs(read_y - 512) > AXIS_TRESHOLD){
      if(read_y < 512) game_move_player(0, -1);
      if(read_y > 512) game_move_player(0, 1);
    }
    
    display.clearDisplay();
    game_draw_map();
    game_draw_player();
    display.display();
    delay(33);
  }
}

