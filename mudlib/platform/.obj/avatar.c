/**
 * Avatar object for the platform zone. This will be the super-avatar to all
 * flavor avatars.
 *
 * @author devo@eotl
 * @alias PlatformAvatar
 */

inherit AvatarMixin;
inherit CommandGiverMixin;

void descend_signal(string session_id, object login) {
  // look up default player (one player allowed)
  // determine start room from last session
  // find/clone start room
  // determine flavor
  // clone flavor avatar
  // avatar->try_descend
  // ::descend_signal
  // avatar->descend_signal
  //   move to start room
  //   restore inventory 
  //   `sense here`  
}
