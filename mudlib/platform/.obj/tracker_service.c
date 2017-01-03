/**
 * A service for initializing and reloading tracker objects.
 * 
 * @author devo@eotl
 * @alias TrackerService
 */
inherit ObjectLib;

/**
 * Load or reload a specified tracker object.
 * 
 * @param  tracker       the path to the tracker object to load
 * @return the loaded tracker object
 */
object reload_tracker(string tracker) {
  object result = FINDO(tracker);
  if (result) {
    result = reload_object(result);
  } else {
    result = load_object(tracker);
    result->init_tracker();
  }
  return result;
}

/**
 * Global tracker load sequence.
 */
void load_trackers() {
  reload_tracker(FileTracker);
  reload_tracker(DomainTracker);
  reload_tracker(ProgramTracker);
  reload_tracker(ObjectTracker);
  reload_tracker(ConnectionTracker);
  reload_tracker(SessionTracker);
  reload_tracker(ZoneTracker);
}

/**
 * Constructor. Load trackers.
 */
void create() {
  load_trackers();
}
