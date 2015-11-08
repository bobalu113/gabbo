/**
 * Support for items capable of communication by speech.
 *
 * @author devo@eotl
 * @alias SpeechMixin
 */

#define SpeechBinDir       BinDir "/speech"

void speak() {
  // tell_object(map)
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
    SpeechBinDir "/say",
    SpeechBinDir "/shout",
    SpeechBinDir "/whisper"
  });
}
