/*! 
 * Copyright (c) 2025 David A. Newman (a.k.a. 0dan0)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
 #include <stdint.h>

#define BUTTON_UP    0x1
#define BUTTON_DOWN  0x2
#define BUTTON_LEFT  0x4
#define BUTTON_RIGHT 0x8
#define BUTTON_BACK  0x20
#define BUTTON_NEG   0x100
#define BUTTON_PLUS  0x200
#define BUTTON_OK    0x800

#define NVM_WBAL        0
#define NVM_SHARPEN     1
#define NVM_SAT         2
#define NVM_FREE        3
#define NVM_DONOT_USE   4
#define NVM_WB_MODS     5
#define NVM_EVBIAS      6
#define NVM_FPS         7
#define NVM_QPMIN       8
#define NVM_ISOMAX      9
#define NVM_EXPLOCK     10

#define NVM_NAV         13

#define NVM_ISO_LOCK    14
#define NVM_SHUT_LOCK   15

#define NVM_SAVE_WBAL    16
#define NVM_SAVE_SHARPEN 17
#define NVM_SAVE_SAT     18

#define WIDTH 656
#define PITCH 656
#if DRAW_RGB
#define TEXT_WIDTH (36*4)
#else
#define TEXT_WIDTH (0)
#endif
#define HIST_WIDTH (128+8)
#define HIST_PITCH (HIST_WIDTH+TEXT_WIDTH)
#define HIST_HEIGHT (64+8)
#define HEIGHT 480
#define NUM_BINS 128
#define EDGE 48
#define EDGE_X1 190 // was 160, need more room for 8mm (v6.8).
#define EDGE_X2 32


#define LCD_X 480
#define LCD_Y 864
#define LCD_P 480

// 8-bit palettle mapped colors
enum Palette {
  TRANSPARENT=0,
  YELLOW,
  PURPLE,
  RED,
  CYAN,
  GREEN,
  BLUE,
  BLACK,  //7 
  GREY25,
  GREY50,
  GREY80,
  GREY90,
  GREY35,
  GREY190,
  GREY200,
  GREY205, //15
  GREY170,
  GREY215,
  GREY130,
  GREY70,
  GREY55,
  GREY185,
  GREY195,
  GREY26,
  GREY15,
  GREY60,
  GREY85,
  GREY95,
  GREY180,
  GREY30,
  GREY10,
  GRADIENT_41=214,
  GRADIENT_40,
  GRADIENT_39,
  GRADIENT_38,
  GRADIENT_37,
  GRADIENT_36,
  GRADIENT_35,
  GRADIENT_34,
  GRADIENT_33,
  GRADIENT_32,
  GRADIENT_31,
  GRADIENT_30,
  GRADIENT_29,
  GRADIENT_28,
  GRADIENT_27,
  GRADIENT_26,
  GRADIENT_25,
  GRADIENT_24,
  GRADIENT_23,
  GRADIENT_22,
  GRADIENT_21,
  GRADIENT_20,
  GRADIENT_19,
  GRADIENT_18,
  GRADIENT_17,
  GRADIENT_16,
  GRADIENT_15,
  GRADIENT_14,
  GRADIENT_13,
  GRADIENT_12,
  GRADIENT_11,
  GRADIENT_10,
  GRADIENT_9,
  GRADIENT_8,
  GRADIENT_7,
  GRADIENT_6,
  GRADIENT_5,
  GRADIENT_4,
  GRADIENT_3,
  GRADIENT_2,
  GRADIENT_1,
  GRADIENT_0,
  WHITE=255
};

#define DRAW        1

#define FONT_BASE_ADDR   ((uintptr_t)0x8033A800u)
#define FONT7x12_ROM     ((const uint8_t (*)[12])FONT_BASE_ADDR)

#define FW  8
#define FH  12

/* buf: uint8_t* grayscale; stride: width in pixels; x,y: top-left */
#define _I(stride,x,y)          ((y)*(stride)+(x))
#define _P(buf,stride,w,h,x,y,v)                                             \
do{ int _X=(x),_Y=(y);                                                       \
    if((unsigned)_X<(unsigned)(w)&&(unsigned)_Y<(unsigned)(h))               \
        (buf)[_I((stride),_X,_Y)]=(uint8_t)(v);                              \
}while(0)

/* 7x11 font, MSB=leftmost (use bits 6..0). Undefined chars render blank. */
/* Draw one glyph at (x,y); v = intensity 0..255. MSB=leftmost */
#define DRAW_GLYPH(buf,stride,w,h,x,y,ch,v)                                        \
do{ unsigned char _c=(unsigned char)(ch); int _r,_c0;                              \
    for(_r=0;_r<FH;++_r){                                                          \
        uint8_t _bits=FONT7x12_ROM[_c][_r];                                        \
        for(_c0=0;_c0<FW;++_c0) {                                                  \
          _P((buf),(stride),(w),(h),(x)+_c0,(y)+_r,(_bits&(1u<<((FW-1)-_c0))?v:0));\
        }                                                                          \
    }                                                                              \
}while(0)

/* Draw ASCII string (single line). Advance = (FW). */
#define DRAW_TEXT(buf,stride,w,h,x,y,str,v)                                      \
do{ const char*_p=(const char*)(str); int _cx=(x);                               \
    for(;*_p;++_p){ DRAW_GLYPH((buf),(stride),(w),(h),_cx,(y),*_p,(v)); _cx+=FW-1; }\
}while(0)






#define _I_V(h,stride,x,y)          (((h)-x)*(stride)+(y))
#define _P_V(buf,stride,w,h,x,y,v)                                             \
do{ int _X=(x),_Y=(y);                                                        \
   (buf)[_I_V((h),(stride),_X,_Y)]=(uint8_t)(v);                               \
}while(0)

/* 7x11 font, MSB=leftmost (use bits 6..0). Undefined chars render blank. */
/* Draw one glyph at (x,y); v = intensity 0..255. MSB=leftmost */
#define DRAW_GLYPH_V(buf,stride,w,h,x,y,ch,v)                                        \
do{ unsigned char _c=(unsigned char)(ch); int _r,_c0;                              \
    for(_r=0;_r<FH;++_r){                                                          \
        uint8_t _bits=FONT7x12_ROM[_c][_r];                                        \
        for(_c0=0;_c0<FW;++_c0) {                                                  \
          _P_V((buf),(stride),(w),(h),(x)+_c0,(y)+_r,(_bits&(1u<<((FW-1)-_c0))?v:7));\
        }                                                                          \
    }                                                                              \
}while(0)

/* Draw ASCII string (single line). Advance = (FW). */
#define DRAW_TEXT_V(buf,stride,w,h,x,y,str,v)                                      \
if(buf[0] == 7) { const char*_p=(const char*)(str); int _cx=(x), _cy=(y), _cv=(v); \
    for(;*_p;++_p){ DRAW_GLYPH_V((buf),(stride),(w),(h),_cx,_cy,*_p,_cv); \
    _cx+=FW-1; if(_p[1] == 10) { ++_p; _cy+=FH+1; _cx=(x); } \
    else if(_p[1]>0x0 && _p[1]<0x20) _cv = _p[1]; \
    }\
}

#define DRAW_TEXT_V_ALWAYS(buf,stride,w,h,x,y,str,v)                                      \
{ const char*_p=(const char*)(str); int _cx=(x), _cy=(y);                               \
  for(;*_p;++_p){ DRAW_GLYPH_V((buf),(stride),(w),(h),_cx,(y),*_p,(v)); _cx+=FW-1; } }


// Integer square root for 32-bit unsigned values
#define ISQRT(n, res)                           \
do {                                            \
    unsigned int __op = (n);                    \
    unsigned int __res = 0;                     \
    unsigned int __one = 1u << 30;              \
    while (__one > __op) __one >>= 2;           \
    while (__one != 0) {                        \
        if (__op >= __res + __one) {            \
            __op -= __res + __one;              \
            __res = __res + 2 * __one;          \
        }                                       \
        __res >>= 1;                            \
        __one >>= 2;                            \
    }                                           \
    (res) = __res;                              \
} while(0)



void calc_histogram(void)
{
	asm volatile (
		".word 0x202d2d2d\n" //--- 
		".word 0x20545543\n" //CUT 
		".word 0x45524548\n" //HERE
		".word 0x2d2d2d20\n" //--- 
	);

	int* frameno = (int *)0x80f8214c; //frame counter
	uint16_t *histogram_stats = (uint16_t *)  0x85bf0100; // was 85bf0100
	uint8_t  *histo_rgb_image = 0;// = (uint8_t *) (0x85bf0000 - (HIST_PITCH * HIST_HEIGHT * 3)); // was 85bf0500  (seems to effect the encoder buffer.)
	uint32_t *expo_change = (uint32_t *)0x85bf0010;
	uint32_t *enc_frames = (uint32_t *) 0x85bf0014;
	//uint32_t *count_frames = (uint32_t *)0x85bf0018;
	uint32_t *current_Qp = (uint32_t *) 0xa56f1f60;
    uint32_t *wb_gains = (uint32_t *)0x85bf0020;
    uint32_t *window_res = (uint32_t *)0x85bf0030;
    uint8_t *LCD = (uint8_t *)0x81821180; // All types
    
	uint8_t *imagebase = (uint8_t *)0xa2730b70; //start of LRV  //PREVIEW
  /*uint8_t *image = (uint8_t *)0x827c8970; //start of LRV
	uint8_t *image = (uint8_t *)0x82860770; //start of LRV
	uint8_t *image = (uint8_t *)0x828f8570; //start of LRV
	uint8_t *image = (uint8_t *)0x82990370; //start of LRV
	uint8_t *image = (uint8_t *)0x82a28170; //start of LRV
    */
    
  /*uint8_t *image = (uint8_t *)0x837AB770; //start of LRV  //Encode
	uint8_t *image = (uint8_t *)0x83843570; //start of LRV
	uint8_t *image = (uint8_t *)0x838DB370; //start of LRV
	uint8_t *image = (uint8_t *)0x83973170; //start of LRV
	uint8_t *image = (uint8_t *)0x83A0AF70; //start of LRV
	uint8_t *image = (uint8_t *)0x83AA2D70; //start of LRV
    */
    
	//int* encoded_frames = (int *)0x85bf0000;
	//if(*encoded_frames < 2 || *encoded_frames > 16384)
	//	return;

	if(*frameno < 25 || *frameno & 0xfff00000) 
		return;  // time to initialize

#if 0 // Draw color palette
    enum { GRID = 16, CELL = 30 };
    if(*frameno < 26)
    {
        //int x,y;
        //for(x=0; x<480; x++)
        //{
        //    for(y=0; y<256*2; y++)
        //    {
        //        LCD[y*480 + x + 64] = (y/8) & 0xff;
        //    }
        //}
        for (int gy = 0; gy < GRID; ++gy) {
            int y0 = gy * CELL;
        
            for (int py = 0; py < CELL; ++py) {
                uint8_t *row = LCD + (y0 + py) * 480;
        
                for (int gx = 0; gx < GRID; ++gx) {
                    uint8_t c = gy*16+gx;
                    int x0 = gx * CELL;
        
                    // Fill this 30-pixel run
                    uint8_t *p = row + x0;
                    for (int i = 0; i < CELL; ++i) {
                        p[i] = c;
                    }
                }
            }
        }
    }
#endif


#if 0 // Test vertical text rendering to the overlay    
    if(*frameno < 10000)
    {   
        char newtxt[20];        
        newtxt[0] = 'H';
        newtxt[1] = 'e';
        newtxt[2] = 'l';
        newtxt[3] = 'l';
        newtxt[4] = 'o';
        newtxt[5] = ' ';
        newtxt[6] = ( *frameno / 10000) + '0';
        newtxt[7] = ((*frameno / 1000) % 10) + '0';
        newtxt[8] = ((*frameno / 100) % 10) + '0';
        newtxt[9] = ((*frameno / 10) % 10) + '0';
        newtxt[10] = (*frameno % 10) + '0';
        newtxt[11] = 0;
        
        DRAW_TEXT_V(LCD, 480, 480, 864, 8, 8, newtxt, 1);
    }
#endif

    if(*enc_frames > 0 && *enc_frames < 99999)
    {
        imagebase = (uint8_t *)0xa37AB770; //start of LRV //0xAxxxxxxx - uncached
    }
    
    if(!(*expo_change == 0xffff0000 || *enc_frames > 0))
       return; // only show histogram in preview or once encoding 
    
    uint32_t *hist[5];
    
    hist[0] = (uint32_t *)0x86000000;
    hist[1] = (uint32_t *)0x86300000; 
    hist[2] = (uint32_t *)0x86600000; 
    hist[3] = (uint32_t *)0x86900000; 
    hist[4] = (uint32_t *)0x86c00000; 
    //hist[0] = (uint32_t *)0x87600000;  // doesn't hurt, doesn't help
    //hist[1] = (uint32_t *)0x87800000; 
    //hist[2] = (uint32_t *)0x87a00000; 
    //hist[3] = (uint32_t *)0x87c00000; 
    //hist[4] = (uint32_t *)0x87e00000; 
    
    if(*hist[0] != 0x12345678 && *hist[1] != 0x12345678 && *hist[2] != 0x12345678 && *hist[3] != 0x12345678 && *hist[4] != 0x12345678)
    {
        *hist[0] = 0x12345678;
        *hist[1] = 0x12345678;
        *hist[2] = 0x12345678;
        *hist[3] = 0x12345678;
        *hist[4] = 0x12345678;
    }
    
    if(*hist[0] != 0x12345678)
    {
        histo_rgb_image = (uint8_t *)hist[3];
        histo_rgb_image += 4;
        *hist[3] = 0x12345678; // reset first
    }    
    if(*hist[1] != 0x12345678)
    {
        histo_rgb_image = (uint8_t *)hist[4];
        histo_rgb_image += 4;
        *hist[4] = 0x12345678; // reset first
    }    
    if(*hist[2] != 0x12345678)
    {
        histo_rgb_image = (uint8_t *)hist[0];
        histo_rgb_image += 4;
        *hist[0] = 0x12345678; // reset first
    }    
    if(*hist[3] != 0x12345678)
    {
        histo_rgb_image = (uint8_t *)hist[1];
        histo_rgb_image += 4;
        *hist[1] = 0x12345678; // reset first
    }    
    if(*hist[4] != 0x12345678)
    {
        histo_rgb_image = (uint8_t *)hist[2];
        histo_rgb_image += 4;
        *hist[2] = 0x12345678; // reset first
    }
    
    if(histo_rgb_image == 0)
    {
        histo_rgb_image = (uint8_t *)hist[3];
    }
    
    //histogram_stats = (uint16_t *)histo_rgb_image;
    //histogram_stats -= 0x1000;
    
	volatile int* reelType = (int *)0x80340000;
    volatile uint32_t* button = (uint32_t *)0xA0E8BFF8; // uncached
    int32_t* nvm_base = (uint32_t *)0x80E0B78C; //Type A - exposure, sharpness, tint
	int* expo_iso = (int *)0x80e56134; //Type A - sensor ISO 
       
	if(*reelType == 2)
    {
        expo_iso = (int *)0x80e56224; //sensor ISO
        nvm_base = (int *)0x80E0B87C; //exposure, sharpness, tint    
        button = (uint32_t *)0xA0E8C0E8;
    }
	if(*reelType == 3)
    {
		expo_iso = (int *)0x80e556b4; //sensor ISO
        nvm_base = (int *)0x80E0AD0C; //exposure, sharpness, tint
        button = (uint32_t *)0xA0E8B578;
    }
	int* expo_time = expo_iso + 1;
    
    if(button[3] > 0 && button[0] == BUTTON_OK) 
        return;  // don't do anything with OK pressed.
    
	for (int i = 0; i < NUM_BINS*4; i++) 
		histogram_stats[i] = 0;
	
	uint32_t *pixels = (uint32_t *)imagebase; // first pixels
	int j,current_frame = 0;
	for(j=0; j<6; j++)
	{
		if(*pixels != 0) current_frame = j;
		*pixels = 0;
		
		pixels += 0x97e00>>2;  //next frame in the six 
	}
    
    uint8_t *image = imagebase;
    image += 0x97e00 * current_frame;
    uint8_t* chroma = image + WIDTH*HEIGHT + 0x18600;
  
    int pixel_counted = 0;
    // Compute histogram
	for (int y = EDGE; y < HEIGHT-EDGE; y+=4) {
		for (int x = EDGE_X1; x < WIDTH-EDGE_X2; x+=4) {
            int yy,u,v,r,g,b;
            
            yy = image[y*PITCH+x];
                        
            u = chroma[(y>>1)*PITCH+(x&0xfffe)] - 128;
            v = chroma[(y>>1)*PITCH+(x&0xfffe)+1] - 128;
            r = yy + (1616 * v >> 10);
            g = yy - (192  * u >> 10) - (479 * v >> 10);
            b = yy + (1899 * u >> 10);
            
            if(r<0) r=0; if(r>255) r=255;
            if(g<0) g=0; if(g>255) g=255;
            if(b<0) b=0; if(b>255) b=255;
            
            yy -= nvm_base[NVM_EVBIAS]*10;
            if(yy<0) yy = 0;
            if(yy>255) yy=255;
            
            yy >>= 1; //0 to 127 range
            r >>= 1;  //0 to 127 range
            g >>= 1;  //0 to 127 range
            b >>= 1;  //0 to 127 range
            
			histogram_stats[yy]++;
			histogram_stats[128+r]++;
			histogram_stats[256+g]++;
			histogram_stats[384+b]++;
            
            pixel_counted++;
		}
	}

#if DRAW
//if(*enc_frames >= 200)
//{
    uint8_t *planes = histo_rgb_image;
    uint32_t *planes32 = (uint32_t *)histo_rgb_image;
    //if(planes[0] != 63)
    {  
        for(int rgb=0; rgb<3; rgb++)
        {
	        for (int y = 0; y < HIST_HEIGHT; y++) {
	           planes[y * HIST_PITCH + 0] = 63;                    // Left border
	           planes[y * HIST_PITCH + 1] = 63;                    // Left border
	           planes[y * HIST_PITCH + 2] = 1;                     // Left border
	           planes[y * HIST_PITCH + 3] = 1;                     // Left border
	           planes[y * HIST_PITCH + HIST_WIDTH - 1] = 63;       // Right border
	           planes[y * HIST_PITCH + HIST_WIDTH - 2] = 63;       // Right border
	           planes[y * HIST_PITCH + HIST_WIDTH - 3] = 1;        // Right border
	           planes[y * HIST_PITCH + HIST_WIDTH - 4] = 1;        // Right border
	        }
	        for (int x = 1; x < HIST_WIDTH-1; x++) {
	           planes[x] = 63;                                      // Top border
	           planes[x+HIST_PITCH] = 63;                           // Top border
	           planes[x+HIST_PITCH*2] = 1;                          // Top border
	           planes[x+HIST_PITCH*3] = 1;                          // Top border
	           planes[(HIST_HEIGHT - 1) * HIST_PITCH + x] = 63;     // Bottom border
	           planes[(HIST_HEIGHT - 2) * HIST_PITCH + x] = 63;     // Bottom border
	           planes[(HIST_HEIGHT - 3) * HIST_PITCH + x] = 1;      // Bottom border
	           planes[(HIST_HEIGHT - 4) * HIST_PITCH + x] = 1;      // Bottom border
	        }
           
           #if DRAW_RGB
            for (int y = 0; y < HIST_HEIGHT; y++) {
                int off = (y * HIST_PITCH + HIST_WIDTH)>>2;
                for(int x=0; x<(TEXT_WIDTH/4); x++) {
	                planes32[off+x] = 0x01010101;
                }
            }
           #endif
            
            planes += HIST_PITCH * HIST_HEIGHT;
            planes32 += (HIST_PITCH * HIST_HEIGHT)>>2;
        }
    }

    char *text;
    
    int power = 2*nvm_base[NVM_ISOMAX]; //0,2,4
    if(power == 0) power = 1;        
    if(power > 4) power = 4;        
    if(*enc_frames > 0)
    {
//Frm:           
//WB gains:      
// 432,256,256  
//ev : +0       
//FPS: 18        
//Qp : 25 / 27  
//ISO: 400        
//Exp:xxxxus   
        char *formattedTextEnc = (char *)0x8033b500;
        text = formattedTextEnc;
    
        // Frame number
        text[4] = ( *enc_frames / 10000) + '0';
        text[5] = ((*enc_frames / 1000) % 10) + '0';
        text[6] = ((*enc_frames / 100) % 10) + '0';
        text[7] = ((*enc_frames / 10) % 10) + '0';
        text[8] = (*enc_frames % 10) + '0';
        if(text[4] == '0')
        {
            text[4] = ' ';
            if(text[5] == '0')
            {
                text[5] = ' ';
                if(text[6] == '0')
                {
                    text[6] = ' ';
                    if(text[7] == '0')
                    {
                        text[7] = ' ';
                    }
                }
            }
        }
        //WB values
        text[2*16+0] = ' ';
        text[2*16+1] = (wb_gains[0] / 100) + '0';
        text[2*16+2] = ((wb_gains[0] / 10) % 10) + '0';
        text[2*16+3] = (wb_gains[0] % 10) + '0';
        text[2*16+4] = ',';             
        text[2*16+5] = (wb_gains[1] / 100) + '0';
        text[2*16+6] = ((wb_gains[1] / 10) % 10) + '0';
        text[2*16+7] = (wb_gains[1] % 10) + '0';
        text[2*16+8] = ',';             
        text[2*16+9] = (wb_gains[2] / 100) + '0';
        text[2*16+10] = ((wb_gains[2] / 10) % 10) + '0';
        text[2*16+11] = (wb_gains[2] % 10) + '0';
        text[2*16+12] = ' ';        
    
        //EV Bias
        text[3*16+4] = ' '; 
        if(nvm_base[NVM_EVBIAS] < 0) 
        {   
            text[3*16+5] = '-'; 
            text[3*16+6] = -nvm_base[NVM_EVBIAS] + '0';
        }
        else 
        {
            text[3*16+5] = '+';
            text[3*16+6] = nvm_base[NVM_EVBIAS] + '0';
        }
        text[3*16+7] = ' '; 
                
        //FPS        
        text[4*16+4] = ' ';
        if(nvm_base[NVM_FPS] == 0)
        {
            text[4*16+5] = '1';
            text[4*16+6] = '6';
        }    
        if(nvm_base[NVM_FPS] == 1)
        {
            text[4*16+5] = '1';
            text[4*16+6] = '8';
        }
        if(nvm_base[NVM_FPS] == 2)
        {
            text[4*16+5] = '2';
            text[4*16+6] = '4';
        }
        text[4*16+7] = ' ';
        
        //QP : 25/27  
        if(current_Qp[0] > 30) current_Qp[0]=30;
        if(current_Qp[0] < 16) current_Qp[0]=16;
        text[5*16+5] = ((current_Qp[0] / 10) % 10) + '0';
        text[5*16+6] = (current_Qp[0] % 10) + '0';
        text[5*16+7] = '/';
        text[5*16+8] = (((nvm_base[NVM_QPMIN]-1) / 10) % 10) + '0';
        text[5*16+9] = ((nvm_base[NVM_QPMIN]-1) % 10) + '0';
        text[5*16+10] = ' ';
       
        if(nvm_base[NVM_QPMIN]-1 > current_Qp[0])
            current_Qp[0] = nvm_base[NVM_QPMIN]-1;
    
        //ISO: 400/400   
        if(expo_iso[0] == 50)
        {
            text[6*16+5] = ' ';
            text[6*16+6] = '5';
        }
        else
        {
            text[6*16+5] = (expo_iso[0] / 100) + '0';
            text[6*16+6] = '0';
        }
        text[6*16+8] = '/';
        text[6*16+9] = power + '0';
        text[6*16+12] = ' ';
    
        //Exp:xxxxus [L]
        text[7*16+4] = (expo_time[0] / 1000) + '0';
        text[7*16+5] = ((expo_time[0] / 100) % 10) + '0';
        text[7*16+6] = ((expo_time[0] / 10) % 10) + '0';
        text[7*16+7] = (expo_time[0] % 10) + '0';
        text[7*16+10] = ' ';
        text[7*16+11] = ((nvm_base[NVM_EXPLOCK] & 1) ? 'L' : 'A');    
        text[7*16+12] = ' ';
    
        if(nvm_base[NVM_NAV]==0)  //WB R
        {
            text[2*16+0] = '[';
            text[2*16+4] = ']';
        }
        if(nvm_base[NVM_NAV]==1) //WB G
        {
            text[2*16+4] = '[';
            text[2*16+8] = ']';
        }
        if(nvm_base[NVM_NAV]==2) //WB B
        {
            text[2*16+8] = '[';
            text[2*16+12] = ']';
        }           
        if(nvm_base[NVM_NAV]==3) //EV Bias
        {
            text[3*16+4] = '[';
            text[3*16+7] = ']';
        }
        if(nvm_base[NVM_NAV]==4) // FPS
        {
            text[4*16+4] = '[';
            text[4*16+7] = ']';
        }
        if(nvm_base[NVM_NAV]==5) //QP
        {
            text[5*16+7] = '[';
            text[5*16+10] = ']';
        }
        if(nvm_base[NVM_NAV]==6) // ISO MAX
        {
            text[6*16+8] = '[';
            text[6*16+12] = ']';
        }
        if(nvm_base[NVM_NAV]==7) // Lock/Auto
        {
            text[7*16+10] = '[';
            text[7*16+12] = ']';
        }
    }
    else
    {    
//                   
//WB gains:      
// 432,256,256  
//Res:1440x1080  
//off:512,296    
//               
//ISO: 400/400        
//Exp:xxxxus     
        char *formattedTextPrev = (char *)0x8033b600;
        text = formattedTextPrev;
        int pos = 4;
        
        //WB values
        text[2*16+0] = ' ';
        text[2*16+1] = (wb_gains[0] / 100) + '0';
        text[2*16+2] = ((wb_gains[0] / 10) % 10) + '0';
        text[2*16+3] = (wb_gains[0] % 10) + '0';
        text[2*16+4] = ',';             
        text[2*16+5] = (wb_gains[1] / 100) + '0';
        text[2*16+6] = ((wb_gains[1] / 10) % 10) + '0';
        text[2*16+7] = (wb_gains[1] % 10) + '0';
        text[2*16+8] = ',';             
        text[2*16+9] = (wb_gains[2] / 100) + '0';
        text[2*16+10] = ((wb_gains[2] / 10) % 10) + '0';
        text[2*16+11] = (wb_gains[2] % 10) + '0';
        text[2*16+12] = ' ';
        
        //Res:1440x1080 
        text[3*16+pos++] = ((window_res[0] / 1000) % 10) + '0';
        if(text[3*16+pos-1] == '0') pos--;  
        text[3*16+pos++] = ((window_res[0] / 100) % 10) + '0';
        text[3*16+pos++] = ((window_res[0] / 10) % 10) + '0';
        text[3*16+pos++] =  (window_res[0] % 10) + '0';
        text[3*16+pos++] = 'x';
        text[3*16+pos++]  = ((window_res[1] / 1000) % 10) + '0';
        if(text[3*16+pos-1] == '0') pos--;  
        text[3*16+pos++] = ((window_res[1] / 100) % 10) + '0';
        text[3*16+pos++] = ((window_res[1] / 10) % 10) + '0';
        text[3*16+pos++] =  (window_res[1] % 10) + '0';
        text[3*16+pos++] = ' ';
        
        //off:512,296  
        pos = 4;
        text[4*16+pos++] = ((window_res[2] / 1000) % 10) + '0';
        if(text[4*16+pos-1] == '0') pos--;  
        text[4*16+pos++] = ((window_res[2] / 100) % 10) + '0';
        text[4*16+pos++] = ((window_res[2] / 10) % 10) + '0';
        text[4*16+pos++] =  (window_res[2] % 10) + '0';
        text[4*16+pos++] = ',';
        text[4*16+pos++]  = ((window_res[3] / 1000) % 10) + '0';
        if(text[4*16+pos-1] == '0') pos--;  
        text[4*16+pos++] = ((window_res[3] / 100) % 10) + '0';
        text[4*16+pos++] = ((window_res[3] / 10) % 10) + '0';
        text[4*16+pos++] =  (window_res[3] % 10) + '0';
        text[4*16+pos++] = ' ';        
        
        //ISO: 400   
        if(expo_iso[0] == 50)
        {
            text[6*16+5] = ' ';
            text[6*16+6] = '5';
        }
        else
        {
            text[6*16+5] = (expo_iso[0] / 100) + '0';
            text[6*16+6] = '0';
        }
    
        //Exp:xxxxus
        text[7*16+4] = (expo_time[0] / 1000) + '0';
        text[7*16+5] = ((expo_time[0] / 100) % 10) + '0';
        text[7*16+6] = ((expo_time[0] / 10) % 10) + '0';
        text[7*16+7] = (expo_time[0] % 10) + '0';
    }
    
    DRAW_TEXT_V(LCD, LCD_P, LCD_X, LCD_Y, 764, 360, text, GREY215);
 


	uint32_t peak = 0;
	for (uint32_t x = 0; x < NUM_BINS; x++) {
		uint32_t value = histogram_stats[x];
		if(peak < value) peak = value;
	}
	uint32_t val = (uint32_t)peak*12;
    uint32_t y_sqrt_peak;
    ISQRT(val, y_sqrt_peak);
    
	// draw histogram in memory
	for (int x = 0; x < 128; x++) {
		uint32_t rval =  (uint32_t)(histogram_stats[128 + x])<<15;
		uint32_t gval =  (uint32_t)(histogram_stats[256 + x])<<15;
		uint32_t bval =  (uint32_t)(histogram_stats[384 + x])<<15;
		
		// integer square root code
        uint32_t r_sqrt, g_sqrt, b_sqrt;
        ISQRT(rval, r_sqrt);
        ISQRT(gval, g_sqrt);
        ISQRT(bval, b_sqrt);
    
		int y;
        int rvalue = 64 - r_sqrt/y_sqrt_peak;  // Get histogram value
        int gvalue = 64 - g_sqrt/y_sqrt_peak;  // Get histogram value
        int bvalue = 64 - b_sqrt/y_sqrt_peak;  // Get histogram value
        if(rvalue < 0) rvalue = 0;
        if(gvalue < 0) gvalue = 0;
        if(bvalue < 0) bvalue = 0;
        
		for (y = 63; y > rvalue; y--)
			histo_rgb_image[(y+4) * HIST_PITCH + (4 + x)] = 127;
		for (; y >= 0; y--)
			histo_rgb_image[(y+4) * HIST_PITCH + (4 + x)] = 1;

		for (y = 63; y > gvalue; y--)
			histo_rgb_image[(HIST_PITCH * HIST_HEIGHT) + (y+4) * HIST_PITCH + (4 + x)] = 127;
		for (; y >= 0; y--)
			histo_rgb_image[(HIST_PITCH * HIST_HEIGHT) + (y+4) * HIST_PITCH + (4 + x)] = 1;

		for (y = 63; y > bvalue; y--)
			histo_rgb_image[(HIST_PITCH * HIST_HEIGHT)*2 + (y+4) * HIST_PITCH + (4 + x)] = 127;
		for (; y >= 0; y--)
			histo_rgb_image[(HIST_PITCH * HIST_HEIGHT)*2 + (y+4) * HIST_PITCH + (4 + x)] = 1;
	}
    
    
    // draw pre-rendered histo_rgb_image into the frame buffer
    {
        image = imagebase;
	    image += 0x97e00 * current_frame; // seems to be a 6 frame buffer during preview
        
        //current_frame++;
        //current_frame &= 6;
        
        uint8_t* ybuff = image;
        uint8_t* uvbuff = image + WIDTH*HEIGHT + 0x18600;
	    
        ybuff += 380 * PITCH + 16;
        uvbuff += (380/2) * PITCH + 16;
        
	    // draw histogram to frame
	    for (int y = 0; y < HIST_HEIGHT; y++) {
		    uint32_t x;
            uint8_t *src, *dsty, *dstc, pixels;
		    src = (uint8_t *)&histo_rgb_image[y * HIST_PITCH];
		    dsty = (uint8_t *)&ybuff[y * PITCH];
		    dstc = (uint8_t *)&uvbuff[(y>>1) * PITCH];
		    for (x = 0; x < HIST_PITCH; x+=2) {
                int y1 = dsty[x];
                int y2 = dsty[x+1];
                int u = dstc[x] - 128;
                int v = dstc[x+1] - 128;
                
                int histr1 = src[x];
                int histr2 = src[x+1];
                int histg1 = src[(HIST_PITCH * HIST_HEIGHT)+x];
                int histg2 = src[(HIST_PITCH * HIST_HEIGHT)+x+1];
                int histb1 = src[(HIST_PITCH * HIST_HEIGHT)*2+x];
                int histb2 = src[(HIST_PITCH * HIST_HEIGHT)*2+x+1];
                
                int hy1 = (218 * histr1 + 732 * histg1 +  74 * histb1 + 512)>>10;
                int hy2 = (218 * histr2 + 732 * histg2 +  74 * histb2 + 512)>>10;
                int hu1 = (-117 * histr1 - 395 * histg1 + 512 * histb1 + 512)>>10;
                int hu2 = (-117 * histr2 - 395 * histg2 + 512 * histb2 + 512)>>10;
                int hv1 = (512 * histr1 - 465 * histg1 - 47 * histb1 + 512)>>10;
                int hv2 = (512 * histr2 - 465 * histg2 - 47 * histb2 + 512)>>10;
    
			    dsty[x] = (hy1 + y1)>>1;
			    dsty[x+1] = (hy2 + y2)>>1;
                dstc[x] = ((u + u + hu1 + hu2)>>2) + 128;
                dstc[x+1] = ((v + v + hv1 + hv2)>>2) + 128;
		    }
	    }
    }
//}
#endif

#if 1
	{        
		if(*expo_iso < 50 || *expo_time < 500 || *expo_time > 16386) // initialize
		{
            if((nvm_base[NVM_EXPLOCK] & 1) == 0)
            {
                *expo_iso = 50;//100//200;//50;
                *expo_time = 2047;//1023;//4095;
            } 
            else
            {
                *expo_iso = nvm_base[NVM_ISO_LOCK];
                *expo_time = nvm_base[NVM_SHUT_LOCK];
            }
        }
        
        if(nvm_base[NVM_EXPLOCK] & 1 && *expo_time > 1)
        {
            if(nvm_base[NVM_ISO_LOCK] != *expo_iso)
                nvm_base[NVM_ISO_LOCK] = *expo_iso;
            if(nvm_base[NVM_SHUT_LOCK] != *expo_time)
                nvm_base[NVM_SHUT_LOCK] = *expo_time;
        }
		
		if(*expo_iso > 0 && (nvm_base[NVM_EXPLOCK] & 1) == 0) // Manual Exposure
		{
			int currexpo = *expo_time * (*expo_iso / 50); 
			int newexpo = currexpo;
			int nextexpo = currexpo;
			
			int total = pixel_counted; // total pixels sampled.  
			int top_stops = 0;
            int twothirds = 0;
			int midA_stops = 0;
			int midB_stops = 0;
			int midC_stops = 0;
			int midD_stops = 0;
			int bot_stops = 0;
			int clipped = 0;
             
			int i=0;
			for(; i<42; i++) bot_stops += histogram_stats[i]; // bottom third
			for(; i<64; i++) midA_stops += histogram_stats[i]; // middle third
			for(; i<85; i++) midB_stops += histogram_stats[i]; // middle third
			for(; i<96; i++) midC_stops += histogram_stats[i]; // middle third
			for(; i<116; i++) midD_stops += histogram_stats[i]; 
			for(; i<128; i++) clipped += histogram_stats[i];  // clipped bright blue sky is luma around 240-242
			top_stops = midC_stops + midD_stops + clipped; // 0 to 170, top third
			twothirds = (bot_stops+midA_stops+midB_stops);

            int maxexpo = 8250 * power;
            if(*enc_frames == 0)
                maxexpo = 33000; // in preview don't limit the gain.
                
			if((midD_stops + clipped + bot_stops)*16 < midA_stops + midB_stops + midC_stops) // low contrast negative, have a peak in the middle.
			{
				// no change
			}
			else if(clipped > (total>>7))
			{
				//maybe clipping
				newexpo = (currexpo * 15984)>>14;   // decrease by 1.025
				if(newexpo > 750)
					nextexpo = newexpo; 
			}
			else if(top_stops > twothirds)
			{
				//maybe overexposed
				newexpo = (currexpo * 16222)>>14;   // decrease by 1.01
				if(newexpo > 750)
					nextexpo = newexpo; 
			}
			else if(top_stops < (total>>8)) // almost no data in the last ~1 stop
			{
				// underexposed 
				newexpo = (currexpo * 16794)>>14;  // increase slightly 1.025x
				if(newexpo < maxexpo)// keep exposure less than 8ms at ISO 400
					nextexpo = newexpo;
			}
            
			{
				int newiso = 50;
                
                if(*enc_frames > 0)
                {
    				*expo_change = *frameno;
				
				    while(nextexpo >= 8000 && newiso < power*100 )
				    {
					    newiso *= 2;
					    nextexpo /= 2;
				    }
                    while(nextexpo >= 8000)
				    {
					    nextexpo /= 2;
				    }
                }
                else // preview
                {
                    // keep the exposure time below 2.5ms to improve frame grab stability
                    while(nextexpo >= 2000 && newiso < 800)  
				    {
					    newiso *= 2;
					    nextexpo /= 2;
				    }
                }
            
				int last_iso = *expo_iso;
				int last_time = *expo_time;
                
				*expo_iso = newiso;
				*expo_time = nextexpo;
            }
		}
	}
#endif
	return;
}

int main(void)
{
    calc_histogram();
    return 0;
}
