/*

// implement more options
//draw_3D_wireframe?  draw_3D_shaded?
static void draw_3D(GContext *ctx, GRect box) { //, int32_t zoom) {
  int32_t colheight, angle, colh; //, z;

  // Draw Box around view (not needed if fullscreen)
  if(view_border) {graphics_context_set_stroke_color(ctx, 1); graphics_draw_rect(ctx, GRect(box.origin.x-1, box.origin.y-1, box.size.w+2, box.size.h+2));}  //White Rectangle Border

  // Draw background
    // Umm... ok... A nice black background.  Done.  Next?
  
  // White Sky  (Lightning?  Daytime?)
  //graphics_context_set_fill_color(ctx, 1); graphics_fill_rect(ctx, GRect(box.x, box.origin.y, box.size.w, box.size.h/2), 0, GCornerNone);

  for(int16_t col = 0; col < box.size.w; col++) {  // Begin RayTracing Loop
    angle = (fov * (col - (box.size.w>>1))) / box.size.w;

    switch(shoot_ray(player.x, player.y, player.facing + angle)) {  //Shoot rays out of player's eyes.  pew pew.
      case 0:  // 0 means out of map bounds, never hit anything.  Draw horizion dot
        graphics_context_set_stroke_color(ctx, 1);
        graphics_draw_pixel(ctx, GPoint(col + box.origin.x, box.origin.y + (box.size.h/2)));
      break;
      
      case 1:  //1 means hit a block.  Draw the vertical line!
///
          int32_t colbot, coltop; graphics_context_set_stroke_color(ctx, 1);
          colheight = (TRIG_MAX_RATIO*(box.size.h<<6)) /  (2*ray.dist*cos_lookup(angle)); if(colheight>box.size.h/2) colheight=box.size.h/2;
          int32_t colbot = (box.size.h/2) + (colheight / 2); int32_t coltop = (box.size.h/2) - (colheight / 2);
          if(ray.offset<4 || ray.offset > 60) graphics_context_set_stroke_color(ctx, 0); else graphics_context_set_stroke_color(ctx, 1); // comment out line to remove black bars
          graphics_draw_line(ctx, GPoint((int)col + box.origin.x,coltop + box.origin.y), GPoint((int)col + box.origin.x,colbot + box.origin.y));
///

          colheight = (box.size.h << 22) /  (ray.dist * cos_lookup(angle));  // Height of wall segment = box.size.h * wallheight * 64(the "zoom factor") / distance (distance =  ray.dist * cos_lookup(angle))
          if(colheight>box.size.h) colh=box.size.h/2; else colh=colheight/2;   // Make sure line isn't drawn beyond bounding box (also halve it cause of 2 32bit textures)
      
          // Texture the Ray hit, point to 1st half of texture (half, cause a 64x64px texture menas there's 2 uint32_t per row)
          switch(ray.hit) { // Convert this to an array of pointers in the future
            case 1: target = (uint32_t*)wBrick->addr + ray.offset * 2; break;
            case 2: target = (uint32_t*)wFifty->addr + ray.offset * 2; break;
            case 3: target = (uint32_t*)wCircle->addr + ray.offset * 2; break;
          }

          uint32_t x, xaddr, xbit;
          x = col+box.origin.x;  // X screen coordinate
          xaddr = x >> 5;  // X memory address
          xbit = (x & 31); // X bit shift level

          // Note: "+=" addition in lines below only work on a black background (assumes 0 in the bit position).
          for(int32_t i=0; i<colh; i++) {
            //yaddr = ((box.origin.y + (box.size.h/2) -+ i) * 5);   // Y Address = Y screen coordinate * 5
            int32_t ch = (i * ray.dist * cos_lookup(angle)) / (TRIG_MAX_RATIO * box.size.h);
            ((uint32_t*)(((GBitmap*)ctx)->addr))[((box.origin.y + (box.size.h/2) - i) * 5) + xaddr] += (((*target >> (31-ch))&1) << xbit);  // Draw Top Half
            ((uint32_t*)(((GBitmap*)ctx)->addr))[((box.origin.y + (box.size.h/2) + i) * 5) + xaddr] += (((*(target+1)  >> ch)&1) << xbit);   // Draw Bottom Half
          }

      break;
    } // End Switch
  } //End For (End RayTracing Loop)
  
}

*/














/*

          for(int32_t i=0; i<colh; i++) {
            //yaddr = ((view_y + (view_h/2) -+ i) * 5);   // Y Address = Y screen coordinate * 5
            int32_t ch = (i * ray.dist * cos_lookup(angle)) / (TRIG_MAX_RATIO * view_h);
            ((uint32_t*)(((GBitmap*)ctx)->addr))[((view_y + (view_h/2) - i) * 5) + xaddr] += (((*target >> (31-ch))&1) << xbit);  // Draw Top Half
            ((uint32_t*)(((GBitmap*)ctx)->addr))[((view_y + (view_h/2) + i) * 5) + xaddr] += (((*(target+1) >> ch)&1) << xbit); // Draw Bottom Half
          }
          
          
static void graphics_layer_update_proc(Layer *me, GContext *ctx) {
  int32_t colheight, z, angle, colh;
  time_t sec1, sec2; uint16_t ms1, ms2; int32_t dt; // time snapshot variables, to calculate render time and FPS
  time_ms(&sec1, &ms1);  //1st Time Snapshot
  
  //-----------------//
  // Draw 3D World   //
  //-----------------//

  // Draw Box around view (not needed if fullscreen, i.e. view_w=144 and view_h=168 and view_x=view_y=0)
  if(view_border) {graphics_context_set_stroke_color(ctx, 1); graphics_draw_rect(ctx, GRect(view_x-1, view_y-1, view_w+2, view_h+2));}  //White Rectangle Border

  // Draw background
    // Umm... ok... A nice black background.  Done.  Next?
  
  // White Sky  (Lightning?  Daytime?)
  //graphics_context_set_fill_color(ctx, 1); graphics_fill_rect(ctx, GRect(view_x, view_y, view_w, view_h/2), 0, GCornerNone);

  for(int16_t col = 0; col < view_w; col++) {  // Begin RayTracing Loop
    //angle = (fov/view_w) * (col - (view_w/2));  // was: angle = (int32_t)(fov * (((float)col/view_w) - 0.5));
    angle = (fov * (col - (view_w>>1))) / view_w;
    colheight = TRIG_MAX_RATIO * (view_h << 6) /  (ray.dist * cos_lookup(angle));  // Height of wall segment = view_h * wallheight * 64(the "zoom factor") / distance (distance =  ray.dist * cos_lookup(angle))
    // note: above, when view_w becomes variable, this equation should be: angle=(fov * (col - (view_w<<1))) / view_w

    switch(shoot_ray(player.x, player.y, player.facing + angle)) {  //Shoot rays out of player's eyes.  pew pew.
      //case -1:  break; // -1 means too far.  Draw black dot over the pre-drawn horizion line
      case 0:  // 0 means out of map bounds, never hit anything.  Draw horizion dot
        graphics_context_set_stroke_color(ctx, 1);
        graphics_draw_pixel(ctx, GPoint(col + view_x, view_y + (view_h/2)));
      break;
      
      case 1:  //1 means hit a block.  Draw the vertical line!
/ *
        // Calculate amount of shade
        z = (ray.dist * cos_lookup(angle)) / TRIG_MAX_RATIO;  // z = distance
        z -= 64; if(z<0) z=0;   // Make everything 1 block (64px) closer (solid white without having to be nearly touching)
        z = sqrt_int(z,10) >> 1; // z was 0-RANGE(max dist visible), now z = 0 to 12: 0=close 10=distant.  Square Root makes it logarithmic
        z -= 2; if(z<0) z=0;    // Closer still (zWas=zNow: 0-64=0, 65-128=2, 129-192=3, 256=4, 320=6, 384=6, 448=7, 512=8, 576=9, 640=10)
* /
        // Draw Wall Column
        if(DRAWMODE_LINES) { // Begin Simple Lines Drawing
          int32_t coltop, colbot;
          if(colheight>view_h) colheight=view_h;   // Make sure line isn't drawn beyond bounding box (also halve it cause of 2 32bit textures)
            colbot = (view_h/2) + (colheight / 2); coltop = (view_h/2) - (colheight / 2);
          //colbot = (view_h/2) + (colheight / 2); coltop = (view_h/2) - (colheight / 2);     // Normal
          //colbot = (view_h/2) + (colheight*1/8); coltop = (view_h/2) - (colheight*7/8);     // Rodent
		      //colbot = (view_h/2) + (colheight*7/8); coltop = (view_h/2) - (colheight*1/8);     // Flying
          if(ray.offset<4 || ray.offset > 60) graphics_context_set_stroke_color(ctx, 0); else // Black edges on left and right 5% of block (Comment this line to remove edges)
            graphics_context_set_stroke_color(ctx, 1);
          graphics_draw_line(ctx, GPoint((int)col + view_x,coltop + view_y), GPoint((int)col + view_x,colbot + view_y));  //Draw the line
        } // End DrawMode Lines

        if(DRAWMODE_TEXTURES) { // Begin Texture Drawing
          if(colheight>view_h) colh=view_h/2; else colh=colheight/2;   // Make sure line isn't drawn beyond bounding box (also halve it cause of 2 32bit textures)
          
          // Texture the Ray hit, point to 1st half of texture (half, cause a 64x64px texture menas there's 2 uint32_t per row)
          switch(ray.hit) { // Convert this to an array of pointers in the future
            case 1: target = (uint32_t*)wBrick->addr + ray.offset * 2; break;
            case 2: target = (uint32_t*)wFifty->addr + ray.offset * 2; break;
            case 3: target = (uint32_t*)wCircle->addr + ray.offset * 2; break;
          }
          
          graphics_context_set_stroke_color(ctx, 1);
          for(int32_t i=0; i<colh; i++) {
            // Wall is 64 pixels high, but textures use 32bit uints, so have to draw wall in halves
/ *
            // Draw Top Half 
            if( ((180-i) + (col*6)) % 9 >= z)    // Shading (Comment this line out to disable shading of top half)
              if(((*target >> ((31-((i<<6)/colheight))))&1) > 0)  // If the hit pixel on texture is white, draw it
                graphics_draw_pixel(ctx, GPoint(col+view_x, view_y + (view_h/2) - i ));
          
            // Draw Bottom Half
            if( (i + (col*6)) %9 >= z)  // Shading (Comment this line out to disable shading of bottom half)
              if(((*(target+1) >> ((i<<6)/colheight))&1) > 0)
                graphics_draw_pixel(ctx, GPoint(col+view_x, view_y + (view_h/2) +  i));
            
            
            
            // Draw Top Half 
            if( ((180-i) + (col*6)) % 9 < z) graphics_context_set_stroke_color(ctx, 0); else // Shading (Comment this line out to disable shading of top half)
              graphics_context_set_stroke_color(ctx, (*target >> ((31-((i<<6)/colheight))))&1);
            graphics_draw_pixel(ctx, GPoint(col+view_x, view_y + (view_h/2) - i ));
          
            // Draw Bottom Half
            if( (i + (col*6)) %9 < z) graphics_context_set_stroke_color(ctx, 0); else // Shading (Comment this line out to disable shading of bottom half)
              graphics_context_set_stroke_color(ctx, (*(target+1) >> ((i<<6)/colheight))&1);
            graphics_draw_pixel(ctx, GPoint(col+view_x, view_y + (view_h/2) +  i));

* /
            //int32_t ch = ((i<<6) * ray.dist * cos_lookup(angle)) / (TRIG_MAX_RATIO * (view_h << 6));
            int32_t ch = (i * ray.dist * cos_lookup(angle)) / (TRIG_MAX_RATIO * view_h);
            // Draw Top Half
            graphics_context_set_stroke_color(ctx, (*target >> (31- ch))&1);
            graphics_draw_pixel(ctx, GPoint(col+view_x, view_y + (view_h/2) - i ));
          
            // Draw Bottom Half
            graphics_context_set_stroke_color(ctx, (*(target+1) >> ch)&1);
            graphics_draw_pixel(ctx, GPoint(col+view_x, view_y + (view_h/2) +  i));
          }
          
        } // End DrawMode Shaded Textures
      break;
    } // End Switch
  } //End For (End RayTracing Loop)

  time_ms(&sec2, &ms2);  //2nd Time Snapshot
  dt = ((int32_t)1000*(int32_t)sec2 + (int32_t)ms2) - ((int32_t)1000*(int32_t)sec1 + (int32_t)ms1);  //dt=delta time: time between two time snapshots in milliseconds
  
  //-----------------//
  // Display TextBox //
  //-----------------//
  if(draw_textbox) {
    GRect textframe = GRect(0, 0, 143, 20);  // Text Box Position and Size
    graphics_context_set_fill_color(ctx, 0);   graphics_fill_rect(ctx, textframe, 0, GCornerNone);  //Black Solid Rectangle
    graphics_context_set_stroke_color(ctx, 1); graphics_draw_rect(ctx, textframe);                //White Rectangle Border  
    static char text[40];  //Buffer to hold text
    //snprintf(text, sizeof(text), " (%d,%d) %dms %dfps %d", (int)(player.x>>6), (int)(player.y>>6),(int)dt, (int)(1000/dt),(int)getmap(player.x,player.y));  // What text to draw
    snprintf(text, sizeof(text), " (%d,%d) %d", (int)(player.x>>6), (int)(player.y>>6),(int)player.facing);  // What text to draw
    graphics_context_set_text_color(ctx, 1);  // White Text
    graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_14), textframe, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);  //Write Text
  }
  
  //-----------------//
  // Done!
  //-----------------//
  //  Set a timer to restart loop
  
  if(dt<90 && dt>0) // if time to render is less than 100ms, force framerate of 10FPS or worse
     app_timer_register(100-dt, main_loop, NULL); // 10FPS
  else
     app_timer_register(10, main_loop, NULL);     // took longer than 100ms, loop asap (or in 10ms)
  
  // Perhaps clean up variables here?
}
*/













//static void up_click_handler(ClickRecognizerRef recognizer, void *context) { // UP button was pressed
//  view_h -= 2; if(view_h<2) view_h=2; else view_x += 1;
//  view_w -= 2; if(view_w<2) view_w=2; else view_y += 1;
//}

//static void down_click_handler(ClickRecognizerRef recognizer, void *context) { // DOWN button was pressed
//  view_h += 2; if(view_h>180) view_h=180; else view_x -= 1;
//  view_w += 2; if(view_w>150) view_w=150; else view_y -= 1;
//}




/*
  //Definitions:
  //  Object = enemy or whatever
  //  Sprite = object as displayed on screen

  // for object = 0 to numobjects

  int32_t diffx, diffy, objectdist, spritecol, offset;
  diffx=object.x - player.x;
  diffy=object.y - player.y;
  //objectdist=sqrt32(diffx*diffx + diffy*diffy);
  angle = atan2_lookup(diffy, diffx); // angle between player's x,y and sprite's x,y
  objectdist = (diffx>diffy) ? (diffx<<16) / cos_lookup(angle) : (diffy<<16) / sin_lookup(angle);
  
  // if object distance > furthest wall distance, skip drawing object. next in for object loop
  
  //int32_t spritesize = ((32*64)) /  objectdist; // << >> 16
  angle=angle - player.facing;          // angle between center column and sprite
  angle=((angle+TRIG_MAX_ANGLE+32768)%TRIG_MAX_ANGLE)-32768; // convert angle to [-32768 to 32767]
  // Should be able to fix the above line by going to signed int16_t variable.
  //angle now is angle from center column. 0=center of view, -fov/2 is left view edge, fov/2 is right view edge

  spritecol = (box.size.w/2) + (box.size.w * angle / fov);  // convert angle to view column (0 - box.width) (col = middle line of sprite)
  // comment out below line and remove it if it still works
  spritecol = (box.size.w/2) + ((box.size.w/2) * (angle) / (fov/2));  // convert angle to on-screen column (col = middle line of sprite)
  
  objectdist = (objectdist * cos_lookup(angle)) >> 16;

  
  int32_t objectwidth = 32; // 32 pixels wide
  int32_t objectheight = 32; // 32 pixels tall
  int32_t objectverticaloffset = -16; // normally center dot is vertically centered, + or - how far to move it.  
  //int32_t spritescale = 64 /  objectdist; // conversion between object size and size portrayed on screen
  
  int32_t spritewidth = (objectwidth*64) /  objectdist;
  int32_t spriteheight = (objectheight*64) /  objectdist;
  
dist[0] = dist[0]+2;
  
  int16_t sprite_xmin = spritecol-(spritewidth/2);
  int16_t sprite_xmax = spritecol+(spritewidth/2);
  if(sprite_xmax>=0 && sprite_xmin<box.size.w) { // if any of the sprite is horizontally within view
    //int16_t sprite_ymin = (box.size.h/2) - (((objectheight+objectverticaloffset) * 32) / objectdist);  //(box.size.h/2)-(spriteheight/2); // offset should be double since it's halved
    //int16_t sprite_ymax = (box.size.h/2) + (((objectheight-objectverticaloffset) * 32) / objectdist);  //(box.size.h/2)+(spriteheight/2);
    int16_t sprite_ymin = (box.size.h/2) - ((((objectheight/2)+objectverticaloffset) * 64) / objectdist);  //(box.size.h/2)-(spriteheight/2);
    int16_t sprite_ymax = (box.size.h/2) + ((((objectheight/2)-objectverticaloffset) * 64) / objectdist);  //(box.size.h/2)+(spriteheight/2);
    if(sprite_ymax>=0 && sprite_ymin<box.size.h) { // if any of the sprite is vertically within view
      int16_t xmin = sprite_xmin<0 ? 0: sprite_xmin;
      int16_t xmax = sprite_xmax>box.size.w ? box.size.w : sprite_xmax;
      int16_t ymin = sprite_ymin<0 ? 0 : sprite_ymin;
      int16_t ymax = sprite_ymax>box.size.h ? box.size.h : sprite_ymax;
    
      //column = (x*32/spritewidth) // (x*32/(spritewidth-1) ?
      for(int16_t x = xmin; x < xmax; x++) {
        xaddr = (box.origin.x + x) >> 5;
         xbit = (box.origin.x + x) & 31;
      
offset = (((x - sprite_xmin)*objectwidth) / spritewidth);
uint32_t* mask = (uint32_t*)sprite_mask[0]->addr; // target = mask
target = (uint32_t*)sprite_image[0]->addr; // target = sprite
        
        
        for(int16_t y = ymin; y < ymax; y++) {
          yaddr = (box.origin.y + y) * 5;
int32_t ch = (objectheight * (y - sprite_ymin)) / spriteheight;
if(((*(offset + mask) >> ch) & 1) == 1) {
graphics_context_set_stroke_color(ctx, ((*(target+offset) >> ch)&1));
graphics_draw_pixel(ctx, GPoint(box.origin.x + x, box.origin.y + y));
            ctx32[xaddr + yaddr] &= ~(1 << xbit);
//ctx32[xaddr + yaddr] |=  (1 << xbit);
ctx32[xaddr + yaddr] |=  (((*(target+offset) >> ch)&1) << xbit);
}
        } // end y loop
      } // end x loop
    } // end within vertical view
  } // end within horizontal view
  // end object loop
  
*/




/*  
  
  graphics_context_set_stroke_color(ctx, 0);
  for(int16_t col = 0; col < box.size.w; col++) {  // Sprite Overlay Loop
    if(col>=(spritecol-(spritesize/2)) && col<=(spritecol+(spritesize/2))) {

      int16_t ymin = (box.size.h/2)-(spritesize/2); if(ymin<0) ymin = 0;
      int16_t ymax = (box.size.h/2)+(spritesize/2); if(ymax>box.size.h) ymax=box.size.h;
      for(int16_t y = ymin; y < ymax; y++)
        graphics_draw_pixel(ctx, GPoint(box.origin.x + col, box.origin.y + y));
    }
      
  }
*/
      
/*  
  if((col+(spritesize/2))>=0 && (col-(spritesize/2))<box.size.w) { // if any of the sprite is within view
    int16_t colmin = box.origin.x + ((col<=(spritesize/2)) ? 0 : (col-(spritesize/2)));
    int16_t colmax = box.origin.x + (((col+(spritesize/2))>=box.size.w) ? box.size.w : (col+(spritesize/2)));
    int16_t   ymin = box.origin.y + ((box.size.h<=spritesize) ? 0 : ((box.size.h-spritesize)/2));
    int16_t   ymax = box.origin.y + ((spritesize/2)>=(box.size.h/2) ? box.size.h : ((box.size.h+spritesize)/2));
    graphics_context_set_stroke_color(ctx, 1);
    for(int16_t x = colmin; x < colmax; x++)
      for(int16_t y = ymin; y < ymax; y++)
        graphics_draw_pixel(ctx, GPoint(x, y));
    graphics_context_set_stroke_color(ctx, 0);
    for(int16_t x = colmin; x < colmax; x++) {
      graphics_draw_pixel(ctx, GPoint(x, ymin));
      graphics_draw_pixel(ctx, GPoint(x, ymax-1));
    }
    for(int16_t y = ymin; y < ymax; y++) {
        graphics_draw_pixel(ctx, GPoint(colmin, y));
        graphics_draw_pixel(ctx, GPoint(colmax-1, y));
    }
    
  }
  //graphics_fill_rect(ctx, GRect(box.origin.x + col-(spritesize/2),box.origin.y + (box.size.h - spritesize)/2,spritesize,spritesize), 0, GCornerNone);
  */
  
  /*
int32_t diffx, diffy, dist;
  diffx=sprite.x - player.x;
  diffy=sprite.y - player.y;
  dist=sqrt32(diffx*diffx + diffy*diffy);
  Q1=dist;
  
  dist = dist * cos_lookup(angle);           // multiply by cos to stop fisheye lens (should be >>16 to get actual dist, as is done often below)
  
  int32_t spritesize = ((32*64) << 16) /  dist;  // 32 pixel width / (dist>>16)
  if(spritesize>168) spritesize=168;
  Q2 = spritesize;
  angle = atan2_lookup(diffy, diffx);
  Q3=angle;
  //angle = angle - player.facing;
  int32_t col = (box.size.w/2) + ((box.size.w/2) * angle / (fov/2));
  Q4=col;
  graphics_context_set_stroke_color(ctx, GColorWhite);
  if(col>0 && col<144)
    graphics_draw_line(ctx, GPoint(col,(box.size.h/2)-spritesize), GPoint(col,(box.size.h/2)+spritesize));
  //angle = (fov * (col - (box.size.w>>1))) / box.size.w;
    
   // x = col+box.origin.x;  // X screen coordinate
   // xaddr = x >> 5;        // X memory address (for which 32bit screen memory word)
   // xbit = x & 31;        // X bit-shift amount (for which bit within screen memory 32bit word)
   // dist[col] = ray.dist;  
   */
