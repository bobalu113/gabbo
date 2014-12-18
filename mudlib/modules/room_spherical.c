/**
 * A base object for rooms located by Spherical coordinates.
 * 
 * @author devo@eotl
 * @alias SphericalRoomCode
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
    object *clones = clones(dest);
    foreach (object *clone : clones) {
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
            clone->set_exit_room(dir, THISO);
            break;
          }
        }
      }
      if (objectp(room)) {
        break;
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
 * Return the rho, theta, and phi coordinates of this room within its 
 * containing zone.
 * 
 * @return an int array of room coordinates, or 0 if coordinates are unknown
 */
int *query_coordinates() {
  return coordinates;
}

/**
 * Set the rho, theta and phi coordinates of this room within its containing 
 * zone.
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
 * Get the rho coorindate of this room within its containing zone.
 * 
 * @return the rho coordinate
 */
int query_rho_coordinate() {
  return coordinates[RHO_COORD];
}

/**
 * Set the rho coordinate of this room within its containing zone.
 * 
 * @param  coords the rho coordinate
 * @return        0 for failure, 1 for success
 */
int set_rho_coordinate(int coord) {
  coordinates[RHO_COORD] = coord;
  return 1;
}

/**
 * Get the rho coorindate of this room within its containing zone.
 * 
 * @return the rho coordinate
 */
int query_theta_coordinate() {
  return coordinates[THETA_COORD];
}

/**
 * Set the theta coordinate of this room within its containing zone.
 * 
 * @param  coords the theta coordinate
 * @return        0 for failure, 1 for success
 */
int set_theta_coordinate(int coord) {
  coordinates[THETA_COORD] = coord;
  return 1;
}

/**
 * Get the phi coorindate of this room within its containing zone.
 * 
 * @return the phi coordinate
 */
int query_phi_coordinate() {
  return coordinates[PHI_COORD];
}

/**
 * Set the phi coordinate of this room within its containing zone.
 * 
 * @param  coords the phi coordinate
 * @return        0 for failure, 1 for success
 */
int set_phi_coordinate(int coord) {
  coordinates[PHI_COORD] = coord;
  return 1;
}

