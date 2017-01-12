/**
 * Utility library for manipulating strings of english words and sentences.
 *
 * @author devo@eotl
 * @alias EnglishLib
 */

/**
 * Return the objective pronoun for a given object.
 *
 * @param  what the object
 * @return      "he, "she", or "it", depending on gender
 */
string objective(object what) {
  switch (what->query_gender()) {
    case "male": return "he";
    case "female": return "she";
  }
  return "it";
}
