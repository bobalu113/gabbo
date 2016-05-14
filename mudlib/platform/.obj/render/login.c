/**
 * A renderer for the login object.
 *
 * @author devo@eotl
 * @alias LoginRenderer
 */

#include <ansi.h>
#include <topic.h>
 
private variables private functions inherit RenderLib;

string render_welcome(string term, string topic, mapping msgdata, 
                      object sender);
string render_default(string term, string topic, mapping msgdata, 
                      object sender);

string render(string term, string topic, mapping msgdata, object sender) {
  switch (topic) {
    case TOPIC_WELCOME:
      return render_welcome(term, topic, msgdata, sender);
    case TOPIC_LOGIN:
    default:
      return render_default(term, topic, msgdata, sender);
  }
  return "";
}

string render_welcome(string term, string topic, mapping msgdata, 
                      object sender) {
  if (term == "gabbo") {
    return render_json(topic, msgdata, sender);
  }
  string msg = sprintf("%s%s%s%s\n", 
    (msgdata["clearScreen"] ? CLEAR_SCREEN : ""), // TODO abstract ansi
    (msgdata["defaultTerm"] ? msgdata["defaultTerm"] + "\n" : ""),
    (msgdata["welcome"] ? msgdata["welcome"] : ""),
    (msgdata["insecure"] ? msgdata["insecure"] + "\n" : "")
  );
  return msg;
}

string render_default(string term, string topic, mapping msgdata, 
                      object sender) {
  if (term == "gabbo") {
    return render_json(topic, msgdata, sender);
  }
  return msgdata["message"];
}
