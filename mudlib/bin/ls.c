inherit CommandCode;

inherit ArgsLib;
inherit GetoptsLib;

#include <sys/files.h>

int do_command(string arg) {
  mixed *args = getopts(explode_args(arg), "lad");
  if (!sizeof(args[0])) {
    args[0] = ({ THISP->query_cwd() });
  }

  string result = "";
  int first = 1;
  foreach (string path : args[0]) {
    if (!first) {
      result += "\n";
    }
    first = 0;
    if (!file_exists(path)) {
      printf("%s: %s: no such file or directory\n", query_verb(), path);
      continue;
    }
    // XXX munge dirname?
    if (is_directory(path) && !member(args[1], 'd') && (path[<1] != '/')) {
      path = path + "/";
    }

    mixed *dir = get_dir(path, 
      GETDIR_NAMES|GETDIR_SIZES|GETDIR_DATES|GETDIR_UNSORTED); 
    string *out = allocate(sizeof(dir) / 3);
    int total = 0;
    for (int i = 0, int j = sizeof(dir); i < j; i += 3) {
      total += dir[i+1];
      if (member(args[1], 'l')) {
        out[i/3] = sprintf("%-9s  %10d  %17s  %s", 
          (dir[i+1] >= 0 ? "file" : 
            (dir[i+1] == -2 ? "directory" : "")),
          (dir[i+1] >= 0 ? dir[i+1] : 0),
          strftime("%b %e " + 
                   (localtime()[5] == localtime(dir[i+2])[5] ? " %R" : "%Y" ),
                   dir[i+2]),
          dir[i]
        );
      } else {
        out[i/3] = dir[i];
      }
    }

    if (member(args[1], 'l')) {
      result += sprintf("total %d\n%s\n", total / 1000, implode(out, "\n"));
    } else {
      // FUTURE implement variable page width
      result += sprintf("%#-80s\n", implode(out, "\n"));
    }
  }

  printf(result);
  return 1;
}
