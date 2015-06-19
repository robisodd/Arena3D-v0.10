// ------------------------------------------------------------------------ //
//  Color Drawing Functions
// ------------------------------------------------------------------------ //
#ifdef PBL_COLOR
#include "global.h"

extern uint8_t map[mapsize * mapsize];

extern SquareTypeStruct squaretype[]; // note squaretype[0]=out of bounds ceiling/floor rendering
extern PlayerStruct player;
extern PlayerStruct object;
extern RayStruct ray;

extern GBitmap *texture[MAX_TEXTURES];
extern GBitmap *sprite_image[1];
extern GBitmap *sprite_mask[1];
extern Layer *graphics_layer;

// ------------------------------------------------------------------------ //
//  Drawing to screen functions
// ------------------------------------------------------------------------ //
// void fill_window(GContext *ctx, uint8_t *data) {
//   for(uint16_t y=0, yaddr=0; y<168; y++, yaddr+=20)
//     for(uint16_t x=0; x<19; x++)
//       ((uint8_t*)(((GBitmap*)ctx)->addr))[yaddr+x] = data[y%8];
// }

void draw_textbox(GContext *ctx, GRect textframe, char *text) {
    graphics_context_set_fill_color(ctx, GColorBlack);   graphics_fill_rect(ctx, textframe, 0, GCornerNone);  //Black Solid Rectangle
    graphics_context_set_stroke_color(ctx, GColorWhite); graphics_draw_rect(ctx, textframe);                //White Rectangle Border  
    graphics_context_set_text_color(ctx, GColorWhite);  // White Text
    graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_14), textframe, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);  //Write Text
}


// 1-pixel-per-square map:
//   for (int16_t x = 0; x < mapsize; x++) for (int16_t y = 0; y < mapsize; y++) {graphics_context_set_stroke_color(ctx, map[y*mapsize+x]>0?1:0); graphics_draw_pixel(ctx, GPoint(x, y));}
void draw_map(GContext *ctx, GRect box, int32_t zoom) {
  // note: Currently doesn't handle drawing beyond screen boundaries
  // zoom = pixel size of each square
  GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);
  if(framebuffer) {   // if successfully captured the framebuffer
    uint8_t* screen = gbitmap_get_data(framebuffer);
//   uint32_t *ctx32 = ((uint32_t*)(((GBitmap*)ctx)->addr));  // framebuffer pointer (screen memory)

    int32_t x, y, yaddr, xaddr, xonmap, yonmap, yonmapinit;

    xonmap = ((player.x*zoom)>>6) - (box.size.w/2);  // Divide by ZOOM to get map X coord, but rounds [-ZOOM to 0] to 0 and plots it, so divide by ZOOM after checking if <0
    yonmapinit = ((player.y*zoom)>>6) - (box.size.h/2);
    for(x=0; x<box.size.w; x++, xonmap++) {
      xaddr = x+box.origin.x;        // X memory address

      if(xonmap>=0 && xonmap<(mapsize*zoom)) {
        yonmap = yonmapinit;
        yaddr = box.origin.y * 144;           // Y memory address

        for(y=0; y<box.size.h; y++, yonmap++, yaddr+=144) {
          if(yonmap>=0 && yonmap<(mapsize*zoom)) {             // If within Y bounds
            if(map[(((yonmap/zoom)*mapsize))+(xonmap/zoom)]>127) //   Map shows a wall >127
              screen[xaddr + yaddr] = 0b11111111;              //     White dot
            else                                               //   Map shows <= 0
              screen[xaddr + yaddr] = 0b11000000;              //     Black dot
          } else {                                             // Else: Out of Y bounds
            screen[xaddr + yaddr] = 0b11000000;                 //   Black dot
          }
        }
      } else {                                // Out of X bounds: Black vertical stripe
        for(yaddr=box.origin.y*144; yaddr<((box.size.h + box.origin.y)*144); yaddr+=144)
          screen[xaddr + yaddr] = 0b11000000;
      }
    }

    graphics_release_frame_buffer(ctx, framebuffer);
  }  // endif successfully captured framebuffer

  graphics_context_set_fill_color(ctx, (GColor){.argb=((time_ms(NULL, NULL) % 250)>125?0b11000000:0b11111111)});                      // Flashing dot

  graphics_fill_rect(ctx, GRect((box.size.w/2)+box.origin.x - 1, (box.size.h/2)+box.origin.y - 1, 3, 3), 0, GCornerNone); // Square Cursor

  graphics_context_set_stroke_color(ctx, GColorWhite); graphics_draw_rect(ctx, GRect(box.origin.x-1, box.origin.y-1, box.size.w+2, box.size.h+2)); // White Border



}



int32_t Q1=0, Q2=0, Q3=0, Q4=0, Q5=0;
// implement more options
//draw_3D_wireframe?  draw_3D_shaded?
void draw_3D(GContext *ctx, GRect box) { //, int32_t zoom) {

//mid_y = (box.size.h/2) or maybe box.origin.y + (box.size.h/2) (middle in view or pixel on screen)
//mid_x = (box.size.w/2) or maybe box.origin.x + (box.size.w/2)
 int32_t dx, dy;
  int16_t angle;
  int32_t farthest = 0; //colh, z;
  int32_t y, colheight, halfheight;
  uint32_t x, addr, xaddr, yaddr, xbit, xoffset, yoffset;
  uint32_t *target, *mask;
  
  
///  int32_t dist[144];                 // Array of non-cos adjusted distance for each vertical wall segment -- for sprite rendering
  halfheight = box.size.h/2;
//   uint32_t *ctx32 = ((uint32_t*)(((GBitmap*)ctx)->addr));  // framebuffer pointer (screen memory)
  
  // Draw Box around view (not needed if fullscreen)
  //TODO: Draw straight to framebuffer
  if(view_border) {graphics_context_set_stroke_color(ctx, GColorWhite); graphics_draw_rect(ctx, GRect(box.origin.x-1, box.origin.y-1, box.size.w+2, box.size.h+2));}  //White Rectangle Border

  // Draw background
    graphics_context_set_fill_color(ctx, GColorBlack);  graphics_fill_rect(ctx, box, 0, GCornerNone); // Black background
    // Draw Sky from horizion on up, rotate based upon player angle
    //graphics_context_set_fill_color(ctx, 1); graphics_fill_rect(ctx, GRect(box.origin.x, box.origin.y, box.size.w, box.size.h/2), 0, GCornerNone); // White Sky  (Lightning?  Daytime?)

  GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);
  if(framebuffer) {   // if successfully captured the framebuffer
    uint8_t* screen = gbitmap_get_data(framebuffer);

  for(int16_t col = 0; col < box.size.w; col++) {        // Begin RayTracing Loop
    angle = atan2_lookup((64*col/box.size.w) - 32, 64);    // dx = (64*(col-(box.size.w/2)))/box.size.w; dy = 64; angle = atan2_lookup(dx, dy);
    
    shoot_ray(player.x, player.y, player.facing + angle);  //Shoot rays out of player's eyes.  pew pew.
    ray.hit &= 127;                                        // Whether ray hit a block (>127) or not (<128), set ray.hit to valid block type [0-127]
    if(ray.dist > (uint32_t)farthest) farthest = ray.dist; // farthest (furthest?) wall (for sprite rendering. only render sprites closer than farthest wall)
///    dist[col] = (uint32_t)ray.dist;                        // save distance of this column for sprite rendering later
    ray.dist *= cos_lookup(angle);                         // multiply by cos to stop fisheye lens (should be >>16 to get actual dist, but that's all done below)
//  ray.dist <<= 16;    // use this if commenting out "ray.dist*=cos" above, cause it >>16's a lot below
    
      // Calculate amount of shade
      //z =  ray.dist >> 16; //z=(ray.dist*cos_lookup(angle))/TRIG_MAX_RATIO;  // z = distance
      //z -= 64; if(z<0) z=0;   // Make everything 1 block (64px) closer (solid white without having to be nearly touching)
      //z = sqrt_int(z,10) >> 1; // z was 0-RANGE(max dist visible), now z = 0 to 12: 0=close 10=distant.  Square Root makes it logarithmic
      //z -= 2; if(z<0) z=0;    // Closer still (zWas=zNow: 0-64=0, 65-128=2, 129-192=3, 256=4, 320=6, 384=6, 448=7, 512=8, 576=9, 640=10)
      // end shade calculation
    
      colheight = (box.size.h << 21) /  ray.dist;    // half wall segment height = box.size.h * wallheight * 64(the "zoom factor") / (distance >> 16) // now /2 (<<21 instead of 22)
      if(colheight>halfheight) colheight=halfheight; // Make sure line isn't drawn beyond bounding box (also halve it cause of 2 32bit textures)
      
      // Texture the Ray hit, point to 1st half of texture (half, cause a 64x64px texture menas there's 2 uint32_t per texture row.  Also why * 2 below)
//       target = (uint32_t*)texture[squaretype[ray.hit].face[ray.face]]->addr + ray.offset*2;// maybe use GBitmap's size veriables to store texture size?

///    uint8_t *target2;
///    target = (uint32_t*)gbitmap_get_data(texture[squaretype[ray.hit].face[ray.face]]);
//      target2 = (uint8_t*)target;
//      target2 = target2 + (ray.offset*32);  // * gbitmap_get_bytes_per_row(the texutre)
///    target = target + (ray.offset*2);
    
    
    
///    x = col+box.origin.x;  // X screen coordinate
///    addr = x + ((box.origin.y + halfheight) * 144); // 32bit memory word containing pixel vertically centered at X. (Address=xaddr + yaddr = (Pixel.X/32) + 5*Pixel.Y)
//       addr = (x >> 5) + ((box.origin.y + halfheight) * 5); // 32bit memory word containing pixel vertically centered at X. (Address=xaddr + yaddr = (Pixel.X/32) + 5*Pixel.Y)
//       xbit = x & 31;        // X bit-shift amount (for which bit within screen memory's 32bit word the pixel exists)

////       y=0; yoffset=0;  // y is y +/- from vertical center, yoffset is the screen memory position of y (and is always = y*5)
////       for(; y<colheight; y++, yoffset+=144) {
////         xoffset = (y * ray.dist / box.size.h) >> 16; // xoffset = which pixel of the texture is hit (0-31).  See Footnote 2
////         screen[addr - yoffset] = *(target2 + ((32 + (31-xoffset))/2));  // Draw Top Half
////         screen[addr + yoffset] = *(target2 + ((32 - xoffset)/2));  // Draw Bottom Half

        
        
//         How to work with Color Palette
//  void replace_gbitmap_color(GColor color_to_replace, GColor replace_with_color, GBitmap *im){
// 	GColor *current_palette = gbitmap_get_palette(im);
 
// 	for(int i = 0; i < 2; i++){
// 		if ((color_to_replace.argb & 0x3F)==(current_palette[i].argb & 0x3F)){
// 			current_palette[i].argb = (current_palette[i].argb & 0xC0)| (replace_with_color.argb & 0x3F);
// 		}
// 	}
// }

        
//         screen[addr - yoffset] = (((*target >> (31-xoffset))&1))*0b11110000;  // Draw Top Half
//         screen[addr + yoffset] = (((*(target+1)  >> xoffset)&1))*0b11110000;  // Draw Bottom Half

////       }

    
//====== This works to convert 1bit B&W walls to 1bit color walls ======//
//     target = (uint32_t*)gbitmap_get_data(texture[squaretype[ray.hit].face[ray.face]]) + ray.offset*2;// maybe use GBitmap's size veriables to store texture size?
//     x = col+box.origin.x;  // X screen coordinate
//     addr = x + ((box.origin.y + halfheight) * 144); // 32bit memory word containing pixel vertically centered at X. (Address=xaddr + yaddr = (Pixel.X/32) + 5*Pixel.Y)
//     y=0; yoffset=0;  // y is y +/- from vertical center, yoffset is the screen memory position of y (and is always = y*5)
//     for(; y<colheight; y++, yoffset+=144) {
//       xoffset = (y * ray.dist / box.size.h) >> 16; // xoffset = which pixel of the texture is hit (0-31).  See Footnote 2
//       screen[addr - yoffset] = (((*target >> (31-xoffset))&1))?0b11110000:0b11000000;  // Draw Top Half
//       screen[addr + yoffset] = (((*(target+1)  >> xoffset)&1))?0b11110000:0b11000000;  // Draw Bottom Half
//     }
//=====================================================================//
    
    
    // Draw Color Walls (Both colored 1bit and GBitmapFormat4BitPalette color)
    if(gbitmap_get_bytes_per_row(texture[squaretype[ray.hit].face[ray.face]])==8) {
      // IF 1bit texture
      // 64px / 8Bytes/row = 8px/Byte = 1bit/px
      target = (uint32_t*)gbitmap_get_data(texture[squaretype[ray.hit].face[ray.face]]) + ray.offset*2;// maybe use GBitmap's size veriables to store texture size?
      x = col+box.origin.x;  // X screen coordinate
      addr = x + ((box.origin.y + halfheight) * 144); // address of pixel vertically centered at X. (Address=xaddr + yaddr = Pixel.X + 144*Pixel.Y)
      y=0; yoffset=0;  // y is y +/- from vertical center, yoffset is the screen memory position of y (and is always = y*144)
      for(; y<colheight; y++, yoffset+=144) {
        xoffset = (y * ray.dist / box.size.h) >> 16; // xoffset = which pixel of the texture is hit (0-31).  See Footnote 2
        screen[addr - yoffset] = (((*target >> (31-xoffset))&1))?0b11110000:0b11000000;  // Draw Top Half
        screen[addr + yoffset] = (((*(target+1)  >> xoffset)&1))?0b11110000:0b11000000;  // Draw Bottom Half
      }
    } else {
      // Else: Draw 4bits/px (16 color) texture (note: Texture size is 4bits/px * 64*64px = 2048 Bytes)
      //64px / 32Bytes/row = 2px/Byte = 4bits/px
      //1px = xxxx (4 bits per pixel, 16 colors)
      //aaaabbbb ccccdddd eeeeffff gggghhhh = XXXX (1 set of 32bits (= 8 pixels))
      //AAAA BBBB CCCC DDDD (mid) EEEE FFFF GGGG HHHH = row (1 row = 8 sets of 32bits (= 8 sets of 8 pixels = 64 pixels))

      GColor *palette = gbitmap_get_palette(texture[squaretype[ray.hit].face[ray.face]]);
      target = (uint32_t*)gbitmap_get_data(texture[squaretype[ray.hit].face[ray.face]]); // Puts pointer at texture beginning // maybe use GBitmap's size veriables to store texture size?
      target += ray.offset*8;  // Puts pointer at row beginning // ray.offset is y position on texture = [0-63].  8 sets of 32bits = 1 row
      target += 4; // puts pointer at (mid) of row (4 changes depending on palette)
      
      x = col+box.origin.x;  // X screen coordinate
      addr = x + ((box.origin.y + halfheight) * 144); // address of pixel vertically centered at X. (Address=xaddr + yaddr = Pixel.X + 144*Pixel.Y)
      y=0; yoffset=0;  // y is y +/- from vertical center, yoffset is the screen memory position of y (and is always = y*144)
      for(; y<colheight; y++, yoffset+=144) {
        xoffset = (y * ray.dist / box.size.h) >> 16; // xoffset = which pixel of the texture is hit (0-31).  See Footnote 2
        screen[addr - yoffset] = palette[((*(target - 1 - (xoffset>>3)) >> ((7-(xoffset&7))*4) )&15)].argb;  // Draw Top Half
        screen[addr + yoffset] = palette[((*(target     + (xoffset>>3)) >> (   (xoffset&7) *4) )&15)].argb;  // Draw Bottom Half
      }
    }


    // Draw Floor/Ceiling
    int32_t temp_x = (((box.size.h << 5) * cos_lookup(player.facing + angle)) / cos_lookup(angle)); // Calculate now to save time later
    int32_t temp_y = (((box.size.h << 5) * sin_lookup(player.facing + angle)) / cos_lookup(angle)); // Calculate now to save time later
//if(false)  // enable/disable floor and ceiling
    for(; y<halfheight; y++, yoffset+=144) {         // y and yoffset continue from wall top&bottom to view edge (unless wall is taller than view edge)
      int32_t map_x = player.x + (temp_x / y);     // map_x & map_y = spot on map the screen pixel points to
      int32_t map_y = player.y + (temp_y / y);     // map_y = player.y + dist_y, dist = (height/2 * 64 * (sin if y, cos if x) / i) (/cos to un-fisheye)
      ray.hit = getmap(map_x, map_y) & 127;        // ceiling/ground of which cell is hit.  &127 shouldn't be needed since it *should* be hitting a spot without a wall
      if(squaretype[ray.hit].floor<MAX_TEXTURES)   // If ceiling texture exists (else just show sky)
        screen[addr + yoffset] = (((*( ((uint32_t*)gbitmap_get_data(texture[squaretype[ray.hit].floor]) + (map_x&63) * 2) + ((map_y&63) >> 5)) >> (map_y&31))&1)) * 0b11001100;
      if(squaretype[ray.hit].ceiling<MAX_TEXTURES) // If floor texture exists (else just show abyss)
        screen[addr - yoffset] = (((*( ((uint32_t*)gbitmap_get_data(texture[squaretype[ray.hit].ceiling]) + (map_x&63) * 2) + ((map_y&63) >> 5)) >> (map_y&31))&1)) * 0b11000011;
    } // End Floor/Ceiling

    
  } //End For (End RayTracing Loop)

  // Draw Sprites!
  // Sort sprites by distance from player
  // draw sprites in order from farthest to closest
  // start from sprites closer than "farthest wall"
  // sprite:
  // x
  // y
  // angle
  // distance
  // type
  // d

/*
// =======================================SPRITES====================================================
 
  uint8_t numobjects=1;
  int32_t spritecol, objectdist;  //, xoffset, yoffset;
//if(false)  // enable/disable drawing of sprites
  for(uint8_t obj=0; obj<numobjects; obj++) {
    dx = object.x - player.x;
    dy = object.y - player.y;
    angle = atan2_lookup(dy, dx); // angle = angle between player's x,y and sprite's x,y
    objectdist =  (((dx^(dx>>31)) - (dx>>31)) > ((dy^(dy>>31)) - (dy>>31))) ? (dx<<16) / cos_lookup(angle) : (dy<<16) / sin_lookup(angle);
//     objectdist = (abs32(dx)>abs32(dy)) ? (dx<<16) / cos_lookup(angle) : (dy<<16) / sin_lookup(angle);
    angle = angle - player.facing;  // angle is now angle between center view column and object. <0=left of center, 0=center column, >0=right of center
    
    if(cos_lookup(angle)>0) { // if object is in front of player.  note: if angle = 0 then object is straight right or left of the player
      if(farthest>=objectdist) { // if ANY wall is further (or =) than object distance, then display it
        spritecol = (box.size.w/2) + ((sin_lookup(angle)*box.size.w)>>16);  // column on screen of sprite center

        int32_t objectwidth = 32;           // 32 pixels wide  TODO: maybe other sized objects?  Besides scaling?
        int32_t objectheight = 32;//16          // 32 pixels tall
        int32_t objectverticaloffset = 64-objectheight;//+32;//16; // normally center dot is vertically centered, + or - how far to move it.
//         int32_t spritescale = box.size.h ;// * 20 / 10;
        
        //objectdist = (objectdist * cos_lookup(angle)) >> 16;
        
        int32_t spritescale = (box.size.h);// * 20 / 10;
        int32_t spritewidth  = (spritescale * objectwidth) / objectdist;   // should be box.size.w, but wanna keep sprite h/w proportionate
        int32_t spriteheight = (spritescale * objectheight)/ objectdist;  // note: make sure to use not-cosine adjusted distance!
//         int32_t halfspriteheight = spriteheight/2;
        int32_t spriteverticaloffset = (objectverticaloffset * spritescale) / objectdist; // fisheye adjustment
//         int32_t spriteverticaloffset = ((((objectverticaloffset * spritescale) + (32*box.size.h)) << 16) / (objectdist * cos_lookup(angle))); // floor it
        
        
        int16_t sprite_xmin = spritecol - (spritewidth/2);
        int16_t sprite_xmax = sprite_xmin + spritewidth;  // was =spritecol+(spritewidth/2);  Changed to display whole sprite cause /2 loses info
        if(sprite_xmax>=0 && sprite_xmin<box.size.w) {    // if any of the sprite is horizontally within view
          int16_t xmin = sprite_xmin<0 ? 0: sprite_xmin;
          int16_t xmax = sprite_xmax>box.size.w ? box.size.w : sprite_xmax;


// Half through floor
//int32_t objectheight = 16;          // 32 pixels tall
//int32_t objectverticaloffset = 64-objectheight;//+32;//16; // normally center dot is vertically centered, + or - how far to move it.
          
// perfectly puts 32x32 sprite on ceiling
//int32_t objectwidth = 32;
//int32_t objectheight = 64;
//int32_t objectverticaloffset = 0;
//int32_t spritescale = box.size.h;
//int32_t spritewidth  = (spritescale * objectwidth) / objectdist;
//int32_t spriteheight = (spritescale * objectheight) / objectdist;
//int32_t spriteverticaloffset = ((objectverticaloffset * spritescale) << 16) / (objectdist * cos_lookup(angle)); // fisheye adjustment
//int16_t sprite_ymax = spriteverticaloffset + ((box.size.h + spriteheight)/2);// + (((32*box.size.h) << 16) / (objectdist * cos_lookup(angle)));
//int16_t sprite_ymin = sprite_ymax - spriteheight; // note: sprite is not cos adjusted but offset is (to keep it in place)

          
          int16_t sprite_ymax = (box.size.h + spriteheight + spriteverticaloffset)/2;// + (((32*box.size.h) << 16) / (objectdist * cos_lookup(angle)));
          int16_t sprite_ymin = sprite_ymax - spriteheight; // note: sprite is not cos adjusted but offset is (to keep it in place)
          
//           int16_t sprite_ymin = halfheight + spriteverticaloffset - spriteheight; // note: sprite is not cos adjusted but offset is (to keep it in place)
//           int16_t sprite_ymax = halfheight + spriteverticaloffset;
    

          
          if(sprite_ymax>=0 && sprite_ymin<box.size.h) { // if any of the sprite is vertically within view
            int16_t ymin = sprite_ymin<0 ? 0 : sprite_ymin;
            int16_t ymax = sprite_ymax>box.size.h ? box.size.h : sprite_ymax;
///BEGIN DRAWING LOOPS
            for(int16_t x = xmin; x < xmax; x++) {
              if(dist[x]>=objectdist) {  // if not behind wall
                xaddr = (box.origin.x + x) >> 5;
                xbit  = (box.origin.x + x) & 31;
                xoffset = ((x - sprite_xmin) * objectdist) / spritescale; // x point hit on texture -- make sure to use the original object dist, not the cosine adjusted one
                mask   = (uint32_t*)sprite_mask[0]->addr + xoffset;  // mask = mask
                target = (uint32_t*)sprite_image[0]->addr + xoffset; // target = sprite
                yaddr = (box.origin.y + ymin) * 5;
                
                for(int16_t y=ymin; y<ymax; y++, yaddr+=5) {
                  //graphics_draw_pixel(ctx, GPoint(box.origin.x + x, box.origin.y + y));
                  yoffset = ((y - sprite_ymin) * objectdist) / spritescale; // y point hit on texture column (was = (objectheight*(y-sprite_ymin))/spriteheight)
                  if(((*mask >> yoffset) & 1) == 1) {   // If mask point isn't clear, then draw point.  TODO: try removing == 1
                    ctx32[xaddr + yaddr] &= ~(1 << xbit);  // blacken bit
                  //ctx32[xaddr + yaddr] |= 1 << xbit;     // whiten bit
                    ctx32[xaddr + yaddr] |= ((*(target) >> yoffset)&1) << xbit;  // make bit white or keep it black
                  //ctx32[xaddr + yaddr] |= ((*((uint32_t*)sprite_image[0]->addr + xoffset) >> yoffset)&1) << xbit;  // make bit white or keep it black
                  }
                } // next y
              } // end display column if in front of wall
            } // next x
//END DRAWING LOOPS      
          } // end display if within y bounds
        } // end display if within x bounds
      } // end display if within farthest
    } // end display if not behind you
  } // next obj

// =======================================END SPRITES====================================================

    */
        graphics_release_frame_buffer(ctx, framebuffer);
  }  // endif successfully captured framebuffer

} // end draw 3D function
#endif