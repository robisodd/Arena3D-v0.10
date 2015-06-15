#pragma once
#include "pebble.h"

  
#define IF_COLOR(x)     COLOR_FALLBACK(x, (void)0)
#define IF_BW(x)        COLOR_FALLBACK((void)0, x)
#define IF_BWCOLOR(x,y) COLOR_FALLBACK(x, y)
  
#define mapsize 21             // Map is mapsize x mapsize squares big
#define MAX_TEXTURES 10        // Most number of textures there's likely to be.  Feel free to increase liberally, but no more than 254.
#define IDCLIP false           // Walk thru walls
#define view_border true       // Draw border around 3D viewing window

typedef struct SquareTypeStruct {
  uint8_t face[4]; // texture[] number
  uint8_t ceiling; // texture[] number (255 = no ceiling / sky.  Well, 255 or any number >MAX_TEXTURES)
  uint8_t floor;   // texture[] number (255 = no floor texture)
  // other characteristics like walk thru and stuff
} SquareTypeStruct;

typedef struct PlayerStruct {
  int32_t x;                  // Player's X Position (64 pixels per square)
  int32_t y;                  // Player's Y Position (64 pixels per square)
  int16_t facing;             // Player Direction Facing (from 0 - TRIG_MAX_ANGLE)
} PlayerStruct;

typedef struct ObjectStruct {
  int32_t x;                  // Player's X Position (64 pixels per square)
  int32_t y;                  // Player's Y Position (64 pixels per square)
  int16_t health;             //
  uint8_t type;               // Enemy, Lamp, Coin, etc
  uint8_t sprite;             // sprite_image[] and sprite_mask[] for object
  int32_t data1;              // 
  int32_t data2;              // 
} ObjectStruct;

typedef struct RayStruct {
   int32_t x;                 // x coordinate the ray hit
   int32_t y;                 // y coordinate the ray hit
  uint32_t dist;              // length of the ray, i.e. distance ray traveled
   uint8_t hit;               // square_type the ray hit [0-127]
   int32_t offset;            // horizontal spot on texture the ray hit [0-63] (used in memory pointers so int32_t)
   uint8_t face;              // face of the block it hit (00=East Wall, 01=North, 10=West, 11=South Wall)
} RayStruct;

int32_t sqrt32(int32_t a);
int32_t sqrt_int(int32_t a, int8_t depth);

// absolute value
int32_t abs32(int32_t x);
int16_t abs16(int16_t x);
int8_t  abs8 (int8_t  x);
int32_t abs_int(int32_t a);

// sign function returns: -1 or 0 or 1 if input is <0 or =0 or >0
int8_t  sign8 (int8_t  x);
int16_t sign16(int16_t x);
int32_t sign32(int32_t x);


void LoadMapTextures();
void UnLoadMapTextures();
void GenerateSquareMap();
void GenerateRandomMap();
void GenerateMazeMap(int32_t startx, int32_t starty);

void walk(int32_t direction, int32_t distance);  
void shoot_ray(int32_t start_x, int32_t start_y, int32_t angle);

uint8_t getmap(int32_t x, int32_t y);
void setmap(int32_t x, int32_t y, uint8_t value);


void draw_textbox(GContext *ctx, GRect textframe, char *text);
void draw_map(GContext *ctx, GRect box, int32_t zoom);
void draw_3D(GContext *ctx, GRect box); //, int32_t zoom);

