#include "global.h"

uint8_t map[mapsize * mapsize];  // 0-255, 0-127 are squaretypes[], high bit set means ray hits wall

// TODO: Texture[] should be object with
GBitmap *texture[MAX_TEXTURES];
uint8_t *texpal[MAX_TEXTURES];
uint8_t *texdat[MAX_TEXTURES];
GBitmap *sprite_image[1];
GBitmap *sprite_mask[1];

SquareTypeStruct squaretype[10]; // note squaretype[0]=out of bounds ceiling/floor rendering
PlayerStruct player;
PlayerStruct object;
RayStruct ray;

  
// square root
#define root_depth 10          // How many iterations square root function performs
int32_t sqrt32(int32_t a) {int32_t b=a; for(int8_t i=0; i<root_depth; i++) b=(b+(a/b))/2; return b;} // Square Root
int32_t sqrt_int(int32_t a, int8_t depth){int32_t b=a; for(int8_t i=0; i<depth; i++) b=(b+(a/b))/2; return b;} // Square Root

// absolute value
int32_t abs32(int32_t x) {return (x^(x>>31)) - (x>>31);}
int16_t abs16(int16_t x) {return (x^(x>>15)) - (x>>15);}
int8_t  abs8 (int8_t  x) {return (x^(x>> 7)) - (x>> 7);}
int32_t abs_int(int32_t a){return (a<0 ? 0 - a : a);} // Absolute Value (might be faster than above)

// sign function returns: -1 or 0 or 1 if input is <0 or =0 or >0
int8_t  sign8 (int8_t  x){return (x > 0) - (x < 0);}
int16_t sign16(int16_t x){return (x > 0) - (x < 0);}
int32_t sign32(int32_t x){return (x > 0) - (x < 0);}

#ifdef PBL_COLOR
  //GColor testpal[2]={{.argb=0b11000000},{.argb=0b11110000}};
  GColor testpal[2]={{.argb=0b11000000},{.argb=0b11111111}};
  //GColor testpal2[]={{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0},{.argb=0}};
  //testpal[0] = GColorOrange;
  //testpal[1] = GColorBlue;
#endif

  
void LoadMapTextures() {
 
  const int Texture_Resources[] = {
    RESOURCE_ID_STONE,          //0
    RESOURCE_ID_WALL_FIFTY,     //1
    RESOURCE_ID_WALL_CIRCLE,    //2
    RESOURCE_ID_FLOOR_TILE,     //3
    RESOURCE_ID_CEILING_LIGHTS, //4
    RESOURCE_ID_WALL_BRICK,     //5
    //RESOURCE_ID_GRASS,
    RESOURCE_ID_REDBRICK,       //6
    RESOURCE_ID_WOOD16,         //7
    RESOURCE_ID_PrizeBox,       //8
    RESOURCE_ID_GRASS64,        //9
    RESOURCE_ID_TEST            //10
  };
  
  for(int i=0; i<MAX_TEXTURES; ++i) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "%d", i);
    texture[i] = gbitmap_create_with_resource(Texture_Resources[i]);
    
    if(texture[i]==NULL) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to load texture: %d", i);
    } else {
      #ifdef PBL_COLOR
        if(gbitmap_get_format(texture[i])==GBitmapFormat1Bit)
          gbitmap_set_data(texture[i], gbitmap_get_data(texture[i]), GBitmapFormat1BitPalette, gbitmap_get_bytes_per_row(texture[i]), true);
      
        if(gbitmap_get_format(texture[i])==GBitmapFormat1BitPalette)  
          gbitmap_set_palette(texture[i], testpal, false);
        
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%d: %d %d %lx", i, gbitmap_get_bytes_per_row(texture[i]), gbitmap_get_format(texture[i]), (uint32_t)gbitmap_get_palette(texture[i]));
      
        texpal[i] = (uint8_t*)gbitmap_get_palette(texture[i]);
        texdat[i] = (uint8_t*)gbitmap_get_data(texture[i]);
      
      #endif
      
    // popup message
    // Test:
    // Is width & height = 64px
    // is successful at loading into memory
    }
}

  
  // TODO: Change to: SPRITE[number of sprites][2] (image/mask)
  sprite_image[0] = gbitmap_create_with_resource(RESOURCE_ID_SPRITE_SMILEY);
  sprite_mask[0] = gbitmap_create_with_resource(RESOURCE_ID_SPRITE_SMILEY_MASK);
}

void UnLoadMapTextures() {
  for(uint8_t i=0; i<MAX_TEXTURES; i++)
    if(texture[i])
      gbitmap_destroy(texture[i]);
}

void GenerateSquareMap() {
  //Type 0 is how to render out-of-bounds
  squaretype[0].ceiling = 255; // outside sky
  squaretype[0].floor   = 9;   // outside grass
  squaretype[0].face[0]=squaretype[0].face[1]=squaretype[0].face[2]=squaretype[0].face[3]=10; // was 5

  squaretype[1].face[0]=squaretype[1].face[1]=squaretype[1].face[2]=squaretype[1].face[3]=10;//7; //5 for 1bit image
  squaretype[1].ceiling = 10;//4;
  squaretype[1].floor   = 10;//3;
  
  for (int16_t i=0; i<mapsize*mapsize; i++)
    map[i] = 1;                            // inside floor/ceiling
  
  for (int16_t i=0; i<mapsize; i++) {
    map[i*mapsize + 0]           = 128+1;  // west wall
    map[i*mapsize + mapsize - 1] = 128+1;  // east wall
    map[i]                       = 128+1;  // north wall
    map[(mapsize-1)*mapsize + i] = 128+1;  // south wall
  }
  map[((mapsize/2) * mapsize) + (mapsize/2)] = 128+1;  // middle block

  
   player.x = 1 * 64; player.y = (64*mapsize)/2; player.facing=0;    // start inside
   object.x = 2 * 64; object.y = (64*mapsize)/2; object.facing=0;    // sprite position
   //player.x = 6 * 32 + 16; player.y = (64*mapsize)/2; player.facing=TRIG_MAX_ANGLE/2;    // start inside
   //object.x = 3 * 32;      object.y = (64*mapsize)/2; object.facing=0;    // sprite position
//  player.x = ((64*mapsize)/2)-64; player.y = (64*mapsize)/2; player.facing=0;    // start inside
//  object.x = (64*mapsize)/2; object.y = (64*mapsize)/2; object.facing=0;    // sprite position
  //setmap(object.x, object.y, 0);
}

void GenerateRandomMap() {
  //squaretype[0].face[0] = 0; squaretype[0].face[1] = 0; squaretype[0].face[2] = 0; squaretype[0].face[3] = 0;
  squaretype[0].ceiling = 255;
  squaretype[0].floor = 6;

  squaretype[1].face[0] = 0;
  squaretype[1].face[1] = 5;
  squaretype[1].face[2] = 5;
  squaretype[1].face[3] = 0;
  squaretype[1].ceiling = 4;
  squaretype[1].floor = 3;
  
  //squaretype[2].face[0] = 0; squaretype[2].face[1] = 0; squaretype[2].face[2] = 0; squaretype[2].face[3] = 0;
  squaretype[2].ceiling = 2;
  squaretype[2].floor = 4;

  
  for (int16_t i=0; i<mapsize*mapsize; i++) map[i] = rand() % 3 == 0 ? 128+1 : 1;       // Randomly 1/3 of spots are [type 2] blocks, the rest are [type 1]
  for (int16_t i=0; i<mapsize*mapsize; i++) if(map[i]==1 && rand()%10==0) map[i]=128+2; // Change 10% of [type 2] blocks to [type 3] blocks
  //for (int16_t i=0; i<mapsize*mapsize; i++) if(map[i]==2 && rand()%2==0) map[i]=3;  // Changes 50% of [type 2] blocks to [type 3] blocks
}

// Generates maze starting from startx, starty, filling map with (0=empty, 1=wall, -1=special)
void GenerateMazeMap(int32_t startx, int32_t starty) {
  // Outside Type
  squaretype[0].face[0]=squaretype[0].face[1]=squaretype[0].face[2]=squaretype[0].face[3]=0;
  squaretype[0].ceiling = 255;
  squaretype[0].floor = 9;//6;  Grass

  //Wall and Empty
  squaretype[1].face[0]=squaretype[1].face[1]=squaretype[1].face[2]=squaretype[1].face[3]=10;//6;
  squaretype[1].ceiling = 4;
  squaretype[1].floor = 3;
  
  // Special
  squaretype[2].ceiling = 8;
  squaretype[2].floor = 8;

  int32_t x, y;
  int8_t try;
  int32_t cursorx, cursory, next=1;
  
  cursorx = startx; cursory=starty;  
  for (int16_t i=0; i<mapsize*mapsize; i++) map[i] = 0; // Fill map with 0s
  
  while(true) {
    int32_t current = cursory * mapsize + cursorx;
    if((map[current] & 15) == 15) {              // If all directions have been tried, then go to previous cell (unless you're back at the start)
      if(cursory==starty && cursorx==startx) {   // If back at the start, then we're done.
        map[current]=1;
        for (int16_t i=0; i<mapsize*mapsize; i++)
          if(map[i]==0) map[i] = 128+1;          // invert map bits (1=empty, 128+1=wall, 2=special)
        return;                                  // Maze is completed!
      }
      switch(map[current] >> 4) {                // Else go back to the previous cell:  NOTE: If the 1st two bits are used, need to "&3" mask this
       case 0: cursorx++; break;
       case 1: cursory++; break;
       case 2: cursorx--; break;
       case 3: cursory--; break;
      }
      map[current]=next; next=1;   // cells which have been double-traversed are not the end of a dead end, cause we backtracked through it, so set it to square-type 1
    } else {                       // not all directions have been tried
      do try = rand()%4; while (map[current] & (1<<try));  // Pick random direction until that direction hasn't been tried
      map[current] |= (1<<try); // turn on bit in this cell saying this path has been tried
      // below is just: x=0, y=0; if(try=0)x=1; if(try=1)y=1; if(try=2)x=-1; if(try=3)y=-1;
      y=(try&1); x=y^1; if(try&2){y=(~y)+1; x=(~x)+1;} //  y = try's 1st bit, x=y with 1st bit xor'd (toggled).  Then "Two's Complement Negation" if try's 2nd bit=1
      
      // Move if spot is blank and every spot around it is blank (except where it came from)
      if((cursory+y)>0 && (cursory+y)<mapsize-1 && (cursorx+x)>0 && (cursorx+x)<mapsize-1) // Make sure not moving to or over boundary
        if(map[(cursory+y) * mapsize + cursorx + x]==0)                                    // Make sure not moving to a dug spot
          if((map[(cursory+y-1) * mapsize + cursorx+x]==0 || try==1))                      // Nothing above (unless came from above)
            if((map[(cursory+y+1) * mapsize + cursorx+x]==0 || try==3))                    // nothing below (unless came from below)
              if((map[(cursory+y) * mapsize + cursorx+x - 1]==0 || try==0))                // nothing to the left (unless came from left)
                if((map[(cursory+y) * mapsize + cursorx + x + 1]==0 || try==2)) {          // nothing to the right (unless came from right)
                  cursorx += x; cursory += y;                                              // All's good!  Let's move
                  next=2;                                                                  // Set dead end spots as square-type 2
                  map[cursory * mapsize + cursorx] |= ((try+2)%4) << 4; //record in new cell where ya came from -- the (try+2)%4 is because when you move west, you came from east
                }
    }
  } //End While True
}

// ------------------------------------------------------------------------ //
//  General game functions
// ------------------------------------------------------------------------ //
void walk(int32_t direction, int32_t distance) {
  int32_t dx = (cos_lookup(direction) * distance) >> 16;
  int32_t dy = (sin_lookup(direction) * distance) >> 16;
  if(getmap(player.x + dx, player.y) < 128 || IDCLIP) player.x += dx;  // currently <128 so blocks rays hit user hits.  will change to walkthru type blocks
  if(getmap(player.x, player.y + dy) < 128 || IDCLIP) player.y += dy;
}


//shoot_ray(x, y, angle)
//  x, y = position on map to shoot the ray from
//  angle = direction to shoot the ray (in Pebble angle notation)
//modifies: global RayStruct ray
//    uses: getmap(), abs32()
void shoot_ray(int32_t start_x, int32_t start_y, int32_t angle) {
  int32_t sin, cos, dx, dy, nx, ny;  // sine & cosine, difference x&y, next x&y

    sin = sin_lookup(angle);
    cos = cos_lookup(angle);
  ray.x = start_x;// + (cos>>11); // fixes fisheye, but puts you inside walls if you're too close. was ((32*cos)>>16), 32 being dist from player to edge of view plane
  ray.y = start_y;// + (sin>>11); //
     ny = sin>0 ? 64 : -1;
     nx = cos>0 ? 64 : -1;

  do {
    do {
      dy = ny - (ray.y & 63);                        //   north-south component of distance to next east-west wall
      dx = nx - (ray.x & 63);                        //   east-west component of distance to next north-south wall
  
      if(abs32(dx * sin) < abs32(dy * cos)) {        // if(distance to north-south wall < distance to east-west wall) See Footnote 1
        ray.x += dx;                                   // move ray to north-south wall: x part
        ray.y += ((dx * sin) / cos);                   // move ray to north-south wall: y part
        ray.hit = getmap(ray.x, ray.y);                // see what the ray is at on the map
        if(ray.hit > 127) {                            // if ray hits a wall (a block)
          ray.face = cos>0 ? 2 : 0;                      // hit west or east face of block
          ray.offset = cos>0 ? 63-(ray.y&63) : ray.y&63; // Offset is where on wall ray hits: 0 (left edge) to 63 (right edge)
          ray.dist = ((ray.x - start_x) << 16) / cos;    // Distance ray traveled.    <<16 = * TRIG_MAX_RATIO
          return;                                      // Exit
        }
      } else {                                       // else: distance to Y wall < distance to X wall
        ray.x += (dy * cos) / sin;                     // move ray to east-west wall: x part
        ray.y += dy;                                   // move ray to east-west wall: y part
        ray.hit = getmap(ray.x, ray.y);                // see what the ray is at on the map
        if(ray.hit > 127) {                            // if ray hits a wall (a block)
            ray.face = sin>0 ? 3 : 1;                    // hit south or north face of block
          ray.offset = sin>0 ? ray.x&63 : 63-(ray.x&63); // Get offset: offset is where on wall ray hits: 0 (left edge) to 63 (right edge)
            ray.dist = ((ray.y - start_y) << 16) / sin;  // Distance ray traveled.    <<16 = * TRIG_MAX_RATIO
          return;                                      // Exit
        }
      }                                              // End if/then/else (x dist < y dist)
      
    } while(ray.hit>0);  //loop while ray is not out of bounds
  } while (!((sin<0&&ray.y<0) || (sin>0&&ray.y>=(mapsize<<6)) || (cos<0&&ray.x<0) || (cos>0&&ray.x>=(mapsize<<6))) ); // loop if ray is not going further out of bounds
  
  // ray will never hit a wall (out of bounds AND going further out of bounds)
  ray.hit  = 0;           // Never hit, so set to out-of-bounds block type (0)
  ray.face = 0;           // TODO: set face to face wall hit on block type 0 //ray.face = sin>0 ? (cos>0 ? 2 : 0) : (sin>0 ? 3 : 1);
  ray.dist = 0x7FFFFFFF;  // Never hits makes distance effectively infinity. 7F instead of FF cause of signed/unsigned conversion issues
  return;
}

uint8_t getmap(int32_t x, int32_t y) {
  x>>=6; y>>=6;
  return (x<0 || x>=mapsize || y<0 || y>=mapsize) ? 0 : map[(y * mapsize) + x];
}

void setmap(int32_t x, int32_t y, uint8_t value) {
  x>>=6; y>>=6;
  if((x >= 0) && (x < mapsize) && (y >= 0) && (y < mapsize))
    map[y * mapsize + x] = value;
}
