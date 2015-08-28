inherit CommandCode;

private variables private functions inherit ArgsLib;
private variables private functions inherit StringsLib;
private variables private functions inherit GetoptsLib;

int do_help(string command);
int do_pullreq(mixed *args);

int do_command(string arg) {
  // TODO add confirmation
  mixed *args = explode_args(arg);
  string cmd = args[0];
  switch (cmd) {
    case "help":
      return do_help(sizeof(args) > 1 ? args[1] : 0);
    case "pullreq":
      args = getopts(args[1..], "hl");
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
    GitHubServer->get_pull_requests((:
      string out = sprintf(
        "Listing %s pull requests. Total: %d\n\n",
        "open", sizeof($1)
      );
      foreach (mapping req : $1) {
        out += sprintf(
          "%4d %s [%s %s]\n",
          req["number"],
          crop_string(req["title"], 35),
          req["user"],
          strftime("%Y-%m-%d %T", req["created_at"])
        );
      }
      tell_object($2, out);
    :), THISP);
    return 1;
  } else if (sizeof(args[0])) {
    foreach (string arg : args[0]) {
      int number = to_int(arg);
      if (!number) {
        printf("Invalid pull request number: %s\n", arg);
      } else {
        GitHubServer->get_pull_request(number, (:
          int i;
          mapping req = $1;
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
          tell_object($2, out);
        :), THISP);
      }
      return 1;
    }
  } else {
    return do_help("pullreq");
  }
}

