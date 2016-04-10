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

struct Message send_message(object target, string topic, mapping msgdata, 
                            object sender) {
  if (!is_capable(target, CAP_SENSOR)) {
    return 0;
  }
  if (sender && (sender != previous_object())) {
    raise_error("not allowed to spoof\n");
    return 0;
  }
  if (target->prevent_message(topic, msgdata, sender)) {
    return 0;
  }

  struct Message msg = target->render_message(topic, msgdata, sender);
  efun::tell_object(target, msg->message);
  target->message_signal(msg);
  return msg;
}
