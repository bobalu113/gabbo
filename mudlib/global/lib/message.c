/**
 * Utility library for composing and rendering output to interactive users, 
 * connected via web and telnet.
 *
 * @author devo@eotl
 * @alias MessageLib
 */

struct MessageResult {
  int muffled;
  string msg;
  mapping msgdata;
};

struct MessageResult tell_object(object ob, string msg, mapping msgdata) {
  return MessageService->send_message(object ob, string msg, mapping msgdata, 
                                      THISO);
}

mapping tell_objects(object *ob, string msg, mapping msgdata) {
  return mkmapping(ob, map(ob, (: tell_object($1, $2, $3) :), msg, msgdata));
}

void render_message() {

}
