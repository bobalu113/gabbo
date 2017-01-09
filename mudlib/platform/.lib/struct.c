/**
 * Utility library for manipulating structs.
 *
 * @author devo@eotl
 * @alias StructLib
 */

private nosave mapping setters = ([ ]);
private nosave mapping getters = ([ ]);

/**
 * Get the value of a struct member.
 * 
 * @param  strct         the struct to index
 * @param  member        a string denoting which member to index
 * @return the result of strct->member
 */
mixed get_struct_member(mixed strct, string member) {
  if (!member(getters, member)) {
    getters[member] = lambda(({ 'struct }), 
      ({ #'->, 'struct, member })
    ); //'
  }
  return funcall(getters[member], strct);
}

/**
 * Set the value of a struct member.
 * 
 * @param  strct         the struct to index
 * @param  member        a string denoting which member to index
 * @param  val           the value to assign to strct->member
 */
void set_struct_member(mixed strct, string member, mixed val) {
  if (!member(setters, member)) {
    setters[member] = lambda(({ 'struct, 'val }), 
      ({ #'=, ({ #'->, 'struct, member }), 'val })
    ); //'
  }
  funcall(setters[member], strct, val);
}
