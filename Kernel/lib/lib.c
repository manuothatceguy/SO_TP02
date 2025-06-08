// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>

void * memset(void * destination, int32_t c, uint64_t length)
{
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destination;

	while(length--)
		dst[length] = chr;

	return destination;
}

void * memcpy(void * destination, const void * source, uint64_t length)
{
	/*
	* memcpy does not support overlapping buffers, so always do it
	* forwards. (Don't change this without adjusting memmove.)
	*
	* For speedy copying, optimize the common case where both pointers
	* and the length are word-aligned, and copy word-at-a-time instead
	* of byte-at-a-time. Otherwise, copy by bytes.
	*
	* The alignment logic below should be portable. We rely on
	* the compiler to be reasonably intelligent about optimizing
	* the divides and modulos out. Fortunately, it is.
	*/
	uint64_t i;

	if ((uint64_t)destination % sizeof(uint32_t) == 0 &&
		(uint64_t)source % sizeof(uint32_t) == 0 &&
		length % sizeof(uint32_t) == 0)
	{
		uint32_t *d = (uint32_t *) destination;
		const uint32_t *s = (const uint32_t *)source;

		for (i = 0; i < length / sizeof(uint32_t); i++)
			d[i] = s[i];
	}
	else
	{
		uint8_t * d = (uint8_t*)destination;
		const uint8_t * s = (const uint8_t*)source;

		for (i = 0; i < length; i++)
			d[i] = s[i];
	}

	return destination;
}

char * strncpy(char * destination, const char * source, uint64_t length)
{
    uint64_t i;
    for (i = 0; i < length && source[i] != '\0'; i++)
        destination[i] = source[i];
    for (; i < length; i++)
        destination[i] = '\0';
    return destination;
}

uint64_t strlen(const char * str) {
    uint64_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int64_t satoi(char *str) {
  uint64_t i = 0;
  int64_t res = 0;
  int8_t sign = 1;

  if (!str)
    return 0;

  if (str[i] == '-') {
    i++;
    sign = -1;
  }

  for (; str[i] != '\0'; ++i) {
    if (str[i] < '0' || str[i] > '9')
      return 0;
    res = res * 10 + str[i] - '0';
  }

  return res * sign;
}

char* itoa(int64_t value, char* str, int base) {
    // Handle negative numbers
    int isNegative = 0;
    if (value < 0 && base == 10) {
        isNegative = 1;
        value = -value;
    }

    // Handle 0 explicitly
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    // Process individual digits
    int i = 0;
    while (value > 0) {
        int digit = value % base;
        str[i++] = (digit > 9) ? (digit - 10) + 'a' : digit + '0';
        value = value / base;
    }

    // Add negative sign if needed
    if (isNegative) {
        str[i++] = '-';
    }

    // Reverse the string
    str[i] = '\0';
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    return str;
}

