#ifndef CHECK_EMAIL_ADDR_H
#define CHECK_EMAIL_ADDR_H

#define MAX_EMAIL_NAME  126
static int check_email_addr(char *EM_Addr) {
  int count = 0;
  int i = 0;
  char conv_buf[MAX_EMAIL_NAME];
  char *c, *domain;
  char *special_chars = "()<>@,;:\"[]";
  
  strcpy(conv_buf,EM_Addr);

  for(c = conv_buf; *c; c++) {
    /* if '"' and beginning or previous is a '.' or '"' */
    if (*c == 34 && (c == conv_buf || *(c - 1) == 46 || *(c - 1) == 34)) {
      while (*++c) {
	/* if '"' break, End of name */
	if (*c == 34)
	  break;
	/* if '' and ' ' */
	if (*c == 92 && (*++c == 32))
	  continue;
	/* if not between ' ' & '~' */
	if (*c <= 32 || *c > 127)
	  return 0;
      }
      /* if no more characters error */
      if (!*c++)
	return 0;
      /* found '@' */
      if (*c == 64)
	break;
      /* '.' required */
      if (*c != 46)
	return 0;
      continue;
    }
    if (*c == 64) {
      break;
    }
    /* make sure between ' ' && '~' */
    if (*c <= 32 || *c > 127) {
      return 0;
    }
    /* check special chars */
    if (strchr(special_chars, *c)) {
      return 0;
    }
  } /* end of for loop */
  /* found '@' */
  /* if at beginning or previous = '.' */
  if (c == conv_buf || *(c - 1) == 46)
    return 0;
  /* next we validate the domain portion */
  /* if the next character is NULL */
  /* need domain ! */
  if (!*(domain = ++c))
    return 0;
  do {
    /* if '.' */
    if (*c == 46) {
      /* if beginning or previous = '.' */
      if (c == domain || *(c - 1) == 46)
	return 0;
      /* count '.' need at least 1 */
      count++;
    }
    /* make sure between ' ' and '~' */
    if (*c <= 32 || *c >= 127)
      return 0;
    if (strchr(special_chars, *c))
      return 0;
  } while (*++c); /* while valid char */
  return (count >= 1); /* return true if more than 1 '.' */
}

#endif /* CHECK_EMAIL_ADDR_H */
