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
#include <sensor.h>

struct Message {
  string topic;
  string message;
  mapping context;
};

varargs struct Message send_msg(string message, mapping context, object ob, 
                                string topic) {  
  if (!message) {
    return 0;
  }
  if (!ob) {
    ob = THISP;
  }
  if (!topic) {
    topic = TopicTracker->get_topic(THISO);
  }
  return PostalService->send_message(ob, topic, message, context);
}

varargs struct Message stdout_msg(string message, mapping context, object ob,
                                  string topic) {
  context ||= ([ ]);
  context[MSGCTX_SENSE] |= SENSE_EXTRA;
  context[MSGCTX_EXTRA_SENSE] ||= ([ ]);
  context[MSGCTX_EXTRA_SENSE] += ([ SENSE_STDOUT ]);
  string trimmed = trim(message, TRIM_BOTH, " \t\n\r");
  if (strlen(trimmed)) {
    return send_msg(message, context, ob, topic);
  }
  return 0;
}

varargs struct Message stderr_msg(string message, mapping context, object ob,
                                  string topic) {
  context ||= ([ ]);
  context[MSGCTX_SENSE] |= SENSE_EXTRA;
  context[MSGCTX_EXTRA_SENSE] ||= ([ ]);
  context[MSGCTX_EXTRA_SENSE] += ([ SENSE_STDERR ]);
  string trimmed = trim(message, TRIM_BOTH, " \t\n\r");
  if (strlen(trimmed)) {
    return send_msg(message, context, ob, topic);
  }
  return 0;
}

varargs struct Message prompt_msg(string message, mapping context, object ob) {
 return send_msg(message, context, ob, TOPIC_PROMPT);
}

struct Message send_prompt(object who) {
  mixed prompt = set_prompt(0, who);
  if (closurep(prompt)) {
    return prompt_msg(funcall(prompt), ([ ]), who);
  } else {
    return prompt_msg(prompt, ([ ]), who);
  }
}

