/**
 * Utility library for manipulating structs.
 *
 * @author devo@eotl
 * @alias StructLib
 */

private nosave mapping setters = ([ ]);
private nosave mapping getters = ([ ]);

mixed get_struct_member(mixed strct, string member) {
  if (!member(getters, member)) {
    getters[member] = lambda(({ 'struct }), 
      ({ #'->, 'struct, member })
    ); //'
  }
  return funcall(getters[member], strct);
}

void set_struct_member(mixed strct, string member, mixed val) {
  if (!member(setters, member)) {
    setters[member] = lambda(({ 'struct, 'val }), 
      ({ #'=, ({ #'->, 'struct, member }), 'val })
    ); //'
  }
  funcall(setters[member], strct, val);
}
