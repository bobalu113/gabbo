/**
 * A renderer for the login object.
 *
 * @author devo@eotl
 * @alias LoginRenderer
 */

#include <ansi.h>
#include <topic.h>

inherit DefaultRenderer;
 
private variables private functions inherit RenderLib;

string render_welcome(string term, string topic, string message, 
                      mapping context, object sender);

string render(string term, string topic, string message, mapping context, 
              object sender) {
  switch (topic) {
    case TOPIC_WELCOME:
      return render_welcome(term, topic, message, context, sender);
    case TOPIC_LOGIN:
      return render_default(term, topic, message, context, sender);
    default:
      return DefaultRenderer::render(term, topic, message, context, sender);
  }
  return "";
}

string render_welcome(string term, string topic, string message, 
                      mapping context, object sender) {
  if (term == "gabbo") {
    return render_json(topic, message, context, sender);
  }
  message = sprintf("%s%s%s%s\n", 
    (context["clearScreen"] ? CLEAR_SCREEN : ""), // TODO abstract ansi
    (context["defaultTerm"] ? context["defaultTerm"] + "\n" : ""),
    (message),
    (context["insecure"] ? context["insecure"] + "\n" : "")
  );
  return message;
}
