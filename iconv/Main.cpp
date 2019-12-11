#include <iostream>
#include "include/iconv.h"

int main()
{


	char src[] = "ÄãºÃ";
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


	system("pause");
	return 0;
}