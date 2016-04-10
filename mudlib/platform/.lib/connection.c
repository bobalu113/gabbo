/**
 * Utility library for tracking connections.
 *
 * @author devo@eotl
 * @alias ConnectionLib
 */

struct ConnectionConfig {
  string id;
  string terminal;
  int terminal_width, terminal_height;
  int *ttyloc;
  int connect_time;
};

