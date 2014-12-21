inherit CommandCode;

private variables private functions inherit ArgsLib;
private variables private functions inherit GetoptsLib;

int do_command(string arg) {
  mixed *args;
  args = getopts(explode_args(arg), "rgmlMLt-d-i-w-e-f-");
  
  // TODO finish me!

  return 1;
}

