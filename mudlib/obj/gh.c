#include <sys/erq.h>
#include <sys/tls.h>
#include <sys/config.h>

// TODO better error recovery

//#define HOST ({ 74, 125, 227, 199 })
#define HOST ({ 127, 0, 0, 1 })
#define PORT 80

#define OUT_SIZE      0
#define OUT_MD5       1
#define OUT_BUFFER    2
#define OUT_CURSOR    3
#define OUT_SENDING   4

#define SIZE_WIDTH    4
#define MD5_WIDTH    32

void open();
void open_callback(int *data, int size);
void send_string(string str);
void flush_buffer();
void send_callback(int *data, int size);

int *ticket, sending;
int in_size;
string incoming, in_md5;
mixed *queue;

void create() {
  queue = ({ });
  sending = 0;
  string incoming = 0;
}

void open() {
  send_erq(ERQ_OPEN_TCP,
           HOST + ({ PORT / 0x100, PORT & 0xFF }),
           #'open_callback); // '
  return;
}

void open_callback(int *data, int size) {
  object logger = LoggerFactory->get_logger(THISO);
  logger->debug("got open callback: %d, %O", size, data);

  if (size >= 2) {
    switch (data[0]) {
      case ERQ_OK:
        ticket = data[1..];
        logger->debug("open succeeded");
        // TODO sanity check queue
        flush_buffer();
        break;
      case ERQ_STDOUT:
        // read size and checksum and create a new buffer
        if (!incoming) {
          incoming = "";
          in_size = 0;
          in_size += (data[1] & 0xFF) * 0x1000000;
          in_size += (data[2] & 0xFF) * 0x10000;
          in_size += (data[3] & 0xFF) * 0x100;
          in_size += (data[4] & 0xFF);
          in_md5 = to_string(data[(SIZE_WIDTH + 1)..
                                  (SIZE_WIDTH + MD5_WIDTH)]);
          incoming += to_string(data[(SIZE_WIDTH + MD5_WIDTH + 1)..]);
          // TODO log a warning if extra data comes in
          if (strlen(incoming) >= in_size) {
            string md5 = hash(TLS_HASH_MD5, incoming);
            if (in_md5 != md5) {
              logger->info("checksums differ: %O %O", in_md5, md5);
            } else {
              logger->info("got message: %O", incoming);
            }
          }
        }
        break;
      case ERQ_E_ARGLENGTH:
      case ERQ_E_NSLOTS:
      case ERQ_E_UNKNOWN:
      default:
        logger->debug("open failed: %O, %O", data[0], data[1]);
        break;
    }
  }
}

void send_string(string str) {
  // TODO add overflow check
  queue += ({ ({ strlen(str), hash(TLS_HASH_MD5, str), str, -1, 0 }) });
  flush_buffer();
}

void flush_buffer() {
  object logger = LoggerFactory->get_logger(THISO);
  if (find_call_out("flush_buffer") != -1) {
    logger->debug("unable to flush buffer, flush call_out pending");
    return;
  }

  if (!ticket) {
    logger->info("unable to flush buffer, no ticket");
    return;
  }

  if (sending) {
    logger->debug("unable to flush buffer, awaiting ERQ_SEND response");
  }

  if (!sizeof(queue)) {
    return;
  }

  int *data = ({ });
  mixed *out = queue[0];
  int cursor = out[OUT_CURSOR];
  if (cursor < 0) {
    // new message, send size and checksum
    data += ({ (out[OUT_SIZE] & 0xFFFFFFFF) / 0x1000000,
               (out[OUT_SIZE] & 0xFFFFFF) / 0x10000,
               (out[OUT_SIZE] & 0xFFFF) / 0x100,
               (out[OUT_SIZE] & 0xFF)
            });
    data += to_array(out[OUT_MD5])[0..(MD5_WIDTH - 1)];
    sending = ERQ_MAX_SEND - sizeof(ticket) - SIZE_WIDTH - MD5_WIDTH;
  } else {
    sending = ERQ_MAX_SEND - sizeof(ticket);
  }

  int len = strlen(out[OUT_BUFFER]);
  if ((cursor + sending) > len) {
    sending = len - cursor;
  }
  data += to_array(out[OUT_BUFFER][(cursor + 1)..(cursor + sending)]);
  send_erq(ERQ_SEND, ticket + data, #'send_callback); //'
  return;
}

void send_callback(int *data, int size) {
  object logger = LoggerFactory->get_logger(THISO);
  logger->debug("got send callback: %d, %O", size, data);
  if (!sizeof(queue)) {
    logger->info("queue vanished while awaiting response");
    return;
  }

  mixed *out = queue[0];
  switch (data[0]) {
    case ERQ_OK:
      out[OUT_CURSOR] += sending;
      break;
    case ERQ_E_WOULDBLOCK:
      sending = 0; // resend
      call_out("flush_buffer", 1);
      return;
    case ERQ_E_INCOMPLETE:
      out[OUT_CURSOR] += data[1];
      break;
    case ERQ_E_TICKET:
      // retry with a new socket
      out[OUT_CURSOR] = -1;
      open();
      return;
    case ERQ_E_PIPE:
    case ERQ_E_UNKNOWN:
    default:
      logger->debug("send failed: %O, %O", data[0], data[1]);
      break;
  }
  sending = 0;
  if (out[OUT_CURSOR] >= (out[OUT_SIZE] - 1)) {
    logger->info("finished sending %d length bytes", out[OUT_SIZE]);
    queue = queue[1..];
  }
  flush_buffer();
}