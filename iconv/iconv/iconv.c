#include "iconv.h"
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "localcharset.h"

#define USE_DOS

struct loop_funcs
{
	size_t(*loop_convert) (iconv_t icd, const char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
	size_t(*loop_reset) (iconv_t icd, char* * outbuf, size_t *outbytesleft);
};

#include "converters.h"
#include "cjk_variants.h"
#include "translit.h"

struct encoding
{
	struct mbtowc_funcs ifuncs; /* conversion multibyte -> unicode */
	struct wctomb_funcs ofuncs; /* conversion unicode -> multibyte */
	int oflags;                 /* flags for unicode -> multibyte conversion */
};
#define DEFALIAS(xxx_alias,xxx)
enum
{
#define DEFENCODING(xxx_names,xxx,xxx_ifuncs1,xxx_ifuncs2,xxx_ofuncs1,xxx_ofuncs2) \
  ei_##xxx ,
#include "encodings.def"
# include "encodings_dos.def"
#include "encodings_local.def"
};

#include "flags.h"
static struct encoding const all_encodings[] = 
{
#define DEFENCODING(xxx_names,xxx,xxx_ifuncs1,xxx_ifuncs2,xxx_ofuncs1,xxx_ofuncs2) \
  { xxx_ifuncs1,xxx_ifuncs2, xxx_ofuncs1,xxx_ofuncs2, ei_##xxx##_oflags },
#include "encodings.def"
# include "encodings_dos.def"
#undef DEFENCODING
#define DEFENCODING(xxx_names,xxx,xxx_ifuncs1,xxx_ifuncs2,xxx_ofuncs1,xxx_ofuncs2) \
  { xxx_ifuncs1,xxx_ifuncs2, xxx_ofuncs1,xxx_ofuncs2, 0 },
#include "encodings_local.def"
};

#include "loop_unicode.h"
# include "aliases.h"

struct stringpool2_t
{
#define S(tag,name,encoding_index) char stringpool_##tag[sizeof(name)];
#include "aliases_dos.h"
};
static const struct stringpool2_t stringpool2_contents =
{
#define S(tag,name,encoding_index) name,
#include "aliases_dos.h"
};

#define stringpool2 ((const char *) &stringpool2_contents)


static const struct alias sysdep_aliases[] = 
{
#define S(tag,name,encoding_index) { (int)(long)&((struct stringpool2_t *)0)->stringpool_##tag, encoding_index },
#include "aliases_dos.h"
};

const struct alias *aliases2_lookup(register const char *str)
{
	const struct alias * ptr;
	unsigned int count;
	for (ptr = sysdep_aliases, count = sizeof(sysdep_aliases) / sizeof(sysdep_aliases[0]); count > 0; ptr++, count--)
		if (!strcmp(str, stringpool2 + ptr->name))
			return ptr;
	return NULL;
}


iconv_t iconv_open(const char* tocode, const char* fromcode)
{
	struct conv_struct * cd;
	unsigned int from_index;
	int from_wchar;
	unsigned int to_index;
	int to_wchar;
	int transliterate;
	int discard_ilseq;

	char buf[45 + 10 + 1];
	const char* cp;
	char* bp;
	const struct alias * ap;
	unsigned int count;

	transliterate = 0;
	discard_ilseq = 0;

	for (to_wchar = 0;;)
	{
		for (cp = tocode, bp = buf, count = 45 + 10 + 1; ; cp++, bp++)
		{
			unsigned char c = *(unsigned char *)cp;
			if (c >= 0x80)
				goto invalid;
			if (c >= 'a' && c <= 'z')
				c -= 'a' - 'A';
			*bp = c;
			if (c == '\0')
				break;
			if (--count == 0)
				goto invalid;
		}
		while(1)
		{
			if (bp - buf >= 10 && memcmp(bp - 10, "//TRANSLIT", 10) == 0)
			{
				bp -= 10;
				*bp = '\0';
				transliterate = 1;
				continue;
			}
			if (bp - buf >= 8 && memcmp(bp - 8, "//IGNORE", 8) == 0)
			{
				bp -= 8;
				*bp = '\0';
				discard_ilseq = 1;
				continue;
			}
			break;
		}
		if (buf[0] == '\0') 
		{
			tocode = locale_charset();
			if (tocode[0] == '\0')
				goto invalid;
			continue;
		}
		ap = aliases_lookup(buf, bp - buf);
		if (ap == NULL)
		{
			ap = aliases2_lookup(buf);
			if (ap == NULL)
				goto invalid;
		}
		if (ap->encoding_index == ei_local_char)
		{
			tocode = locale_charset();
			if (tocode[0] == '\0')
				goto invalid;
			continue;
		}
		if (ap->encoding_index == ei_local_wchar_t)
		{
			if (sizeof(wchar_t) == 4)
			{
				to_index = ei_ucs4internal;
				break;
			}
			if (sizeof(wchar_t) == 2)
			{
				to_index = ei_ucs2internal;
				break;
			}
			if (sizeof(wchar_t) == 1)
			{
				to_index = ei_iso8859_1;
				break;
			}
			goto invalid;
		}
		to_index = ap->encoding_index;
		break;
	}
	for (from_wchar = 0;;)
	{
		for (cp = fromcode, bp = buf, count = 45 + 10 + 1; ; cp++, bp++)
		{
			unsigned char c = *(unsigned char *)cp;
			if (c >= 0x80)
				goto invalid;
			if (c >= 'a' && c <= 'z')
				c -= 'a' - 'A';
			*bp = c;
			if (c == '\0')
				break;
			if (--count == 0)
				goto invalid;
		}
		while(1)
		{
			if (bp - buf >= 10 && memcmp(bp - 10, "//TRANSLIT", 10) == 0)
			{
				bp -= 10;
				*bp = '\0';
				continue;
			}
			if (bp - buf >= 8 && memcmp(bp - 8, "//IGNORE", 8) == 0)
			{
				bp -= 8;
				*bp = '\0';
				continue;
			}
			break;
		}
		if (buf[0] == '\0')
		{
			fromcode = locale_charset();
			if (fromcode[0] == '\0')
				goto invalid;
			continue;
		}
		ap = aliases_lookup(buf, bp - buf);
		if (ap == NULL)
		{
			ap = aliases2_lookup(buf);
			if (ap == NULL)
				goto invalid;
		}
		if (ap->encoding_index == ei_local_char)
		{
			fromcode = locale_charset();
			if (fromcode[0] == '\0')
				goto invalid;
			continue;
		}
		if (ap->encoding_index == ei_local_wchar_t)
		{
			if (sizeof(wchar_t) == 4)
			{
				from_index = ei_ucs4internal;
				break;
			}
			if (sizeof(wchar_t) == 2)
			{
				from_index = ei_ucs2internal;
				break;
			}
			if (sizeof(wchar_t) == 1)
			{
				from_index = ei_iso8859_1;
				break;
			}
			goto invalid;
		}
		from_index = ap->encoding_index;
		break;
	}

	cd = (struct conv_struct *) malloc(from_wchar != to_wchar
		? sizeof(struct conv_struct)
		: sizeof(struct conv_struct));
	if (cd == NULL) 
	{
		errno = ENOMEM;
		return (iconv_t)(-1);
	}

	cd->iindex = from_index;
	cd->ifuncs = all_encodings[from_index].ifuncs;
	cd->oindex = to_index;
	cd->ofuncs = all_encodings[to_index].ofuncs;
	cd->oflags = all_encodings[to_index].oflags;
	cd->lfuncs.loop_convert = unicode_loop_convert;
	cd->lfuncs.loop_reset = unicode_loop_reset;
	memset(&cd->istate, '\0', sizeof(state_t));
	memset(&cd->ostate, '\0', sizeof(state_t));
	cd->transliterate = transliterate;
	cd->discard_ilseq = discard_ilseq;
	cd->fallbacks.mb_to_uc_fallback = NULL;
	cd->fallbacks.uc_to_mb_fallback = NULL;
	cd->fallbacks.mb_to_wc_fallback = NULL;
	cd->fallbacks.wc_to_mb_fallback = NULL;
	cd->fallbacks.data = NULL;
	cd->hooks.uc_hook = NULL;
	cd->hooks.wc_hook = NULL;
	cd->hooks.data = NULL;

	if (from_wchar != to_wchar)
	{
		struct wchar_conv_struct * wcd = (struct wchar_conv_struct *) cd;
	}

	return (iconv_t)cd;

invalid:
	errno = EINVAL;
	return (iconv_t)(-1);
}

size_t iconv(iconv_t icd,ICONV_CONST char* * inbuf, size_t *inbytesleft,char* * outbuf, size_t *outbytesleft)
{
	conv_t cd = (conv_t)icd;
	return cd->lfuncs.loop_convert(icd,(const char* *)inbuf, inbytesleft,outbuf, outbytesleft);
}

int iconv_close(iconv_t icd)
{
	conv_t cd = (conv_t)icd;
	free(cd);
	return 0;
}
