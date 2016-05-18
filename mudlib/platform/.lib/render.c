/**
 * Utility library for rendering messages to the screen.
 *
 * @author devo@eotl
 * @alias RenderLib
 */

private variables private functions inherit JSONLib;

string render_json(string topic, string message, mapping context, 
                   object sender) {
  return json_encode(({ message, context }));
}
