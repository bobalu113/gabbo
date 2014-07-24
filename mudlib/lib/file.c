/**
 * Utility library for dealing with the filesystem.
 * 
 * @author devo@eotl
 * @alias FileLib
 */

/**
 * Test whether a file exists.
 * 
 * @param  filename the filename to test
 * @return          1 if the file exists, otherwise 0
 */
int file_exists(string filename) {
  return file_size(filename) != -1;
}

/**
 * Test whether a path is a directory.
 * 
 * @param  filename the path to test
 * @return          1 if the path is a directory, otherwise 0
 */
int is_directory(string filename) {
  return file_size(filename) == -2;
}

/**
 * Return the filename component of a complete path.
 * 
 * @param  filename the path to check
 * @return          the base filename of the path
 */
string basename(string filename) {
  return explode(filename, "/")[<1];
}

/**
 * Return the name of a file's containing directory.
 * 
 * @param  filename the path of the file to check
 * @return          the name of the containing directory
 */
string dirname(string filename) {
  if (is_directory(filename)) {
    return filename;
  }
  return implode(explode(filename, "/")[0..<2], "/");
}
