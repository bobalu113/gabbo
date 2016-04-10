/**
 * Utility library for composing and rendering output to interactive users, 
 * connected via web and telnet.
 *
 * @author devo@eotl
 * @alias MessageLib
 */

#include <message.h>

struct Message {
  string topic;
  mapping data;
  string message;
};


struct Message system_msg(object ob, string topic, mapping msgdata) {  
  return PostalService->send_message(ob, topic, msgdata);
}
