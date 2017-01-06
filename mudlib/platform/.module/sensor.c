/**
 * The basic unit of interaction in the game is a "message," the SensorMixin
 * provides objects the capability to "sense" those messages. Without it, any
 * output or messages an object may receive are simply regarded as noise.
 *
 * @author devo@eotl
 * @alias SensorMixin
 */
#include <capabilities.h>
#include <command_giver.h>
#include <message.h>
#include <sensor.h>

inherit JSONLib;
inherit MessageLib;
inherit ObjectLib;

private mapping CAPABILITIES_VAR = ([ CAP_SENSOR ]);
private mapping CMD_IMPORTS_VAR = ([ ]);

protected void setup();
mixed *try_message(string topic, string message, mapping context, 
                   object sender);
struct Message render_message(string topic, string message, mapping context, 
                              object sender);
void on_message(struct Message msg);
string query_terminal_type();

/**
 * Setup the SensorMixin.
 */
protected void setup() {
}

/**
 * Invoked by PostalService before a message is sent to prevent or otherwise
 * prepare the message for delivery.
 * 
 * @param  topic         the message topic
 * @param  message       the message
 * @param  context       the message context
 * @param  sender        the sending object, or for anonymous messages
 * @return extra args to be passed to on_message() after delivery
 */
mixed *try_message(string topic, string message, mapping context, 
                   object sender) {
  return ({ });
}

/**
 * Render an incoming message. The SensorMixin's implementation of 
 * render_message() is to dispatch the actual rendering to a shared renderer 
 * object based on topic and terminal type of this sensor.
 * 
 * @param  topic         the message topic
 * @param  message       the message
 * @param  context       the message context
 * @param  sender        the sending object, or for anonymous messages
 * @return a new Message struct representing the rendered message info
 */
struct Message render_message(string topic, string message, mapping context, 
                              object sender) {
  object logger = LoggerFactory->get_logger(THISO);
  struct Message result = (<Message>
    topic: topic,
    message: message,
    context: context,
  );
  string term = query_terminal_type();
  object renderer = TopicTracker->get_renderer(topic, term);
  if (renderer) {
    result->message = (renderer->render(term, topic, message, context, 
                                        sender));
  } else {
    logger->warn("No renderer found for topic %O, term %O", topic, term);
  }
  return result;
}

/**
 * This function is invoked by PostalService after a message has been rendered
 * and delivered.
 * 
 * @param  msg  the rendered message
 */
void on_message(struct Message msg) {
  return;

/**
 * Get the terminal type of this sensor. Returns a reasonable default at the
 * base implementation, but my be overridden by avatars to get from connection
 * info instead.
 * 
 * @return the default terminal type
 */
string query_terminal_type() {
  return DEFAULT_TERMINAL_TYPE;
}