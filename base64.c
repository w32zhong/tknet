#include "base64.h"
 
void 
Base64Encode(char *src, int src_len, char *dst)
{
	int i = 0, j = 0;
	char base64_map[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	for (; i < src_len - src_len % 3; i += 3) 
	{
		dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
		dst[j++] = base64_map[((src[i] << 4) & 0x30) + ((src[i + 1] >> 4) & 0xF)];
		dst[j++] = base64_map[((src[i + 1] << 2) & 0x3C) + ((src[i + 2] >> 6) & 0x3)];
		dst[j++] = base64_map[src[i + 2] & 0x3F];
	}

	if (src_len % 3 == 1) 
	{
		dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
		dst[j++] = base64_map[(src[i] << 4) & 0x30];
		dst[j++] = '=';
		dst[j++] = '=';
	}
	else if (src_len % 3 == 2) 
	{
		dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
		dst[j++] = base64_map[((src[i] << 4) & 0x30) + ((src[i + 1] >> 4) & 0xF)];
		dst[j++] = base64_map[(src[i + 1] << 2) & 0x3C];
		dst[j++] = '=';
	}

	dst[j] = '\0';
}

