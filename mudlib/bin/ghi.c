#include <ansi.h>

inherit CommandCode;

private variables private functions inherit ArgsLib;
private variables private functions inherit StringsLib;
private variables private functions inherit GetoptsLib;

int do_help(string command);
int do_pullreq(mixed *args);
void print_request_list(mapping *reqList, object who);
void print_request(mapping req, object who);
void print_request_diff(mapping req, object who);

int do_command(string arg) {
  // TODO add confirmation
  mixed *args = explode_args(arg);
  string cmd = args[0];
  switch (cmd) {
    case "help":
      return do_help(sizeof(args) > 1 ? args[1] : 0);
    case "pullreq":
      args = getopts(args[1..], "hld:");
      return do_pullreq(args);
    default:
      printf("%s: %s: invalid command\n", query_verb(), cmd);
      return 1;
  }

  return 1;
}

int do_help(string command) {
  switch (command) {
    case "pullreq":
      printf(
        "NAME\n"
        "    %s pullreq - display and manipulate pull requests\n"
        "SYNOPSIS\n"
        "    %s pullreq [-lh] <id>\n"
        "DESCRIPTION\n"
        "    Display pull request information for request number <id>.\n"
        "    Options are as follows:\n"
        "      -l : list all open pull requests\n"
        "      -h : this help screen\n",
        query_verb(), query_verb()
      );
      break;
    default:
      printf(
        "usage: %s <command> [<args>]\n\n"
        "The currently available github interface commands are:\n"
        "  help       get help on commands\n"
        "  pullreq    display and manipulate pull requests\n",
        query_verb());
      break;
  }

  return 1;
}

int do_pullreq(mixed *args) {
  if (member(args[1], 'l')) {
    GitHubServer->get_pull_requests(#'print_request_list, THISP); //'
    return 1;
  } else if (!sizeof(args[0])) {
    return do_help("pullreq");
  } else {
    foreach (string arg : args[0]) {
      int number = to_int(arg);
      if (!number) {
        printf("Invalid pull request number: %s\n", arg);
        return 1;
      }

      if (member(args[1], 'd')) {
        int file = to_int(args[1]['d']);
        GitHubServer->get_pull_request_diff(number,
                                            (file ? file - 1 : args[1]['d']),
                                            #'print_request_diff, THISP); //'
      } else {
        GitHubServer->get_pull_request(number, #'print_request, THISP); //'
      }
      return 1;
    }
  }
}

void print_request_list(mapping *reqList, object who) {
  string out = sprintf(
    "Listing %s pull requests. Total: %d\n\n",
    "open", sizeof(reqList)
  );
  foreach (mapping req : reqList) {
    out += sprintf(
      "%4d %s [%s %s]\n",
      req["number"],
      crop_string(req["title"], 35),
      req["user"],
      strftime("%Y-%m-%d %T", req["created_at"])
    );
  }
  tell_object(who, out);
  return;
}

void print_request(mapping req, object who) {
  int i;
  string files = "";
  i = 0;
  foreach (string file : req["files"]) {
    files += sprintf("%2d) %s\n", ++i, file);
  }
  string comments = "";
  i = 0;
  foreach (mapping comment : req["comments"]) {
    comments += sprintf("%2d) %s [%s]\n    %s\n\n",
                        ++i,
                        comment["user"],
                        strftime("%Y-%m-%d %T",
                                 comment["updated_at"]),
                        implode(explode(comment["body"], "\n"),
                                "\n    "));
  }
  string out = sprintf(
    "Title: %s\n"
    "Description:\n"
    "%-=76s\n\n"
    " Number: %-30d  Commits: %d\n"
    " Status: %-30s Addtions: %d\n"
    "   User: %-30sDeletions: %d\n"
    "Created: %-30s   Closed: %s\n"
    "\nFiles:\n"
    "%s"
    "\nComments:\n"
    "%s",
    req["title"], req["body"], req["number"], req["commits"],
    req["state"], req["additions"], req["user"], req["deletions"],
    strftime("%Y-%m-%d %T", req["created_at"]),
    (req["closed_at"] ?
     strftime("%Y-%m-%d %T", req["closed_at"]) :
     "N/A"
    ), files, comments);
  tell_object(who, out);
  return;
}

void print_request_diff(mapping req, object who) {
  string out = sprintf(
    "Title: %s\nFile: %2d) %s\n",
    req["title"], req["file"] + 1, req["path"]
  );
  for (int i = 0, int j = sizeof(req["diff"]); i < j; i++) {
    string line = req["diff"][i];
    string is = to_string(i - 4);
    if (member(req["comments"], to_string(is))) {
      int tag = line[0];
      out += sprintf("%s%c%s%s%s\n",
                     (tag == '+' ? BOLD_GREEN : tag == '-' ? BOLD_RED : ""),
                     tag, BOLD_YELLOW, line[1..], NORM);
      foreach (mapping comment : req["comments"][is]) {
        string prefix = sprintf("%s [%s] ",
                                comment["user"],
                                strftime("%Y-%m-%d %T",
                                         comment["updated_at"]));
        out += sprintf("%s%s%-=*s%s\n",
                       BOLD_YELLOW,
                       prefix,
                       76 - strlen(prefix),
                       comment["body"],
                       NORM);
      }
    } else {
      if (line[0] == '+') {
        out += sprintf("%s%s%s\n", BOLD_GREEN, line, NORM);
      } else if (line[0] == '-') {
        out += sprintf("%s%s%s\n", BOLD_RED, line, NORM);
      } else if (line[0] == '@') {
        out += sprintf("%s%s%s\n", CYAN, line, NORM);
      } else {
        out += line + "\n";
      }
    }
  }
  tell_object(who, out);
}