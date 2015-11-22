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
#define AXIS_TRESHOLD 256
#define AXIS_X_CALIBRATION 18
#define AXIS_Y_CALIBRATION 0

#define STATE_READY 1
#define STATE_LOG 2
#define STATE_MENU 4
#define STATE_GAME 8
#define STATE_END 16
int GAME_STATE = 0;

#define LOGO_SIZE 16
#define SPRITE_SIZE 8
#define MAP_BOUNDS 6
#define MAP_WIDTH 16
#define MAP_HEIGHT 8

#define LAYER_TERRAIN 0
#define LAYER_WALL 1
#define LAYER_ITEMS 2
#define LAYER_MONSTERS 4
#define LAYER_TEMP 8

#define ATTACK_RANGE 2
byte player_x = 8;
byte player_y = 4;
boolean player_item = false;
boolean player_alive = true;

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
  
static const unsigned char PROGMEM sprite_player[] =
{ B00000000,
  B00110000,
  B01011000,
  B01111100,
  B10111010,
  B10011001,
  B00100100,
  B01101100};

static const unsigned char PROGMEM sprite_terrain1[] =
{ B00000000,
  B01000000,
  B00000100,
  B00000000,
  B00000000,
  B00100000,
  B00000010,
  B00000000};


static const unsigned char PROGMEM sprite_terrain2[] =
{ B00001000,
  B01000000,
  B00010010,
  B00000000,
  B00000100,
  B01000000,
  B00000001,
  B00010000};
  
 static const unsigned char PROGMEM sprite_wall[] =
 {B01111110,
  B11110101,
  B11010101,
  B10101011,
  B11010111,
  B10101111,
  B11111111,
  B01111110};
  
static const unsigned char PROGMEM sprite_item[] =
{ B01111110,
  B10000001,
  B10110101,
  B10001001,
  B10010101,
  B10100101,
  B10000001,
  B01111110};
  
static const unsigned char PROGMEM sprite_monster[] =
{ B00000000,
  B00001100,
  B00111110,
  B01010110,
  B00111110,
  B00101001,
  B00100101,
  B00010101};
  
char* game_strings[] = {
  "MICRO PLATFORM",
  "KILL ALL MONSTERS",
  "GAME ENGINE LOG: ",
  "[A] ",
  "[B] ",
  "[X] ",
  "[Y] ",
  "[PLAYER POS] ",
  "YOU WIN!",
  "YOU DIED!" };


byte game_map_terrain[] =
{ B11111111, B11111111,
  B11001000, B01110011,
  B10000000, B01000001,
  B11000000, B00000001,
  B10000000, B00000001,
  B10000000, B00000011,
  B10110000, B00100011,
  B11111111, B11111111 };

byte game_map_items[] =
{ B00000000, B00000000,
  B00000000, B00001000,
  B00000000, B00000000,
  B00000100, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B10000000,
  B00000000, B00000000 };
  
byte game_map_monsters[] =
{ B00000000, B00000000,
  B00100000, B00000000,
  B00000000, B00001000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00100000, B00010000,
  B00000000, B00000000,
  B00000000, B00000000 };

byte game_map_temp[] =
{ B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000 };
  
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
}

void game_message(int string_id, boolean clear_screen = false){
  if (clear_screen) display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(game_strings[string_id]);
  display.display();
}

void game_log(boolean a, boolean b, int x, int y){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(game_strings[2]);
  
  display.print(game_strings[3]);
  display.print(a);
  display.print("   ");
  display.print(game_strings[4]);
  display.println(b);
  
  display.print(game_strings[5]);
  display.println(x - 512);
  
  display.print(game_strings[6]);
  display.println(y - 512);
  
  display.print(game_strings[7]);
  display.print(player_x);
  display.print(" x ");
  display.print(player_y);
  
  display.display();
}







// ----------------------------------------------- AUDIO --
void buzz_stereo(int freq, byte channel = 3, boolean music = false){
    if(channel == 1 or channel == 3) tone(BUZZ_LEFT, freq);
    if(channel == 2 or channel == 3) tone(BUZZ_RIGHT, freq);
    if(music) delay(freq * 1.30);
    noTone(BUZZ_LEFT);
    noTone(BUZZ_RIGHT);
}

void intro_melody(void){
  int freq = 80;
  for (byte i = 0; i < 4; i++) {
    buzz_stereo(freq, 1, true);
    if(i%2 == 0) buzz_stereo(freq, 2, true);
    freq = freq + 22;
  }
}

// ----------------------------------------------- MAP --

byte game_map_read(byte x, byte y, byte type){
  byte shift = 0;
  byte block;
  byte block_bit;
  if(x >= 8) shift = 1;
  block = (y*2)+shift;
  block_bit = 7-(x-(8*shift));
 
  if( type == LAYER_TERRAIN) return bitRead(game_map_terrain[block], block_bit);
  if( type == LAYER_ITEMS) return bitRead(game_map_items[block], block_bit);
  if( type == LAYER_MONSTERS) return bitRead(game_map_monsters[block], block_bit);
};

void game_map_write(byte x, byte y, byte type, byte set_bit = 0){
  byte shift = 0;
  byte block;
  byte block_bit;
  if(x >= 8) shift = 1;
  block = (y*2)+shift;
  block_bit = 7-(x-(8*shift));
  
  
  if( type == LAYER_TERRAIN) bitWrite(game_map_terrain[block], block_bit, set_bit);
  if( type == LAYER_ITEMS) bitWrite(game_map_items[block], block_bit, set_bit);
  if( type == LAYER_MONSTERS) bitWrite(game_map_monsters[block], block_bit, set_bit);
}




// ----------------------------------------------- PLAYER --

void game_player_move(byte x, byte y){
  byte new_x = player_x + x;
  byte new_y = player_y + y;
  
  if (!game_map_read(new_x, new_y, LAYER_TERRAIN)){
    
    // TAKE ITEM
    if (!player_item and game_map_read(new_x, new_y, LAYER_ITEMS)){
      game_player_take_item(new_x, new_y);
    }else{
      buzz_stereo(44);
      player_x = new_x;
      player_y = new_y;
    }
    
  }else{
    buzz_stereo(22, 3, true);
  }  
}

void game_player_take_item(byte x, byte y){
  if(!player_item){
    buzz_stereo(80, 3, true);
    player_item = true;
    game_map_write(x, y, LAYER_ITEMS); // clear item
  }
};

void game_player_attack(){
  if (player_item){
    buzz_stereo(12, 2, true);
    player_item = false;
    for( byte x = player_x - ATTACK_RANGE; x <= player_x + ATTACK_RANGE; x++){
    for( byte y = player_y - ATTACK_RANGE; y <= player_y + ATTACK_RANGE; y++){
      if(!( x == player_x and y == player_y) and game_map_read(x, y, LAYER_MONSTERS)){
        game_map_write(x, y, LAYER_MONSTERS); // kill monster
      }
    }}
  }
};






// ----------------------------------------------- DRAW --

void game_draw_player(){
  display.drawBitmap(player_x*SPRITE_SIZE, player_y*SPRITE_SIZE, sprite_player, SPRITE_SIZE, SPRITE_SIZE, 1);
}

void game_draw_map(){
  byte draw_tile = 0;

  for (byte x = 0; x < MAP_WIDTH; x++) {
  for (byte y = 0; y < MAP_HEIGHT; y++) {
      
      draw_tile = 0;
      
      if (!(player_x == x and player_y == y)){
        // CHECK TILE TYPE
        // if (!game_map_read(x, y, LAYER_TERRAIN)) draw_tile += LAYER_TERRAIN; // it's = 0 :)
        if (game_map_read(x, y, LAYER_TERRAIN)) draw_tile += LAYER_WALL;
        if (game_map_read(x, y, LAYER_ITEMS)) draw_tile += LAYER_ITEMS;
        if (game_map_read(x, y, LAYER_MONSTERS)) draw_tile += LAYER_MONSTERS;
        
        // DRAW SPRITES
        
        // DRAW GRASS
        if (draw_tile == 0){
          if( (x+y) % 2 == 0) display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_terrain1, SPRITE_SIZE, SPRITE_SIZE, 1);
          else display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_terrain2, SPRITE_SIZE, SPRITE_SIZE, 1);
        }
        
        // FOREST
        if (draw_tile == 1) display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_wall, SPRITE_SIZE, SPRITE_SIZE, 1);
        
        // DRAW ITEM
        if (draw_tile == 2) display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_item, SPRITE_SIZE, SPRITE_SIZE, 1);
        if (draw_tile == 4 or draw_tile == 6) display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_monster, SPRITE_SIZE, SPRITE_SIZE, 1);
      }
  }}
}

void game_draw_hud(){
  if (player_item){
    display.drawCircle((player_x*SPRITE_SIZE) + (SPRITE_SIZE/2), (player_y*SPRITE_SIZE) + (SPRITE_SIZE/2), SPRITE_SIZE * ATTACK_RANGE, 1);
  }
};


// ----------------------------------------------- GAME LOGIC --

void game_change_state(byte state){
  display.clearDisplay();
  GAME_STATE = state;
  buzz_stereo(128, 3, true);
  delay(300);
}


void game_ai_run(){
  byte new_x;
  byte new_y;
  byte monsters = 0;
  
  for (byte x = 0; x < MAP_WIDTH; x++) {
  for (byte y = 0; y < MAP_HEIGHT; y++) {
    if (game_map_read(x, y, LAYER_MONSTERS)){
      if(!(new_x == x and new_y == y)){
        monsters++;
        new_x = x + random(-1,2);
        new_y = y + random(-1,2);
        if (!game_map_read(new_x, new_y, LAYER_TERRAIN) and !game_map_read(new_x, new_y, LAYER_MONSTERS)){
          if(player_x == new_x and player_y == new_y){
            player_alive = false;
            game_change_state(STATE_END);
            return;
          }else{
            game_map_write(x, y, LAYER_MONSTERS);
            game_map_write(new_x, new_y, LAYER_MONSTERS, true);
          }
        }
      }
    }
  }}
  
  if (monsters == 0){
    game_change_state(STATE_END);
  }
};







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
    if (read_a() or read_b()) game_player_attack();
    
    if(abs(read_x - 512) > AXIS_TRESHOLD){
      if(read_x < 512) game_player_move(-1,0);
      if(read_x > 512) game_player_move(1,0);
      game_ai_run();
    }
    if(abs(read_y - 512) > AXIS_TRESHOLD){
      if(read_y < 512) game_player_move(0, -1);
      if(read_y > 512) game_player_move(0, 1);
      game_ai_run();
    }
    
    display.clearDisplay();
    game_draw_map();
    game_draw_player();
    game_draw_hud();
    display.display();
    delay(33);
  }
  
    // STATE - MENU
  // -------------------------------------
  if (GAME_STATE == STATE_END){
    if(player_alive){
      game_message(8, true);
    }else{
      game_message(9, true);
    }
  }
}

