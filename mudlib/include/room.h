#ifndef _ROOM_H
#define _ROOM_H

#define EXIT_HIDDEN       0x01
#define EXIT_BLOCKED      0x02
#define EXIT_MUFFLED      0x04
#define EXIT_NO_FOLLOW    0x08
#define EXIT_PEDESTRIAN   0x10

#define GRID_DIRS         ([ "north"     : "south",      \
                             "south"     : "north",      \
                             "east"      : "west",       \
                             "west"      : "east",       \
                             "northeast" : "southwest",  \
                             "southwest" : "northeast",  \
                             "southeast" : "northwest",  \
                             "northwest" : "southeast",  \
                             "up"        : "down",       \
                             "down"      : "up"          \
                          ])

#endif  // _ROOM_H
