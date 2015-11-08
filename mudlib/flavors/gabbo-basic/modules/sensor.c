/**
 * The basic unit of interaction in the game is a "message," the SensorMixin
 * provides objects the capability to "sense" those messages. Without it, any
 * output or messages an object may receive are simply regarded as noise.
 *
 * @author devo@eotl
 * @alias SensorMixin
 */

#define SensorBinDir        BinDir "/seensor"

inherit RenderMessageLib;

void catch_msg(mapping msg, object ob) {
  // efun::write(str) ?
}

/**
 * Initialize VisibleMixin.
 */
protected void setup_sensor() {
}

/**
 * Return the capabilities this mixin provides.
 *
 * @return the 'visible' capability
 */
public mapping query_capabilities() {
  return ([ CAP_SENSOR ]);
}

/**
 * Return the CommandCode commands that will be made available to
 * implementors of this mixin. This will be invoked by objects implementing
 * CommandGiverMixin for each inherited blueprint.
 *
 * @return an array of paths to CommandCode objects
 */
public string *query_command_imports(object command_giver) {
  return ({
    SensorBinDir "/look"
  });
}
