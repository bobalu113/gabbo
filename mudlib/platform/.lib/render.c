/**
 * Utility library for rendering messages to the screen.
 *
 * @author devo@eotl
 * @alias RenderLib
 */

private variables private functions inherit JSONLib;

string render_json(string topic, mapping msgdata, object sender) {
  return json_encode(([
    "topic" : topic,
    "data" : msgdata,
  ]));
}
