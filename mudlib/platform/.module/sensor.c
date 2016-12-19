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

inherit JSONLib;
inherit MessageLib;
inherit ObjectLib;

private mapping CAPABILITIES_VAR = ([ CAP_SENSOR ]);
private mapping CMD_IMPORTS_VAR = ([ ]);

int prevent_message(string topic, string message, mapping context, 
                    object sender) {
  return 0;
}

struct Message render_message(string topic, string message, mapping context, 
                              object sender) {
  object logger = LoggerFactory->get_logger(THISO);
  struct Message result = (<Message>);
  result->topic = topic;
  result->message = message;
  result->context = context;
  string term = 0;
  if (is_capable(THISO, CAP_AVATAR)) {
    term = THISO->query_terminal_type();
  }
  object renderer = TopicTracker->get_renderer(topic, term);
  if (renderer) {
    result->message = (renderer->render(term, topic, message, context, 
                                        sender));
  } else {
    logger->warn("No renderer found for topic %O, term %O", topic, term);
  }
  return result;
}

void message_signal(struct Message msg) {
  return;
}

/**
 * Initialize SensorMixin.
 */
protected void setup() {
}
