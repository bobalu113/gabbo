/**
 * The FlavorTracker maintains the game's flavor hierarchy.
 *
 * @author devo@eotl
 * @alias FlavorTracker
 */

#define DefaultFlavor   "gabbo-basic"
#define FlavorAvatar    "/.obj/avatar"

string query_avatar(string flavor, string player_id) {
  return FlavorDir "/" + flavor + FlavorAvatar;
}
