/**
 * Utility library for manipulating arrays.
 * 
 * @author devo@eotl
 * @alias ArrayLib
 */

inherit "/lib/doctest";
inherit "/lib/file";
inherit "/lib/user";

varargs int anti_searcha(mixed list, mixed el, int start, int step) {
  int size = sizeof(list);
  step ||= 1;

  if (step > 0) {
    for (; (start < size) && (list[start] == el); start += step);
    return (start < size) ? start : -1;
  } else {
    for (; (start >= 0) && (list[start] == el); start += step);
    return (start >= 0)   ? start : -1;
  }
}

varargs int searcha_any(mixed list, mixed elements, int start, int step) {
  int size = sizeof(list);
  step ||= 1;

  for (; (start < size); start += step) {
    if (member(elements, list[start]) != -1) {
      return start;
    }
  }
  
  return -1;
}

varargs int anti_searcha_any(mixed list, mixed elements, int start, 
                             int step) {
  int size = sizeof(list);
  step ||= 1;
 
  for (; (start < size); start += step) {
    if (member(elements, list[start]) == -1) {
      return start;
    }
  }
  
  return -1;
}

