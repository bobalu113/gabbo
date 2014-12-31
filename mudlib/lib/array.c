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
 * 
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

/**
 * Return a copy of an array with all duplicates removed. When an element 
 * appears multiple times in the array, the first occurrence will be used for
 * its position in the new array, unless keep_last!=0, in which case the last
 * occurrence will be used.
 *
 * Note: If you don't care about preserving order, you probably want to forgo
 *       using this function in favor of the more efficient: 
 *       <code>m_indices(mkmapping(list))</code>
 * 
 * @param  list      the array from which to remove duplicates
 * @param  keep_last 0 to use first occurrence, otherwise use last
 * @return           a copy of the array with duplicates removed
 */
varargs mixed *unique_array(mixed *list, int keep_last) {
  mixed *out = ({ });
  mapping bag = ([ ]);
  if (keep_last) {
    for (int i = sizeof(list)- 1; i >= 0; i--) {
      if (!member(bag, list[i])) {
        out = ({ list[i] }) + out;
        m_add(bag, list[i]);
      }
    }
  } else {
    for (int i = 0, int j = sizeof(list); i < j; i++) {
      if (!member(bag, list[i])) {
        out += ({ list[i] });
        m_add(bag, list[i]);
      }
    }
  }
  return out;
}

/**
 * Flatten an array one level. e.g. turns: 
 *   <code>({ ({ a, b }), c, ({ d, ({ e }) }) })</code>
 * into:
 *   <code>({ a, b, c, d, ({ e }) })</code>
 *   
 * @param  list the array to flatten
 * @return      the flattened array
 */
mixed flatten_array1(mixed *list) {
  if (!pointerp(list)) {
    return ({ });
  }

  mixed *newlist = ({ });
  for (int i = 0, int j = sizeof(list); i < j; i++) {
    if (pointerp(list[i])) {
      newlist += list[i];
    } else {
      newlist += ({ list[i] });
    }
  }
  return newlist;
}

/**
 * Completely flatten an array. e.g. turns: 
 *   <code>({ ({ a, b }), c, ({ d, ({ e }) }) })</code>
 * into:
 *   <code>({ a, b, c, d, e, })</code>
 *   
 * @param  list the array to flatten
 * @return      the flattened array
 */
mixed flatten_array(mixed *list) {
  if (!pointerp(list)) {
    return ({ list });
  }

  mixed *newlist = ({ });
  for (int i = 0, int j = sizeof(list); i < j; i++) {
    newlist += flatten_array(list[i]);
  }
  return newlist;
}

/**
 * Return all the values associated with a mapping key in the form of an 
 * array. The resulting array will have the same width as the specified 
 * mapping.
 * 
 * @param  map the mapping to index
 * @param  key the mapping key
 * @return     an array of the mapping values for key, in order
 */
mixed *m_value(mapping map, string k) {
  if (!member(map, k)) {
    return 0;
  }
  int width = widthof(map);
  mixed *result = allocate(width);
  for (int i = 0; i < width; i++) {
    result[i] = map[k, i];
  }
  return result;
}

/**
 * Internal function to recurse through nested arrays of a lambda closure.
 * 
 * @param  arr array to rebuild
 * @return     the rebuilt array
 */
private mixed *_reconstruct(mixed *arr) {
  int size = sizeof(arr);
  mixed *result = allocate(size + 1);
  result[0] = #'({;
  for (int i = 0; i < size; i++) {
    if (pointerp(arr[i])) {
      result[i + 1] = _reconstruct(arr[i]);
    } else {
      result[i + 1] = arr[i];
    }
  }
  return result;
}

/**
 * This function will return a lambda closure that, upon execution, will 
 * return the array used to construct the same closure. The new closure will
 * take the specified args, which, when evaluated, will end up as constants
 * in the newly compiled closure. All other symbols found in the input array 
 * will appear in the new lambda closure unquoted one level.
 *
 * Example: 
 * <code><pre>
 * funcall(reconstruct_lambda(({ 'arg })), 
                              ({ #'+, 'arg, ''s })), "str")</pre></code>
 * returns:
 * <code><pre>({ #'+, "str", 's})</pre></code>
 * 
 * @param  args an array of arguments to the new lambda closure
 * @param  arr  the closure which is to be reconstructed
 * @return      a closure which, when executed, should return arr
 */
closure reconstruct_lambda(symbol *args, mixed *arr) {
  return lambda(args, _reconstruct(arr));
}
