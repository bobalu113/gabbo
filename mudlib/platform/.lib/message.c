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

/**
 * Send a message with the specified context. The context can hold anything,
 * but at the minimum should declare a "sense" that sensor objects can receive
 * the message using. Without a sense, the message may not be delivered.
 * 
 * @param  message       the message to send
 * @param  context       the message context
 * @param  ob            the target object
 * @param  topic         the message topic, optional; if not provided, a 
 *                       suitable default for the current object will be used
 * @return the sent message, or 0 if it could not be sent
 */
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
  return PostalService->send_message(ob, topic, message, context, THISO);
}

/**
 * Send a message over STDOUT. Unlike UNIX where STDOUT is a file handle, in
 * MessageLib and SensorMixin it is an "extra sense." Sensor objects are able 
 * to receive messages via extra senses in addition to the standard five, and 
 * stdout is one of these senses.
 * 
 * @param  message       the message to send
 * @param  context       the message context
 * @param  ob            the target object
 * @param  topic         the message topic, optional; if not provided, a 
 *                       suitable default for the current object will be used
 * @return the sent message, or 0 if it could not be sent
 */
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

/**
 * Send a message over STDERR. Unlike UNIX where STDERR is a file handle, in
 * MessageLib and SensorMixin it is an "extra sense." Sensor objects are able 
 * to receive messages via extra senses in addition to the standard five, and 
 * stderr is one of these senses.
 * 
 * @param  message       the message to send
 * @param  context       the message context
 * @param  ob            the target object
 * @param  topic         the message topic, optional; if not provided, a 
 *                       suitable default for the current object will be used
 * @return the sent message, or 0 if it could not be sent
 */
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

/**
 * Send a prompt message. The prompt is one of the few types of messages that
 * isn't "sensed" and can be delivered to any object even if it doesn't
 * implement the SensorMixin.
 * 
 * @param  {[type]} string  message       [description]
 * @param  {[type]} mapping context       [description]
 * @param  {[type]} object  ob            [description]
 * @return {[type]}         [description]
 */
varargs struct Message prompt_msg(string message, mapping context, object ob) {
 return send_msg(message, context, ob, TOPIC_PROMPT);
}

/**
 * Send a prompt string.
 * 
 * @param  who           the object to prompt
 * @return the prompt string that was sent
 */
string send_prompt(object who) {
  return PostalService->prompt_message(who, THISO);
}
