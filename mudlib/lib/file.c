/**
 * Utility library for dealing with the filesystem.
 * 
 * @author devo@eotl
 * @alias FileLib
 */
#include <sys/files.h>

private string *expand_files(string *path, string *dirs);
private mixed *collate_files(string dir, string pattern);

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
  // TODO finalize how paths get munged
  if (filename[<1] == '/') { 
    filename = filename[0..<2];
  }
  if (!strlen(filename)) { return 0; }
  string *path = explode(filename, "/");

  return implode(path[0..<2], "/");
}

/**
 * Returns the absolute filename of the possibly relative filename described 
 * by pattern.  Relative paths will be resolved according to 
 * who->query_pwd(), or dirname(object_name(who)) if 'who' does not define 
 * query_pwd().  'who' defaults to this_object() if not specified.  Home
 * directories specified with '~' will also be resolved according to 'who'.
 * 
 * @param  pattern path to be expanded
 * @param  who     optional object from which relative paths should be 
 *                 resolved
 * @return         the absolute path
 */
varargs string expand_path(string pattern, object who) {
  if (!objectp(who)) {
    who = THISO;
  }
  string cwd = who->query_cwd() || dirname(object_name(who));

  if (!stringp(pattern) || !strlen(pattern)) {
    pattern = cwd;
  }

  // first make pattern absolute
  switch (pattern[0]) {
    case '/':   /* already absolute */
      break;
    case '~':   /* home dir */
      string name = who->query_username();
      if (pattern == "~") {
        if (name) {
          pattern = HomeDir + "/" + name;
        } else {
          pattern = HomeDir;
        }
      } else if (pattern[1] == '/') {
        if (name) {
          pattern = sprintf("%s/%s%s", HomeDir, name, pattern[1..]);
        } else {
          pattern = HomeDir + pattern[1..];
        }
      } else {
        pattern = HomeDir + "/" + pattern[1..];
      }
      break;
    default:
      pattern = cwd + "/" + pattern;
      break;
  }

  // expand . and ..
  string *parts = explode(pattern, "/");
  string *path = ({ });
  foreach (string part : parts) {
    switch (part) {
      case ".."  : /* walk back one dir */
        path = path[0..<2];
        break;
      case "."   : /* skip */
        break;
      default    :
        path += ({ part });
        break;
    }
  }
  
  // put it all back together
  if (!sizeof(path)) {
    pattern =  "/";
  } else {
    pattern = implode(path, "/");
    if (!strlen(pattern)) {
      pattern = "/";
    }
  }

  return pattern;
}

/**
 * Resolve a file pattern containing possible wildcards. Wildcards may be
 * expressed as a '*' or '?' as specified by get_dir(E). Wildcard characters
 * in the middle of the path will be expanded in place (e.g.
 * "home/*&#47;workroom.c" expands to all workroom files).
 * 
 * @param  pattern the file pattern to expand
 * @param  who     optional object from which relative paths should be 
 *                 resolved, defaults to THISO
 * @return         a list of all matching files (constrained by valid_read)
 */
varargs mixed *expand_pattern(string pattern, object who) {
  if (!objectp(who)) {
    who = THISO;
  }
  pattern = expand_path(pattern, who);
  if (pattern[<1] == '/') {
    pattern += "*";
  }
  return expand_files(explode(pattern, "/")[1..], 
                      ({ ({ 0, "", 0, 0, 0 }) }));
}

/**
 * Recursive function to resolve wildcards in the middle of a path.
 * 
 * @param  path the path to resolve, split by '/'
 * @param  dirs a running array of directories to inspect for matching files
 * @return      the list of all matching files
 */
private mixed *expand_files(string *path, mixed *dirs) {
  // FUTURE add '**' support
  mixed *result = ({ });
  string pattern = "/" + path[0];

  object logger = LoggerFactory->get_logger(THISO);
  logger->trace("pattern: %O", pattern);
  logger->trace("path: %O", path);
  if (sizeof(path) > 1) {
    foreach (mixed *dir : dirs) {
      mixed *alist = collate_files(dir[1], pattern);
      if (!alist) { continue; }
      alist = order_alist(alist[1], alist[0], alist[2], alist[3], alist[4]);
      int dir_index = rmember(alist[0], -2);
      alist[0] = alist[0][0..dir_index];
      alist[1] = alist[1][0..dir_index];
      alist[2] = alist[2][0..dir_index];
      alist[3] = alist[3][0..dir_index];
      alist[4] = alist[4][0..dir_index];
      result += transpose_array(alist);
    }
    logger->trace("result: %O", result);
    return expand_files(path[1..], result);
  } else {
    foreach (mixed *dir : dirs) {
      mixed *alist = collate_files(dir[1], pattern);
      if (!alist) { continue; }
      result += transpose_array(alist);
    }
    logger->trace("result: %O", result);
    return result;
  }
}

/**
 * Collate all file information for the given directory and file pattern.
 * Returns an alist for the form:
 * <pre><code>
 * ({ names, sizes, modified_dates, accessed_dates, modes })
 * </code></pre>
 * 
 * @param  dir     the parent directory of the files being collated
 * @param  pattern the file pattern, may contain wildcards
 * @return         an alist of the target directory's contents
 */
private mixed *collate_files(string dir, string pattern) {
  object logger = LoggerFactory->get_logger(THISO);
  logger->trace("dir: %O", dir);
  logger->trace("pattern: %O", pattern);
  mixed *contents = get_dir(dir + pattern, GETDIR_ALL
                                          |GETDIR_PATH
                                          |GETDIR_UNSORTED);
  int size = sizeof(contents);
  if (!size) { return 0; }

  int step = 5;
  mixed *names = allocate((size / step));
  mixed *sizes = allocate((size / step));
  mixed *modified = allocate((size / step));
  mixed *accessed = allocate((size / step));
  mixed *modes = allocate((size / step));
  int pos = 0;
  for (int i = 0; i < size; i += 5) {
    // XXX workaround a driver bug that sometimes puts nulls at the end of
    // filenames
    if (!contents[i][<1]) {
      contents[i] = contents[i][0..<2];
    }
    if (contents[i][<3..<1] == "/..") { continue; }
    if (contents[i][<2..<1] == "/.") { continue; }
    names[pos] = contents[i];
    sizes[pos] = contents[i+1];
    modified[pos] = contents[i+2];
    accessed[pos] = contents[i+3];
    modes[pos] = contents[i+4];
    pos++;
  }
  names[pos..] = ({ });
  sizes[pos..] = ({ });
  modified[pos..] = ({ });
  accessed[pos..] = ({ });
  modes[pos..] = ({ });
  return ({ names, sizes, modified, accessed, modes });
}

/**
 * Return true if a file can be loaded (ends in .c basically).
 * 
 * @param  file the filename
 * @return      1 if file can be loaded, otherwise 0
 */
int is_loadable(string file) {
  return ((strlen(file) > 2) || (file[<2..<1] == ".c"));
}
