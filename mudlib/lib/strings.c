inherit ArrayLib;

/*
      Faster than using AcmeArray->anti_searcha( str, ' ', start ),
      but yields the same results.
*/
varargs int find_nonws(string str, int start) {
  for (int size = strlen(str); 
       (start < size) && (str[start] == ' '); 
       start++);  
  return start;
}

#define UNQUOTE_CHARS "\"'"    /* default quote chars to remove */
#define QUOTE_CHARS "\"'"     /* default quote chars to match on */

int is_escaped(string str, int index) {
  /* It's complicated because backslashes can be escaped too. */
  int last_non_bs = anti_searcha(str, '\\', index-1, -1);
  return (index - last_non_bs + 1) % 2;
}
 
varargs int search_str_unescaped(string str, int el, int pos, int step) {
  step ||= 1;
 
  for (int size = strlen(str); (pos < size) && (pos >= 0); pos += step) {
    if ((str[pos] == el) && !is_escaped(str, pos)) {
      return pos;
    }
  }
  return -1;
}

varargs int match_quote(string str, int start, string quote_chars) {
  int end;
  
  if (member(quote_chars || QUOTE_CHARS, str[start]) != -1)
    end = search_str_unescaped(str, str[start], start + 1, 1);
  else
    end = search_str_unescaped(str, ' ', start + 1, 1) - 1;
  return (end < 0) ? strlen(str) - 1 : end;
}


/*
      This deals with literal backslashes only.  In the examples, and in
      the strings this function is meant to deal with "\a" is meant to 
      represent a string containing the two characters '\' and 'a',
      not a string containing a single '\a' character.)
*/
string unescape(string str) {
  string copy = "";

  for (int len = strlen(str), int i = 0, int next = 0; 
       i < len; 
       i = next + 2) {
    next = member(str, '\\', i);
    if (next == -1) { return copy + str[i..len-1]; }
    if (next == len - 1) {
      if(i == next) { return copy; }
      else { return copy + str[i..len-2]; }
    }
    copy += str[i..next-1] + str[next+1..next+1];
  }
  return copy;
}
 
/* Removes outer layer of quotes, if there is one */
varargs string unquote(string str, string quote_chars) {
  int len = strlen(str);
  
  // no room for quotes
  if (len < 2) { return str; }
  
  if (!quote_chars) {
    quote_chars = UNQUOTE_CHARS;
  }
 
  if (( (str[0] == '\"') || (str[0] == '\'') ) && (str[0] == str[len-1])) { 
    return (len == 2) ? "" : str[1..len-2]; //"
  }
  else {
    return str;
  }
}
 