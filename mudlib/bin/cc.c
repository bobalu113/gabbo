inherit CommandCode;

inherit ArgsLib;
inherit GetoptsLib;

#include <sys/commands.h>

#define CCDIR "/usr/" + getuid() + "/"
#define CCFILE CCDIR "cc"

#define USAGE "cc<id> [-msdrf] [-i<path>] [-p[<code>]] code"

mixed *query_actions() {
  return ({ ({ "cc", AA_SHORT }) });
}

int do_command(string arg) {
  // TODO design a formal usage API
  if (!arg || !strlen(arg)) { 
    notify_fail("Usage: " USAGE "\n"); 
    return 0; 
  }

  string label = query_verb(0)[strlen(query_verb(1))..];
  mixed *args = getopts(explode_args(arg), "msdrfoi:");
  
  arg = implode(args[0], " ");
  mapping opts = args[1];

  printf ("%O\n%O\n", label, args);
  return 1;
  /*
  out += "\ncc() {\n";
  out += implode(stuff[i..], " ");
  if(out[<1] != ';') out += ";";
  out += "\n}\n";
  if(format) out = format_cc(out);
  if(FINDO(file)) destruct(FINDO(file));
  if(exists(file)) rm(file);
  if( file_size( CCDIR ".cctemplate" ) > -1 )
      copy_file( CCDIR ".cctemplate", file );
  write_file(file, out);
  if( err = catch( call_other( file, "??" ) ) )
  {
    printf( "Compile-time error: %s", err );
    if( errmsg = get_error_file( this_object()->query_real_name(), 0 ) )
      printf( "%s line %d: %s\n", errmsg[0], errmsg[1], errmsg[2] );
    printf( "Check your error log for details.\n" );
  }
  else if( err = catch( ret = call_other( file, "cc" ) ) )
  {
    printf( "Run-time error: %s\n", err );
  }
  else
  {
    if(dest) destruct(FINDO(file));
    if(remove) rm(file);
    if(!silent && more)
      this_player()->more_string( sprintf( "Returned: %O\n", ret ) );
    else if
      (!silent) printf( "Returned: %O\n", ret );
   }
   return 1;
   */
}

/*
string
format_cc(string str)
{
  int odd;
  int i, len, where;
  len=strlen(str);
  odd=0;
  for(i=0;i<len;i++)
  {
    switch(str[i])
    {
      case '\'':
        if(!odd&&str[i-1]!='#') odd='\'';
        else if(odd=='\'') odd=0;
        break;
      case '\"': 
        if(!odd) odd='\"';
        else if(odd=='\"') odd=0; // "
        break;
      case '\\':
        i++;
        break;
      case '\;':
        if(!odd)
        {
          str=str[0..i]+"\n"+str[i+1..];
          len++;
        }
        break;
    }
  }
  return(str);
}
*/
