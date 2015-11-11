

struct MessageResult send_message(object target, string msg, mapping msgdata, object sender){
  if (!is_capable(who, CAP_SENSOR)) {
    return 0;
  }
  struct MessageResult result = who->recv_message(msg, msgdata, sender);
  if (1) {
    efun::tell_object(msg);
  }
  return result;
}
