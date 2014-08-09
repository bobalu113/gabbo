/**
 * Utility library for manipulating arrays.
 * 
 * @author devo@eotl
 * @alias ArrayLib
 */

/**
 * Return the index of the first element of an array or string which is a 
 * specified value. Like member(), but provides the start/step capability.
 * 
 * @param  list  an array or string to search
 * @param  el    a value to search against
 * @param  start optional starting position of the search
 * @param  step  optional step
 * @return       the index of the first member of list which is el
 */
varargs int searcha(mixed list, mixed el, int start, int step) {
  int size = sizeof(list);
  step ||= 1;

  if (step > 0) {
    for (; (start < size) && (list[start] != el); start += step);
    return (start < size) ? start : -1;
  } else {
    for (; (start >= 0) && (list[start] != el); start += step);
    return (start >= 0)   ? start : -1;
  }
}

/**
 * Return the index of the first element of an array or string which is not
 * a specified value.
 * 
 * @param  list  an array or string to search
 * @param  el    a value to search against
 * @param  start optional starting position of the search
 * @param  step  optional step
 * @return       the index of the first member of list which is not el
 */
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

/**
 * Like member(), but instead of searching for a single item, search for any
 * item of a set.
 * 
 * @param  list     an array or string to search
 * @param  elements an array of potential elements, or a string or characters
 *                  to search against
 * @param  start    optional starting position of the search
 * @param  step     optional step
 * @return          the index of the first member of list which matches one
 *                  of the specified elements
 */
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

/**
 * Like anti_searcha(), but instead of searching for a single item, search 
 * for any item of a set.
 * @param  list     an array or string to search
 * @param  elements an array of potential elements, or a string of characters
 *                  to search against
 * @param  start    optional starting position of the search
 * @param  step     optional step
 * @return          the index of the first member of list which is not one of
 *                  the specified elements
 */
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
