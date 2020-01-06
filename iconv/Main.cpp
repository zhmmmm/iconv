#include <iostream>
#include "iconv/iconv.h"

int main()
{


	char src[] = "¹þ¹þ1321321fsdff1sd231·Å´øÉÏ";
	char dst[100] = { 0 };
	size_t srclen = strlen(src);
	size_t dstlen = strlen(src) * 3;
	fprintf(stderr, "in: %s\n", src);
	char * pIn = src;
	char * pOut = (char*)dst;
	iconv_t conv = iconv_open("UTF-8", "GB2312");
	iconv(conv, (const char **)&pIn, &srclen, &pOut, &dstlen);
	iconv_close(conv);
	fprintf(stderr, "out: %s\n", dst);
	unsigned short _0 = (unsigned short)src;
	unsigned short _1 = (unsigned short)(src + 2);
	std::cout << _0 << " " << _1 << std::endl;



	system("pause");
	return 0;
}