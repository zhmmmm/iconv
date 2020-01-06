#ifndef _LIBICONV_H
#define _LIBICONV_H
///-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <stddef.h>
#include <errno.h>

#define ICONV_TRIVIALP            0  /* int *argument */
#define ICONV_GET_TRANSLITERATE   1  /* int *argument */
#define ICONV_SET_TRANSLITERATE   2  /* const int *argument */
#define ICONV_GET_DISCARD_ILSEQ   3  /* int *argument */
#define ICONV_SET_DISCARD_ILSEQ   4  /* const int *argument */
#define ICONV_SET_HOOKS           5  /* const struct iconv_hooks *argument */
#define ICONV_SET_FALLBACKS       6  /* const struct iconv_fallbacks *argument */
#define _LIBICONV_VERSION 0x010D

#define iconv_t libiconv_t
extern  int _libiconv_version;
typedef void* iconv_t;
#define iconv_open libiconv_open
#define iconv libiconv
#define iconv_close libiconv_close
#define iconv_open_into libiconv_open_into
#define iconvctl libiconvctl
#define iconvlist libiconvlist

typedef void(*iconv_unicode_char_hook) (unsigned int uc, void* data);
typedef void(*iconv_wide_char_hook) (wchar_t wc, void* data);
typedef void(*iconv_unicode_mb_to_uc_fallback)(const char* inbuf, size_t inbufsize, void(*write_replacement) (const unsigned int *buf, size_t buflen, void* callback_arg), void* callback_arg, void* data);
typedef void(*iconv_unicode_uc_to_mb_fallback)(unsigned int code, void(*write_replacement) (const char *buf, size_t buflen, void* callback_arg), void* callback_arg, void* data);
typedef void(*iconv_wchar_mb_to_wc_fallback) ();
typedef void(*iconv_wchar_wc_to_mb_fallback) ();
typedef struct
{
	void* dummy1[28];
} iconv_allocation_t;
struct iconv_hooks
{
	iconv_unicode_char_hook uc_hook;
	iconv_wide_char_hook wc_hook;
	void* data;
};
struct iconv_fallbacks
{
	iconv_unicode_mb_to_uc_fallback mb_to_uc_fallback;
	iconv_unicode_uc_to_mb_fallback uc_to_mb_fallback;
	iconv_wchar_mb_to_wc_fallback mb_to_wc_fallback;
	iconv_wchar_wc_to_mb_fallback wc_to_mb_fallback;
	void* data;
};

#ifdef __cplusplus 
extern "C"
{
	extern iconv_t iconv_open(const char* tocode, const char* fromcode);
	extern size_t iconv(iconv_t cd, const char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
	extern int iconv_close(iconv_t cd);
	extern int iconv_open_into(const char* tocode, const char* fromcode, iconv_allocation_t* resultp);
	extern int iconvctl(iconv_t cd, int request, void* argument);
	extern void iconvlist(int(*do_one) (unsigned int namescount,const char * const * names,void* data),void* data);
	extern const char * iconv_canonicalize(const char * name);
	extern const char * locale_charset(void);
}
#endif
#endif /* _LIBICONV_H */
