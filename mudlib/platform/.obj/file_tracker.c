/**
 * A service object to track changes to files.
 *
 * @alias FileTracker
 */
#pragma no_clone

// ([ regexp : ({ callbacks, ... }) ])
mapping subscribers;

void create() {
  subscribers = ([ ]);
}

int subscribe(string regexp, closure callback) {
  if (!member(subscribers, regexp)) {
    subscribers[regexp] = ({ });
  }
  subscribers[regexp] += ({ callback });
  return 1;
}

void write_signal(string file, string func) {
  object logger = LoggerFactory->get_logger(THISO);
  if (object_name(previous_object()) == MasterObject) {
      string *f = ({ file });
      foreach (string regexp, closure *callbacks : subscribers) {
        if (sizeof(regexp(f, regexp))) {
          foreach (closure callback : callbacks) {
            funcall(callback, file, func);
          }
        }
      }
  } else {
    logger->info("unauthorized object %O invoking write_signal",
                 previous_object());
  }
  return;
}
