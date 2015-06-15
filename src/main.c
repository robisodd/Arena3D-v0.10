/**********************************************************************************
   Pebble 3D FPS Engine v0.9 beta
   Created by Rob Spiess (robisodd@gmail.com) on June 23, 2014
  *********************************************************************************
  v0.1: Initial Release
  
  v0.2: Converted all floating points to int32_t which increased framerate substancially
        Gets overflow errors, so when a number squared is negative, sets it to 2147483647.
        Added door blocks for another example
  
  v0.3: Added distance shading
        Removed door and zebra blocks
        
  v0.4: Optimizations
  
  v0.5: Changed 1000x1000 pixel blocks to 64 x 64
        Changed all /1000 to >>6 for quicker division (*64 = <<6)
        Updated Square Root Function
        Added 64x64bit Textures
        Added Texture Distance Shading overlay
        Modified FOV to give more square view (not squished or stretched)
        Added mirror block and black block
        Select button shoots ray to change blocks
  
  v0.6: Isolated shootray function
        Created new 3D routine - more optimzied and works better?
        All textures 32x32bits -> Faster due to single uint32_t
        Repaired shoot_ray function (no longer need ceiling function)
        
  v0.7: Cleaned up mess
        Added comments to inform those how this thing kinda works
        ray.dist is now unsigned
        raycasting is more optimized
        Got rid of floor/ceil function
        Super optimized raycasting (got rid of square root)
        Added strafing (hold the DOWN button to strafe)
        BEEFED up the drawing routine (direct framebuffer writing)
        Added MazeMap Generation
        Added Floor/Ceiling casting
        
  v0.8: Walls all now made of 32x32px blocks
        32x32bit textures map per block
        Wall heights are variable by increments of 4px
        
  v0.9: Scrapped v0.8, started back with v0.7
        Removed mirror blocks.  Won't work with how sprites will be implemented.
        Added 4 wall faces to blocks
        Changed map to unsigned (uint8 from int8)
        Map squares now point to Square_type array
        Adding sprite support...
        Got rid of FOV!  Now uses arctan to determine angle; MUCH straighter walls and better positioning
        Got rid of view GRect
        player.facing is now int16 -- removed all player.facing%TRIG_MAX_ANGLE and other large angle corrections
        
  To Do:
        Maybe X&Y coordinates can be int16 (max board of 512x512=256kB)
        Texture looping
        Add switches on walls to change cells
        Open doors
        Ceiling height?  Crushing
        
        Change how map/walls/textures/levels work:
          Levels specify size/shape, layout and which textures are to be loaded
          Any negative map square means ray passes through
          Map squares -128 to 127 point to array of Square_Type
          Square_Type[256] struct array holds fields:
            Ceiling texture (Texture_Array) (or none -- just sky)
            Floor Texture (Texture_Array)
            Each of the 4 walls have a separate texture (or 0 for clear on this side), uint8_t index of Texture_Array
              Texture_Array[256 (or less)] structure:
              Holds pointer to texture
              Info if texture is 32x32 or 64x64 or 64x32 or 16x32 or whatever
            Possible inner/outer texture depending on direction hit?
              Currently texture is invisible from the inside
            Darkness Amount (and if distance shading is on/off)
            Permissable to walk through
            Permissable for enemies to walk through
            Permissable for bullets/missles/magic?
            Invert texture bit? uint8_t = 4 walls + ceiling + floor + 2 spare bits
            Maybe ray modifying? e.g. Mirror or Light-bending gravity well?  No, nevermind.
            Note: Square_Type[0] is out of bounds render type
            
        Sprite shading is the shading of the square it's in.
                      
Definitions:
  Object: enemy or item or other thing
  Sprite: object as displayed on screen
          Tile: 
          Square: X&Y on map
                  10x10 map has 100 squares
                  each square is 8bits: High bit=ray hits walls, 7bits=squaretype
                  = coordinate / 64 (aka >>6)
          SquareType: [0-127] Type of square for the map
                      Holds information about textures, passible, etc
          Map: Grid (or Array) of squares
               Mabye should rename to Level or PlayField? Map sounds like 2D top-down
          Wall:  What a ray hits, usually impassible by player
          Coordinate: X & Y position on map
                      Pixel specific, 64 per square
                      e.g. 10x10 map has 640x640=409600 coordinate points
          Shading:
          

  To Fix:
        Wall textures map to exact coordinate, not distance from left edge
          So textures on corners don't line up
          e.g. Textures on parallel sides of map square have texture going same direction.
          
  *********************************************************************************
  Created with the help of these tutorials:
    http://www.playfuljs.com/a-first-person-engine-in-265-lines/
    http://www.permadi.com/tutorial/raycast/index.html

  CC Copyright (c) 2015 All Right Reserved
  THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
  http://creativecommons.org/licenses/by/3.0/legalcode
  
  *********************************************************************************
   Notes and Ideas
  *********************************************************************************
   Poisoned = Black Boxes and White Blocks -- i.e. Renders without Textures or changes texture
   Night time = Darkness fades off closer.  Can only see 3 away unless torch is lit
   Full brightness = no darkness based on distance
   Mirror seems to reflect magic, too. Magic repulsing shield?
   Rain or Snow overlay
     Snow would eventually change ground to white, maybe raise lower-wall level when deep, slowing walking
     Rain would also increase darkness due to cloudcover.  Black sky?  Clouds in sky?
   Drunk or Poisoned mode: autowalk or autoturn or random strafing when walking or inverted accel.x
   IDCLIP on certain block types -- walk through blocks which look solid (enemies can't penetrate to expose - unless in chase mode?)
   
   
   map[] uint8_t  bit:76543210
     bit7: invisible walls (ray passes through): 0 yes, 1 no
           currently also if can walk through (though this will change)
     
  
  *********************************************************************************/
// 529a7262-efdb-48d4-80d4-da14963099b9
#include "global.h"

extern uint8_t map[mapsize * mapsize];  // 0-255, 0-127 are squaretypes[], high bit set means ray hits wall

extern GBitmap *texture[MAX_TEXTURES];
extern GBitmap *sprite_image[1];
extern GBitmap *sprite_mask[1];

extern SquareTypeStruct squaretype[10]; // note squaretype[0]=out of bounds ceiling/floor rendering
extern PlayerStruct player;
extern PlayerStruct object;
extern RayStruct ray;

static Window *window;
Layer *graphics_layer;

  static bool up_button_depressed = false; // Whether Pebble's   Up   button is held
  static bool dn_button_depressed = false; // Whether Pebble's  Down  button is held
  static bool sl_button_depressed = false; // Whether Pebble's Select button is held
//static bool bk_button_depressed = false; // Whether Pebble's  Back  button is held

// ------------------------------------------------------------------------ //


// ------------------------------------------------------------------------ //
//  Map Functions
// ------------------------------------------------------------------------ //

static void main_loop(void *data) {
  AccelData accel=(AccelData){.x=0, .y=0, .z=0};          // all three are int16_t
  accel_service_peek(&accel);                             // read accelerometer
  walk(player.facing, accel.y>>5);                        // walk based on accel.y
  if(dn_button_depressed)                                 // if down button is held
    walk(player.facing + (1<<14), accel.x>>5);            //   strafe (1<<14 = TRIG_MAX_ANGLE / 4)
  else                                                    // else
    player.facing = (player.facing + (accel.x<<3));       //   spin
  
  //if(dn_button_depressed) player.facing = (player.facing + 300);       //   if dn is held, spin
  
  layer_mark_dirty(graphics_layer);                       // tell pebble to draw when it's ready
}


static void graphics_layer_update_proc(Layer *me, GContext *ctx) {
  static char text[40];  //Buffer to hold text
  time_t sec1, sec2; uint16_t ms1, ms2, dt; // time snapshot variables, to calculate render time and FPS
  time_ms(&sec1, &ms1);  //1st Time Snapshot
  
  //draw_3D(ctx,  GRect(view_x, view_y, view_w, view_h));
  draw_3D(ctx,  GRect(1, 34, 142, 128));
//  draw_3D(ctx,  GRect(4, 110, 40, 40));
   draw_map(ctx, GRect(4, 110, 40, 40), 4);
  
  time_ms(&sec2, &ms2);  //2nd Time Snapshot
  dt = (uint16_t)(1000*(sec2 - sec1)) + (ms2 - ms1);  //dt=delta time: time between two time snapshots in milliseconds
  
  
  
	int16_t  w 	= gbitmap_get_bounds(texture[7]).size.w;
// 	int16_t  h 	= gbitmap_get_bounds(texture[7]).size.h;
  int16_t  h 	= gbitmap_get_bytes_per_row(texture[7]);
  
    
  
  snprintf(text, sizeof(text), "(%ld,%ld) %ld %ldms %ldfps\n%ld %ld", (long)player.x, (long)player.y, (long)player.facing, (long)dt, (long)(1000/dt), (long)w, (long)h);  // What text to draw
//   snprintf(text, sizeof(text), "(%ld,%ld) %ld %ldms %ldfps", (long)player.x, (long)player.y, (long)player.facing, (long)dt, (long)(1000/dt));  // What text to draw
//   snprintf(text, sizeof(text), "%db (%ld,%ld) %d\n%ld %ld %ld %ld %ld", heap_bytes_free(), player.x, player.y, player.facing, Q1,Q2,Q3,Q4,Q5);  // What text to draw
  
  draw_textbox(ctx, GRect(0, 0, 143, 32), text);
   
  //  Set a timer to restart loop in 50ms
  if(dt<40 && dt>0) // if time to render is less than 40ms, force framerate of 20FPS or worse
     app_timer_register(50-dt, main_loop, NULL); // 10FPS
  else
     app_timer_register(10, main_loop, NULL);     // took longer than 40ms, loop  in 10ms (asap)
}

// ------------------------------------------------------------------------ //
//  Button Pushing
// ------------------------------------------------------------------------ //
void up_push_in_handler(ClickRecognizerRef recognizer, void *context) {up_button_depressed = true;
                                                                      GenerateMazeMap(mapsize/2, 0);
                                                                      }
void up_release_handler(ClickRecognizerRef recognizer, void *context) {up_button_depressed = false;}
void dn_push_in_handler(ClickRecognizerRef recognizer, void *context) {dn_button_depressed = true;}
void dn_release_handler(ClickRecognizerRef recognizer, void *context) {dn_button_depressed = false;}
void sl_push_in_handler(ClickRecognizerRef recognizer, void *context) {sl_button_depressed = true;   // SELECT button was pushed in
  shoot_ray(player.x, player.y, player.facing);             // Shoot Ray from center of screen.  If it hit something:
  //if(ray.hit>127) 
  setmap(ray.x, ray.y, ray.hit&127); // If you shoot a block, remove it.
  //if(ray.hit==128+1) setmap(ray.x, ray.y, 1);   // If Ray hit normal block(1), change it to a Circle Block (3) (Changed from Mirror Block(4), as it was confusing)
  //if(ray.hit==128+3) setmap(ray.x, ray.y, 1);   // If Ray hit Circle Block(3), change it to a Normal Block (1)
}
void sl_release_handler(ClickRecognizerRef recognizer, void *context) {sl_button_depressed = false;}

void click_config_provider(void *context) {
  window_raw_click_subscribe(BUTTON_ID_UP, up_push_in_handler, up_release_handler, context);
  window_raw_click_subscribe(BUTTON_ID_DOWN, dn_push_in_handler, dn_release_handler, context);
  window_raw_click_subscribe(BUTTON_ID_SELECT, sl_push_in_handler, sl_release_handler, context);
  //window_single_click_subscribe(BUTTON_ID_SELECT, sl_push_in_handler);
}

// ------------------------------------------------------------------------ //
//  Main Program Structure
// ------------------------------------------------------------------------ //
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  graphics_layer = layer_create(layer_get_frame(window_layer));
  layer_set_update_proc(graphics_layer, graphics_layer_update_proc);
  layer_add_child(window_layer, graphics_layer);
}

static void window_unload(Window *window) {
  layer_destroy(graphics_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  
  IF_BW(window_set_fullscreen(window, true));   // make sure full screen
  
  window_stack_push(window, false /* False = Not Animated */);
  window_set_background_color(window, GColorBlack);
  accel_data_service_subscribe(0, NULL);  // Start accelerometer
  
  srand(time(NULL));  // Seed randomizer so different map every time
  player = (struct PlayerStruct){.x=(64*(mapsize/2)), .y=(-2 * 64), .facing=10000};    // Seems like a good place to start
  object = (struct PlayerStruct){.x=(2 * 64), .y=(64*(mapsize/2)), .facing=10000};    // sprite position
  //GenerateRandomMap();                // generate a randomly dotted map
  //GenerateMazeMap(mapsize/2, 0);    // generate a random maze, enterane on middle of top side
  GenerateSquareMap();
  
  LoadMapTextures(); // Load textures
  // MainLoop() will be automatically started with dirty layer drawing
}

static void deinit(void) {
  accel_data_service_unsubscribe();
  window_destroy(window);
  UnLoadMapTextures();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

// ------------------------------------------------------------------------ //
//  Footnotes
// ------------------------------------------------------------------------ //
// 1: if(abs32(dx * sin) < abs32(dy * cos)) {
//   translates to: if("distance to x wall" is smaller than "distance to y wall")
//      Used to be: if((dx*TRIG_MAX_RATIO/cos) < (dy*TRIG_MAX_RATIO/sin)).  Added abs cause of sign changes
//   & before that: if(sqrt(dxx*dxx + dxy*dxy) < sqrt(dyx*dyx + dyy*dyy)), though got rid of "sqrt" on both sides
//   To learn more: www.permadi.com/tutorial/raycast/rayc8.html

// 2: xoffset = (y * ray.dist / box.size.h) >> 16;
//    was 
// total_column_height = box.size.h / (dist>>16) (Total height of 32pixel wall half. This isn't clipped to screen, so could be thousands of pixels tall)
// i IS clipped to screen and goes from 3D view's middle to 3D view's top/bottom edge
// which pixel in the texture hit is i/total_colum_height = i / (box.size.h / (dist>>16)) = (i * dist / box.size.h) >> 16
