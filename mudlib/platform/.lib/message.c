/**
 * Utility library for composing and rendering output to interactive users, 
 * connected via web and telnet.
 *
 * @author devo@eotl
 * @alias MessageLib
 */

#include <sys/strings.h>
#include <message.h>
#include <topic.h>

struct Message {
  string topic;
  string message;
  mapping context;
};

varargs struct Message system_msg(object ob, string message, 
                                  mapping context, string topic) {  
  return PostalService->send_message(ob, topic, message, context);
}

varargs struct Message fail_msg(string message, mapping context,
                                string topic) {
  if (!topic) {
    topic = TopicTracker->get_controller_topic(THISO);
  }
  string trimmed = trim(message, TRIM_BOTH, " \t\n\r");
  if (strlen(trimmed)) {
    return PostalService->send_message(THISP, topic, message, context);
  }
  return 0;
}

varargs struct Message prompt_msg(string message, mapping context) {
 return PostalService->send_message(THISP, TOPIC_PROMPT, message, context);
}
