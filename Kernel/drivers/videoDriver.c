// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <videoDriver.h>

struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y) {
    uint8_t * framebuffer = (uint8_t *)(uintptr_t) VBE_mode_info->framebuffer;
	/**
	 videoDriver.c: In function 'putPixel':
	 videoDriver.c:46:29: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
   		46 |     uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
	 */
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

uint8_t * getFrameBuffer(){
	return (uint8_t *)(uintptr_t) VBE_mode_info->framebuffer;
}

void clearScreen(uint32_t color) {
    for (uint64_t y = 0; y < VBE_mode_info->height; y++) {
        for (uint64_t x = 0; x < VBE_mode_info->width; x++) {
            putPixel(color, x, y);
        }
    } 
}

uint16_t getWidth(){
	return VBE_mode_info->width;
}

uint16_t getHeight(){
	return VBE_mode_info->height;
}

uint16_t getPitch(){
	return VBE_mode_info->pitch;
}

int drawSquare(uint64_t x, uint64_t y, uint64_t sideLength, uint32_t hexColor){
	if(x > VBE_mode_info->width || y > VBE_mode_info->height || sideLength <= 0){
		return -1; // Error de argumentos.
	}
	int i = x, j = y;
	for(; i < x+sideLength && i < VBE_mode_info->width; i++){
		for(j = y; j < y+sideLength && j < VBE_mode_info->height; j++ ){
			putPixel(hexColor,i,j);
		}
	}
	return (sideLength * sideLength) - ((i - x) * (j - y)); // Retorna la cantidad de pixeles no dibujados.
}

int drawRectangle(uint64_t x, uint64_t y, uint64_t vLength, uint64_t hLength, uint32_t hexColor){
	if(x > VBE_mode_info->width || y > VBE_mode_info->height || vLength <= 0 || hLength <= 0){
		return -1; // Error de argumentos.
	}
	for(int i = x; i < hLength + x; i++){
		for (int j = y; j < vLength + y; j++){
			putPixel(hexColor,i,j);
		}
	}
	return 0;
}


