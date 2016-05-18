/**
 * The postal service object. It is responsible for sending messages back and
 * forth between objects.
 *
 * @author devo@eotl
 * @alias PostalService
 */

#include <message.h>
#include <capabilities.h>

private variables private functions inherit MessageLib;
private variables private functions inherit ObjectLib;

varargs struct Message send_message(object target, string topic, string message,
                                    mapping context, object sender) {
  if (!mappingp(context)) {
    context = ([ ]);
  }
  if (!is_capable(target, CAP_SENSOR)) {
    return 0;
  }
  if (sender && (sender != previous_object())) {
    raise_error("not allowed to spoof\n");
    return 0;
  }
  if (target->prevent_message(topic, message, context, sender)) {
    return 0;
  }

  struct Message msg = target->render_message(topic, message, context, sender);
  efun::tell_object(target, msg->message);
  target->message_signal(msg);
  return msg;
}
