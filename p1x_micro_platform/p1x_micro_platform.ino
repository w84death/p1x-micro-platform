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
#define STATE_INVENTORY 2
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


byte player_x = 1;
byte player_y = 1;
boolean player_item = false;
boolean player_alive = true;
boolean player_direction_left = true;
byte player_attack_range = 2;
byte player_ammo = 1;

byte aliens_alive = 0;
unsigned int game_tick = 0;

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
  
static const unsigned char PROGMEM sprite_player_left[] =
{ B00000000,
  B00110000,
  B01011000,
  B01111100,
  B10111010,
  B10011000,
  B00100100,
  B01101100};
  
static const unsigned char PROGMEM sprite_player_right[] =
{ B00000000,
  B00001100,
  B00011010,
  B00111110,
  B01011101,
  B00011001,
  B00100100,
  B00110110};

static const unsigned char PROGMEM sprite_terrain1[] =
{ B00000000,
  B00000000,
  B00000000,
  B00001000,
  B00000000,
  B00000000,
  B00000000,
  B00000000};


static const unsigned char PROGMEM sprite_terrain2[] =
{ B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00010000,
  B00000000,
  B00000000,
  B00000000};
  
 static const unsigned char PROGMEM sprite_wall[] =
 {B01100110,
  B11000011,
  B10110101,
  B00000100,
  B00100000,
  B10101101,
  B11000011,
  B01100110};
  
static const unsigned char PROGMEM sprite_item[] =
{ B10000001,
  B00111100,
  B01000010,
  B01011010,
  B01010010,
  B01000010,
  B00111100,
  B10000001};
  
static const unsigned char PROGMEM sprite_monster_anim0[] =
{ B00000000,
  B00001100,
  B00111110,
  B01010110,
  B00111110,
  B00101001,
  B00100101,
  B00010101};
  
static const unsigned char PROGMEM sprite_monster_anim1[] =
{ B00000000,
  B00001100,
  B00111110,
  B01101010,
  B00111110,
  B00101001,
  B01000101,
  B00101010};

byte game_map_terrain[] =
{ B11110001, B11100011,
  B10100000, B00100011,
  B00100100, B10110001,
  B01111111, B10111001,
  B00111111, B00011011,
  B01100100, B00010011,
  B00000111, B11000011,
  B11000100, B00011111 };

byte game_map_items[] =
{ B00000000, B00000000,
  B00010000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00010000, B00000100,
  B00000000, B00000000 };
  
byte game_map_aliens[] =
{ B00000000, B00000000,
  B00000000, B00000100,
  B00000000, B01000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B01000000,
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
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(26,50);
  display.print("MICRO PLATFORM");
  display.setCursor(12,68);
  display.print("PRESS A TO START");
  display.display();
  intro_melody();
}

void game_draw_intro(){
  display.clearDisplay();
  display.setCursor(0,38);
  display.println("NEVADA, USA");  
  display.println("MILITARY BASE");
  display.display();
};

void game_draw_end_screen(boolean win){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(16, 32);
  if(win)  display.println("YOU WIN!");
  if(!win)  display.println("GAME OVER");
  display.display();
}


void game_draw_inventory(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  
  display.println("--INVENTORY---------");
  
  display.print("BLASTER: ");
  if (player_item) display.println("IN HAND");
  if (!player_item) display.println("-");
  
  display.print("ATTACK RANGE: ");
  display.print(player_attack_range);
  display.println();
  
  display.print("AMMO: ");
  for(byte a = 0; a<player_ammo; a++){
    display.print("[+]");
  }
  display.println();

  display.setCursor(0, 48);
  display.println("--ALINENS DETECTED--");
  display.print("> ");
  display.print(aliens_alive);
  
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
  if( type == LAYER_MONSTERS) return bitRead(game_map_aliens[block], block_bit);
  if( type == LAYER_TEMP) return bitRead(game_map_temp[block], block_bit);
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
  if( type == LAYER_MONSTERS) bitWrite(game_map_aliens[block], block_bit, set_bit);
  if( type == LAYER_TEMP) bitWrite(game_map_temp[block], block_bit, set_bit);
}




// ----------------------------------------------- PLAYER --

void game_player_move(byte x, byte y){
  byte new_x = player_x + x;
  byte new_y = player_y + y;
  
  if (new_x >= 0 and new_y >= 0 and new_x < MAP_WIDTH and new_y < MAP_HEIGHT and !game_map_read(new_x, new_y, LAYER_TERRAIN)){
    
    // TAKE ITEM
    if (game_map_read(new_x, new_y, LAYER_ITEMS)){
      game_player_take_item(new_x, new_y);
    }else{
      buzz_stereo(44);
      if (new_x < player_x) player_direction_left = true;
      if (new_x > player_x) player_direction_left = false;
      player_x = new_x;
      player_y = new_y;
    }
    
  }else{
    buzz_stereo(22, 3, true);
  }  
}

void game_player_take_item(byte x, byte y){
  if(!player_item){
    buzz_stereo(200, 3, true);
    player_attack_range = 1;
    player_ammo++;
  }else{
     buzz_stereo(100, 3, true);
     buzz_stereo(100, 3, true);
     player_attack_range = 3;
     player_ammo += 2;
  }
  player_item = true;
  game_map_write(x, y, LAYER_ITEMS); // clear item
};

void game_player_attack(){
  if (player_item and player_ammo > 0){
    buzz_stereo(40, 3, true);
    buzz_stereo(30, 3, true);
    buzz_stereo(60, 3, true);
    player_ammo--;
    if(player_ammo<1) player_item = false;
    
    for( int x = player_x - player_attack_range; x <= player_x + player_attack_range; x++){
    for( int y = player_y - player_attack_range; y <= player_y + player_attack_range; y++){
      if(x >= 0 and y >= 0 and x < MAP_WIDTH and y < MAP_HEIGHT and !( x == player_x and y == player_y)){
        game_map_write(x, y, LAYER_TERRAIN); // destroy terrain
        game_map_write(x, y, LAYER_MONSTERS); // kill monster
        game_map_write(x, y, LAYER_TEMP); // and in temp
      }
    }}
  }
  
  game_ai_run();
};






// ----------------------------------------------- DRAW --

void game_draw_player(){
  if (player_direction_left){
    display.drawBitmap(player_x*SPRITE_SIZE, player_y*SPRITE_SIZE, sprite_player_left, SPRITE_SIZE, SPRITE_SIZE, player_alive);
  }else{
    display.drawBitmap(player_x*SPRITE_SIZE, player_y*SPRITE_SIZE, sprite_player_right, SPRITE_SIZE, SPRITE_SIZE, player_alive);
  }
}

void game_draw_map(){
  byte draw_tile = 0;

  for (byte x = 0; x < MAP_WIDTH; x++) {
  for (byte y = 0; y < MAP_HEIGHT; y++) {
      
      draw_tile = 0;
      
      
        // CHECK TILE TYPE
        // if (!game_map_read(x, y, LAYER_TERRAIN)) draw_tile += LAYER_TERRAIN; // it's = 0 :)
        if (game_map_read(x, y, LAYER_TERRAIN)) draw_tile += LAYER_WALL;
        if (game_map_read(x, y, LAYER_ITEMS)) draw_tile += LAYER_ITEMS;
        if (game_map_read(x, y, LAYER_MONSTERS)) draw_tile += LAYER_MONSTERS;
        
        // DRAW SPRITES
        
        if(!(player_x == x and player_y == y)){
          // DRAW GRASS
          if (draw_tile == 0){
            if( (x+y) % 2 == 0) display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_terrain1, SPRITE_SIZE, SPRITE_SIZE, 1);
            else display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_terrain2, SPRITE_SIZE, SPRITE_SIZE, 1);
          }
          
          // FOREST
          if (draw_tile == 1) display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_wall, SPRITE_SIZE, SPRITE_SIZE, 1);
          
          // DRAW ITEM
          if (draw_tile == 2) display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_item, SPRITE_SIZE, SPRITE_SIZE, 1);
        } 
        // DRAW MONSTER
        if (draw_tile == 4 or draw_tile == 6){
          if (game_tick%2 == 0){
            display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_monster_anim0, SPRITE_SIZE, SPRITE_SIZE, 1);
          }else{
            display.drawBitmap(x*SPRITE_SIZE, y*SPRITE_SIZE, sprite_monster_anim1, SPRITE_SIZE, SPRITE_SIZE, 1);
          }
        }
      
  }}
}

void game_draw_hud(){
  if (player_item){
    display.drawCircle((player_x*SPRITE_SIZE) + (SPRITE_SIZE/2), (player_y*SPRITE_SIZE) + (SPRITE_SIZE/2), SPRITE_SIZE +  (player_attack_range * SPRITE_SIZE), 1);
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
  byte x;
  byte y;
  byte new_x;
  byte new_y;
  byte monsters = 0;
  byte temp = 0;
  
  for (x = 0; x < MAP_WIDTH; x++) {
  for (y = 0; y < MAP_HEIGHT; y++) {
    if (game_map_read(x, y, LAYER_MONSTERS)){
      monsters++;
      new_x = x + random(-1, 2);
      new_y = y + random(-1, 2);
      if (new_x >= 0 and new_y >= 0 and new_x < MAP_WIDTH and new_y < MAP_HEIGHT and !game_map_read(new_x, new_y, LAYER_TERRAIN) and !game_map_read(new_x, new_y, LAYER_MONSTERS) and !game_map_read(new_x, new_y, LAYER_TEMP)){
        game_map_write(new_x, new_y, LAYER_TEMP, true);
        game_map_write(x, y, LAYER_TEMP);
        game_map_write(x, y, LAYER_MONSTERS);
      }else{
        game_map_write(x, y, LAYER_TEMP, true);
      }
      
    }
  }}
  
  if (monsters == 0){
    game_change_state(STATE_END);
  }else{
    // SWAP LAYERS
    temp = 0;
    for (x = 0; x < MAP_WIDTH; x++) {
    for (y = 0; y < MAP_HEIGHT; y++) {
      temp = game_map_read(x, y, LAYER_TEMP);

      if(temp and (player_x == x and player_y == y)){
        player_alive = false;
        game_change_state(STATE_END);
      }
      
      game_map_write(x, y, LAYER_MONSTERS, temp);
      aliens_alive = monsters;
    }}
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
    game_draw_intro();
    if (read_a() or read_b()){
      game_change_state(STATE_GAME);
      game_ai_run();
    }
  }

  // STATE - INVENTORY
  // -------------------------------------
  if (GAME_STATE == STATE_INVENTORY){  
    game_draw_inventory();
    if (read_b()) game_change_state(STATE_GAME);
  }
  
  // STATE - MENU
  // -------------------------------------
  if (GAME_STATE == STATE_MENU){

  }
  
  // STATE - GAME
  // -------------------------------------
  if (GAME_STATE == STATE_GAME or GAME_STATE == STATE_END){
    
    
    if (read_b()) game_change_state(STATE_INVENTORY);
    
    if(player_alive){
      game_tick++;
      if (read_a()) game_player_attack();
      
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
    }else{
      game_tick = 2;
    }
    
    display.clearDisplay();
    game_draw_map();
    if(player_alive) game_draw_player();
    game_draw_hud();
    display.display();
    delay(33);
  }
  
  // STATE - MENU
  // -------------------------------------
  if (GAME_STATE == STATE_END){
    delay(1200);
    if(player_alive){
      game_draw_end_screen(true);
    }else{
      game_draw_end_screen(false);
    }
    delay(500);
  }
}

