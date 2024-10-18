#include <naiveConsole.h>

static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

static char buffer[64] = { '0' };
static uint8_t * const video = (uint8_t*)0xB8000;
static uint8_t * currentVideo = (uint8_t*)0xB8000;
static const uint32_t width = 80;
static const uint32_t height = 25;
#define DEFAULT_FMT 0x0f

void ncPrint(const char * string)
{
	for (int i = 0; string[i] != 0; i++)
		ncPrintChar(string[i]);
}

void ncPrintFmt(const char * string, char fmt){
	for (int i = 0; string[i] != 0; i++)
		ncPrintCharFmt(string[i],fmt);
}

void checkFull(){
	if(currentVideo == (video + height*width*sizeof(uint16_t))){
		ncClear();
	}
}

void ncPrintChar(char character)
{
	checkFull();
	*currentVideo = character;
	currentVideo += 2;
}

void ncPrintCharFmt(char character, char fmt)
{
	checkFull();
	*currentVideo++ = character;
	*currentVideo++ = fmt;
}

void ncNewline()
{
	do
	{
		ncPrintChar(' ');
	}
	while((uint64_t)(currentVideo - video) % (width * 2) != 0);
}

void ncPrintDec(uint64_t value)
{
	ncPrintBase(value, 10);
}

void ncPrintHex(uint64_t value)
{
	ncPrintBase(value, 16);
}

void ncPrintBin(uint64_t value)
{
	ncPrintBase(value, 2);
}

void ncPrintBase(uint64_t value, uint32_t base)
{
    uintToBase(value, buffer, base);
    ncPrint(buffer);
}

typedef struct coloredChar{
	char c;
	char color;
} coloredChar;

void ncClear()
{
	int i;
	coloredChar * v = video;

	for(int i = 0; i < height * width; i++){
		v[i].c = ' ';
		v[i].color = DEFAULT_FMT;
	}
	currentVideo = video;
}

void ncDelete()
{
	currentVideo -= 2;
	ncPrintCharFmt(' ',DEFAULT_FMT);
	currentVideo -= 2;
}

void ncScrollUp(){
	coloredChar ** v = video;
	for(int i = 0; i < height - 1; i++){
		for(int j = 0; j < width; j++){
			v[i][j] = v[i+1][j];
		}
	}
	for(int i = 0; i < width; i++){
		v[height-1][i].c = ' ';
	}
	
}

void ncScrollDown(){
	coloredChar ** v = video;
	for(int i = 0; i < height-1; i++){
		for(int j = 0; j < width; j++){
			v[i+1][j] = v[i][j];
		}
	}
	for(int i = 0; i < width; i++){
		v[0][i].c = ' ';
	}
	currentVideo += 2*width;
}



static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base)
{
	char *p = buffer;
	char *p1, *p2;
	uint32_t digits = 0;

	//Calculate characters for each digit
	do
	{
		uint32_t remainder = value % base;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
		digits++;
	}
	while (value /= base);

	// Terminate string in buffer.
	*p = 0;

	//Reverse string in buffer.
	p1 = buffer;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}

	return digits;
}
