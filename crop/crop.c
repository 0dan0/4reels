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

extern void console(const char *ctx, int a, int b);
extern void consoleD(const char *ctx, int a, int b); 
extern void setdate(int year, int month, int day, int zero); 

void crop_mod(void)
{
    register uintptr_t s0val asm("s0");   
    uint32_t *scanSize = (uint32_t *)s0val;
    
	char* format_old_offsets = (char *)0x8033b020;
	char* format_new_offsets = (char *)0x8033b0f0;
	char* format_old_sizes = (char *)0x8033b010;
	char* format_new_sizes = (char *)0x8033b0e0;
    char* format_proc_size = (char *)0x8033b060;
           
    uint32_t x_off = scanSize[0x04 >> 2];
    uint32_t y_off = scanSize[0x08 >> 2];
    uint32_t width = scanSize[0x0c >> 2];
    uint32_t height = scanSize[0x10 >> 2];
        
    uint32_t uVar1;
    uint32_t uVar2;
    int iVar3;
    uint32_t uVar4;
    uint32_t uVar5;
           
    int32_t new_width = width;
	volatile int* reelType = (int *)0x80340000;
	if(*reelType < 4)
        new_width = (width-440)*4;  // Math good for A thru C, not for D
        
    if (1600 < new_width) new_width = 1600;
    scanSize[0x0c >> 2] = new_width; //new width
    scanSize[0x14 >> 2] = new_width; //new width
    scanSize[0x20 >> 2] = new_width; //new width
    int pitch = (new_width + (new_width >> 1) + 3 >> 2) << 2;
    scanSize[0x1c >> 2] = pitch; //pitch?
    scanSize[0x28 >> 2] = pitch;
    int new_height = (((new_width >> 1) + (new_width >> 2) + 3 >> 2) << 2);
    scanSize[0x10 >> 2] = new_height; // new height
    scanSize[0x18 >> 2] = new_height; // new height
    scanSize[0x24 >> 2] = new_height; // new height
    
    int new_x_off = (int)x_off + (width>>1) - (new_width>>1);
    int new_y_off = (int)y_off - 80 + (height>>1) - (new_height>>1);
    if(new_x_off < 0) new_x_off = 0;
    if(new_y_off < 0) new_y_off = 0;
    scanSize[0x04 >> 2] = new_x_off; //new x offset
    scanSize[0x08 >> 2] = new_y_off; //new y offset
    
    if(*reelType < 4)
    {
        console(format_old_offsets, x_off, y_off); 
        console(format_old_sizes, width, height); 
        console(format_new_offsets, new_x_off, new_y_off); 
        console(format_new_sizes, new_width, new_height); 
        console(format_proc_size, new_width, new_height); 
        setdate(2026,1,27,0);
    }
    else
    {
        consoleD(format_old_offsets, x_off, y_off); 
        consoleD(format_old_sizes, width, height); 
        consoleD(format_new_offsets, new_x_off, new_y_off); 
        consoleD(format_new_sizes, new_width, new_height); 
        consoleD(format_proc_size, new_width, new_height); 
        //setdateD(2026,1,27,0);
    }
    
	volatile uint32_t *my_memory = (uint32_t *)0x85bf0000;
    my_memory[0x14 >> 2] = 0;
    my_memory[0x18 >> 2] = 0;
    my_memory[0x1c >> 2] = 0;
    my_memory[0x10 >> 2] = 0xffff0000;
    my_memory[0x30 >> 2] = new_width;
    my_memory[0x34 >> 2] = new_height;
    my_memory[0x38 >> 2] = new_x_off;
    my_memory[0x3c >> 2] = new_y_off;
    
	volatile uint32_t *iso = (uint32_t *) 0x80e556b4;
    iso[0] = 800;
    iso[1] = 1;  
	return;
}

int main(void)
{
    crop_mod();

    return 0;
}
