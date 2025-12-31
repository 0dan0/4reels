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

enum LEVELS { 
   LVL_P20,
   LVL_P15,
   LVL_P10,
   LVL_P05,
   LVL_000,
   LVL_N05,
   LVL_N10,
   LVL_N15,
   LVL_N20,
};

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


void select_wb(void)
{	
	asm volatile (
		".word 0x202d2d2d\n" //--- 
		".word 0x20545543\n" //CUT 
		".word 0x45524548\n" //HERE
		".word 0x2d2d2d20\n" //--- 
	);
	
	volatile int* reelType = (int *)0x80340000;
	volatile int* frameno = (int *)0x80f8214c; //frame counter
	volatile uint32_t *enc_frames = (uint32_t *) 0x85bf0014;
	int32_t* nvm_base = (uint32_t *)0x80E0B78C; //Type A - exposure, sharpness, tint
	uint32_t* active_settings = (uint32_t *)0x80DDC11C; // Settings for exposure, sharpness, tint
    volatile uint32_t* button = (uint32_t *)0xA0E8BFF8; // uncached
	volatile int* button_read = (int *)0x85bf002c; // my flag to acknowledge the button press.
    
    if(*reelType == 2)
    {
		nvm_base = (int *)0x80E0B87C; //exposure, sharpness, tint
        active_settings = (uint32_t *)0x80DDC204;
        button = (uint32_t *)0xA0E8C0E8;
    }
	if(*reelType == 3)
    {
		nvm_base = (int *)0x80E0AD0C; //exposure, sharpness, tint
        active_settings = (uint32_t *)0x80DDB69C;
        button = (uint32_t *)0xA0E8B578;
    }
    
	if(*frameno > 25 && *frameno < 3600*24*25) 
	{
	    uint32_t r,g,b,*wb_gains = (uint32_t *)0x85bf0020;
                
        uint32_t* whitebal = &nvm_base[NVM_WBAL];
	    uint32_t* sharpness = &nvm_base[NVM_SHARPEN];
	    uint32_t* saturation = &nvm_base[NVM_SAT];       
        
        if(nvm_base[NVM_FREE] == 0) // reset to zero on a FW update.
        {
            nvm_base[NVM_FREE] = 1;
            nvm_base[NVM_EXPLOCK] = 0; //reset to Auto exposure.
            nvm_base[NVM_ISO_LOCK] = 100;
            nvm_base[NVM_SHUT_LOCK] = 2048;
            nvm_base[NVM_NAV] = 3; //EV  <- This is causing the first boot after flashing, not to run (when NVM_NAV was 4)
            nvm_base[NVM_WB_MODS]=0;    
            
            if(nvm_base[NVM_SAVE_WBAL] > 0 || nvm_base[NVM_SAVE_SHARPEN] > 0 || nvm_base[NVM_SAVE_SAT] > 0)
            {
                active_settings[NVM_WBAL] = nvm_base[NVM_WBAL] = nvm_base[NVM_SAVE_WBAL]; 
                active_settings[NVM_SHARPEN] = nvm_base[NVM_SHARPEN] = nvm_base[NVM_SAVE_SHARPEN]; 
                active_settings[NVM_SAT] = nvm_base[NVM_SAT] = nvm_base[NVM_SAVE_SAT]; 
                
                if(active_settings[NVM_WBAL] == 4 && active_settings[NVM_SHARPEN] == 4 && active_settings[NVM_SAT] == 4)
                {
                    active_settings[NVM_WBAL] = nvm_base[NVM_WBAL];
                    active_settings[NVM_SHARPEN] = nvm_base[NVM_SHARPEN];
                    active_settings[NVM_SAT] = nvm_base[NVM_SAT];
                }
            }
        }
        
        int sr = (nvm_base[NVM_WB_MODS]<<8) >> 24;
        int sb = (nvm_base[NVM_WB_MODS]<<16) >> 24;
        int sg = (nvm_base[NVM_WB_MODS]<<24) >> 24;
        
        r = 0x1D0; g = 0x100; b = 0x100;
	    if(*whitebal == LVL_P20) { r += 0x80;              }
	    if(*whitebal == LVL_P15) { r += 0x60;              }
	    if(*whitebal == LVL_P10) { r += 0x40;              }
	    if(*whitebal == LVL_P05) { r += 0x20;              }
	    if(*whitebal == LVL_000) {                         }
	    if(*whitebal == LVL_N05) {             b += 0x20;  }
	    if(*whitebal == LVL_N10) { r -= 0x20;  b += 0x40;  }
	    if(*whitebal == LVL_N15) { r -= 0xA0;  b += 0xc0;  }
	    if(*whitebal == LVL_N20) { r -= 0xD0;  b += 0xc0;  }
       
       
        if(button[3] >= 2)// button pressed and held ~0.1s
        {
            if(*enc_frames > 0 && *enc_frames < 100000 && *button_read == 0) // if recording, use navigation
            {
                *button_read = 1;
                if(button[0] == BUTTON_UP || button[0] == BUTTON_LEFT) 
                    nvm_base[NVM_NAV]--;
                if(button[0] == BUTTON_DOWN || button[0] == BUTTON_RIGHT) 
                    nvm_base[NVM_NAV]++;
                    
                // 0 wb_mods_r, 1 blue, 2 green, 3 ev, 4 fps, 5 Qp, 6 ExpLock 
                if(nvm_base[NVM_NAV] < 0) 
                    nvm_base[NVM_NAV] = 7;
                if(nvm_base[NVM_NAV] > 7) 
                    nvm_base[NVM_NAV] = 0;
                 
                int addr = nvm_base[NVM_NAV] - 2;
                if(addr >= 1)
                {
                    if(button[0] == BUTTON_PLUS)  //EV Bias
                        nvm_base[NVM_WB_MODS+addr]++;
                      
                    if(button[0] == BUTTON_NEG)
                        nvm_base[NVM_WB_MODS+addr]--;
                }
                else
                {
                    if(addr == -2) //red
                    {
                        if(button[0] == BUTTON_PLUS)
                            sr++;
                        if(button[0] == BUTTON_NEG)
                            sr--;
                    }
                    if(addr == -1) //green
                    {
                        if(button[0] == BUTTON_PLUS)
                            sg++;
                        if(button[0] == BUTTON_NEG)
                            sg--;
                    }
                    if(addr == 0) //blue
                    {
                        if(button[0] == BUTTON_PLUS)
                            sb++;
                        if(button[0] == BUTTON_NEG)
                            sb--;
                    }
                    
                    nvm_base[NVM_WB_MODS] = ((sr << 16) & 0xff0000) | ((sb << 8) & 0xff00) | (sg & 0xff);
                }
            }
        }
        else
        {
            *button_read = 0;
        }
        //if(button[3] >= 10 && button[3] < 18)// button pressed and held ~0.6s
        //{
        //    {                
        //        if(button[0] == BUTTON_RIGHT)  //Green Tint
        //            sg++;
        //        if(button[0] == BUTTON_LEFT)
        //            sg--;
        //    }
        //}
        if(nvm_base[NVM_WB_MODS] < 0) nvm_base[NVM_WB_MODS]=0;  //RGB Tint Initialize
        
        if(nvm_base[NVM_EVBIAS] < -8 || nvm_base[NVM_EVBIAS] > 8) nvm_base[NVM_EVBIAS]=0;  //EV Bias Initialize
        if(nvm_base[NVM_EVBIAS] < -7) nvm_base[NVM_EVBIAS]=-7;  //EV Bias
        if(nvm_base[NVM_EVBIAS] >  7) nvm_base[NVM_EVBIAS]=7;
        
        if(nvm_base[NVM_FPS] < 0 || nvm_base[NVM_FPS] > 2) nvm_base[NVM_FPS]=1;  //Frame Rate, 16, 18 & 24
        if(nvm_base[NVM_QPMIN] < 16) nvm_base[NVM_QPMIN]=16;  //Qp min
        if(nvm_base[NVM_QPMIN] > 30) nvm_base[NVM_QPMIN]=30;  //Qp min
        if(nvm_base[NVM_ISOMAX] > 2) nvm_base[NVM_ISOMAX]=2;  //400 ISO max
        if(nvm_base[NVM_ISOMAX] < 0) nvm_base[NVM_ISOMAX]=0;  //100 ISO max

        // wb tint control 
        r += sr * 0x10 + (sr ? 1 : 0);
        g += sg * 0x10;
        b += sb * 0x10 + (sb ? 1 : 0);
	
		wb_gains[0] = r; 
		wb_gains[1] = g; 
		wb_gains[2] = b; 
        
        if(nvm_base[NVM_SAVE_WBAL] != nvm_base[NVM_WBAL])
            nvm_base[NVM_SAVE_WBAL] = nvm_base[NVM_WBAL]; 
            
        if(nvm_base[NVM_SAVE_SHARPEN] != nvm_base[NVM_SHARPEN])
            nvm_base[NVM_SAVE_SHARPEN] = nvm_base[NVM_SHARPEN]; 
            
        if(nvm_base[NVM_SAVE_SAT] != nvm_base[NVM_SAT])
            nvm_base[NVM_SAVE_SAT] = nvm_base[NVM_SAT]; 
	}
	return;
}

int main(void)
{
    select_wb();

    return 0;
}
