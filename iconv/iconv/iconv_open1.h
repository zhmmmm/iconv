{
char buf[MAX_WORD_LENGTH + 10 + 1];
const char* cp;
char* bp;
const struct alias * ap;
unsigned int count;

transliterate = 0;
discard_ilseq = 0;

for (to_wchar = 0;;)
{

	for (cp = tocode, bp = buf, count = MAX_WORD_LENGTH + 10 + 1; ; cp++, bp++)
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
	for (;;)
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
	if (buf[0] == '\0') {
		tocode = locale_charset();
		/* Avoid an endless loop that could occur when using an older version
		   of localcharset.c. */
		if (tocode[0] == '\0')
			goto invalid;
		continue;
	}
	ap = aliases_lookup(buf, bp - buf);
	if (ap == NULL) {
		ap = aliases2_lookup(buf);
		if (ap == NULL)
			goto invalid;
	}
	if (ap->encoding_index == ei_local_char) {
		tocode = locale_charset();
		/* Avoid an endless loop that could occur when using an older version
		   of localcharset.c. */
		if (tocode[0] == '\0')
			goto invalid;
		continue;
	}
	if (ap->encoding_index == ei_local_wchar_t) {
		/* On systems which define __STDC_ISO_10646__, wchar_t is Unicode.
		   This is also the case on native Woe32 systems.  */
#if __STDC_ISO_10646__ || ((defined _WIN32 || defined __WIN32__) && !defined __CYGWIN__)
		if (sizeof(wchar_t) == 4) {
			to_index = ei_ucs4internal;
			break;
		}
		if (sizeof(wchar_t) == 2) {
			to_index = ei_ucs2internal;
			break;
		}
		if (sizeof(wchar_t) == 1) {
			to_index = ei_iso8859_1;
			break;
		}
#endif
#if HAVE_MBRTOWC
		to_wchar = 1;
		tocode = locale_charset();
		continue;
#endif
		goto invalid;
	}
	to_index = ap->encoding_index;
	break;
}
for (from_wchar = 0;;)
{
	for (cp = fromcode, bp = buf, count = MAX_WORD_LENGTH + 10 + 1; ; cp++, bp++)
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
	for (;;)
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
#if __STDC_ISO_10646__ || ((defined _WIN32 || defined __WIN32__) && !defined __CYGWIN__)
		if (sizeof(wchar_t) == 4) {
			from_index = ei_ucs4internal;
			break;
		}
		if (sizeof(wchar_t) == 2) {
			from_index = ei_ucs2internal;
			break;
		}
		if (sizeof(wchar_t) == 1) {
			from_index = ei_iso8859_1;
			break;
		}
#endif
#if HAVE_WCRTOMB
		from_wchar = 1;
		fromcode = locale_charset();
		continue;
#endif
		goto invalid;
	}
	from_index = ap->encoding_index;
	break;
}
}
