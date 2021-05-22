#ifndef URL_ENCODE_H
#define URL_ENCODE_H

static char rfc3986[256] = {0};
static char html5[256] = {0};
static void url_encoder_rfc_tables_init() {
  static char x = 0;
  if (x) return;
  int i;
  for (i = 0; i < 256; i++){
    rfc3986[i] = isalnum( i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
    html5[i] = isalnum( i) || i == '*' || i == '-' || i == '.' || i == '_' ? i : (i == ' ') ? '+' : 0;
  }
  x=1;
}

/*
 * a very conservative method
 */
static size_t encoded_size(size_t t)
{
  return t*3 +1;
}

static void url_encode(const char *s, char *enc) {
  url_encoder_rfc_tables_init();
  char * table = html5;
  for (; *s; s++) {
    if (table[*s]) sprintf(enc, "%c", table[*s]);
    else sprintf(enc, "%%%02X", *s);
    while (*++enc);
  }
  return;
}

static void urldecode2(char *dst, const char *src)
{
  char a, b;
  while (*src) {
    if ((*src == '%') &&
	((a = src[1]) && (b = src[2])) &&
	(isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
	a -= 'a'-'A';
      if (a >= 'A')
	a -= ('A' - 10);
      else
	a -= '0';
      if (b >= 'a')
	b -= 'a'-'A';
      if (b >= 'A')
	b -= ('A' - 10);
      else
	b -= '0';
      *dst++ = 16*a+b;
      src+=3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}
#endif // URL_ENCODE_H
