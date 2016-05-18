/**
 * The default renderer.
 *
 * @author devo@eotl
 * @alias DefaultRenderer
 */
 
private variables private functions inherit RenderLib;

string render_default(string term, string topic, string message, 
                      mapping context, object sender);

string render(string term, string topic, string message, mapping context, 
              object sender) {
  return render_default(term, topic, message, context, sender);
}

string render_default(string term, string topic, string message, 
                      mapping context, object sender) {
  if (term == "gabbo") {
    return render_json(topic, message, context, sender);
  }
  return message;
}
