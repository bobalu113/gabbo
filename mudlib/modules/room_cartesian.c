/**
 * A base object for rooms located by Cartesian coordinates.
 * 
 * @author devo@eotl
 * @alias CartesianRoomCode
 */

inherit RoomCode;

default private variables;

int *coordinates;

default public functions;

/**
 * Get the room this exit is to move the user into, loading/cloning it from 
 * exit_dest if necessary. This will return 0 only if the blueprint couldn't 
 * load or the new room could otherwise not be cloned.
 * 
 * @param  dir the direction of the exit
 * @return     the cloned destination room, or 0 if the room could not be 
 *             created
 */
object load_exit_room(string dir) {
  // room is already loaded, return it
  if (objectp(exits[dir, EXIT_ROOM]) {
    return exits[dir, EXIT_ROOM];
  }
  // null destination, return 0
  string dest = exits[dir, EXIT_DEST]; 
  if (!stringp(dest)) {
    return 0;
  }
  // first look for the object named dest
  object room = find_object(dest);
  // look for a previously cloned room with a valid back exit
  if (!objectp(room)) {
    string backdir = GRID_DIRS[dir];
    object *clones = clones(dest);
    foreach (object *clone : clones) {
      if (stringp(backdir)) {
        // normal exit
        object backroom = clone->query_exit_room(backdir);
        if (objectp(backroom)) {
          if (backroom == THISO) {
            room = clone;
            break;
          }
        } else {
          string backdest = clone->query_exit_dest(backdir);
          if ((backdest == load_name()) || (backdest == object_name())) {
            room = clone;
            clone->set_exit_room(THISO);
            break;
          }
        }
      } else {
        // zone exit
        foreach (string backdir, 
                 string backdest, 
                 int backflags, 
                 object backroom : clone->query_exits()) {
          if (backroom == THISO) {
            room = clone;
            break;
          } else if (!objectp(backroom)) {
            if ((backdest == load_name()) || (backdest == object_name())) {
              room = clone;
              clone->set_exit_room(THISO);
              break;
            }
          }
        }
        if (objectp(room)) {
          break;
        }
      }
    }
  }
  // try loading a new room
  if (!objectp(room)) {
    string ret = catch(room = load_object(dest); publish);
  } 
  // it wouldn't load or dest refered to a clone
  if (!objectp(room)) {
    return 0;
  }
  // we have a room, but it's a blueprint
  if (!clonep(room)) {
    string ret = catch(room = clone_object(dest); publish);
  }
  // cloning the blueprint failed
  if (!objectp(room)) {
    return 0;
  }

  return room;
}

/**
 * Return the x, y, and z coordinates of this room within its containing 
 * zone.
 * 
 * @return an int array of room coordinates, or 0 if coordinates are unknown
 */
int *query_coordinates() {
  return coordinates;
}

/**
 * Set the x, y and z coordinates of this room within its containing zone.
 * 
 * @param  coords an int array of room coordinates, or 0 of coordinates are
 *                unknown
 * @return        0 for failure, 1 for success
 */
int set_coordinates(int *coords) {
  coordinates = coords;
  return 1;
}

/**
 * Get the x coorindate of this room within its containing zone.
 * 
 * @return the x coordinate
 */
int query_x_coordinate() {
  return coordinates[X_COORD];
}

/**
 * Set the x coordinate of this room within its containing zone.
 * 
 * @param  coords the x coordinate
 * @return        0 for failure, 1 for success
 */
int set_x_coordinate(int coord) {
  coordinates[X_COORD] = coord;
  return 1;
}

/**
 * Get the y coorindate of this room within its containing zone.
 * 
 * @return the y coordinate
 */
int query_y_coordinate() {
  return coordinates[Y_COORD];
}

/**
 * Set the y coordinate of this room within its containing zone.
 * 
 * @param  coords the y coordinate
 * @return        0 for failure, 1 for success
 */
int set_y_coordinate(int coord) {
  coordinates[Y_COORD] = coord;
  return 1;
}

/**
 * Get the z coorindate of this room within its containing zone.
 * 
 * @return the z coordinate
 */
int query_z_coordinate() {
  return coordinates[Z_COORD];
}

/**
 * Set the z coordinate of this room within its containing zone.
 * 
 * @param  coords the z coordinate
 * @return        0 for failure, 1 for success
 */
int set_z_coordinate(int coord) {
  coordinates[Z_COORD] = coord;
  return 1;
}
