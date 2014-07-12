// Humbly derived from AcmeGetOpts

mixed *getopts(string *args, string validopts) {
  string badopts = "";
  mapping options = ([ ]);
  int anyopt;
 
  if (validopts) {
    // pad it to make checking for ':' easier
    validopts += " ";
  } else {
    // any option will be gathered
    anyopt = 1;
  }
  
  int i = 0;
  for (int size = sizeof(args); i < size; i++ ) {
    // Reached a non-option
    if (args[i][0] != '-') { break; }  
    
    if (args[i] == "--") {  
      // Explicit end of options
      // increment past this so we don't keep it
      i++;                   
      break;
    }
 
    // Stray '-'.  Skip it
    if (args[i] == "-") { continue; }
    
    for (int j = 1, int sz = strlen(args[i]) - 1; j < sz; j++) {
      int opt = args[i][j];
      int pos;
      if (anyopt 
        || ( ((pos = member(validopts, opt)) > -1)
          && (validopts[pos+1] != '-') 
          && (validopts[pos+1] != ':') )) {
        options[opt] = 0;
      } else {
        badopts += args[i][j..j];
      }
    }
 
    // Extract the option char  ('s' from "-s")
    int opt = args[i][<1];
    
    if (anyopt) {
      options[opt] = 0;
      continue;
    }
    
    int pos = member(validopts, opt);
    if (pos > -1) { 
      // Valid option
      if (validopts[pos+1] == ':') { 
        // This option takes an argument
        if (++i < size) {
          // If we're not at the end
          options[opt] = args[i];
          continue;
        }
      } else if (validopts[pos+1] == '-') { 
        // This option takes an argument and can be repeated. 
        if (++i < size) {
          // If we're not at the end
          if (!pointerp(options[opt])) {
            options[opt] = ({ });
          }
          options[opt] += ({ args[i] });
          continue;
        }
      } else {
        options[opt] = 0;
        continue;
      }
    }
 
    // If we made it this far, the option is either invalid or requires and 
    // arg and wasn't given one.
    badopts = sprintf("%s%c", badopts, opt);
  }
 
  return ({ args[i..], options, badopts });
}
 
mixed *getopts_long(string *args, string validopts, mapping longopts) {
  mixed *badopts = ({ });
  mapping options = ([ ]);
  int anyopt, anylongopt;
 
  if (validopts) {
    // pad it to make checking for ':' easier
    validopts += " ";
  } else {
    // any option will be gathered
    anyopt = 1;
  }
 
  if (!mappingp(longopts)) {
    anylongopt = 1;
  }
 
  int i = 0;
  for (int size = sizeof(args); i < size; i++) {
    // Reached a non-option
    if (args[i][0] != '-') { break; }
    
    if (args[i] == "--") {
      // Explicit end of options
      // increment past this so we don't keep it
      i++;
      break;
    }
  
    // Stray '-'.  Skip it
    if (args[i] == "-" ) { continue; }
    
    if (args[i][1] != '-') {
      for (int j = 1, int sz = strlen(args[i]) - 1; j < sz; j++) {
        int opt = args[i][j];
        if (anyopt 
          || (member(validopts, opt) > -1)) {
          options[ opt ] = 0;
        } else {
          badopts += ({ opt });
        }
      }
    }

    // Extract the option char  ('s' from "-s")
    int opt = args[i][<1];
 
    if (args[i][1] == '-') {
      // It's a long option
      // We already checked for "--"
      string longopt = args[i][2..];
      if (anylongopt) {
        options[longopt] = 0;
        continue;
      }
      
      if (!member(longopts, longopt)) {
        badopts += ({ longopt });
        continue;
      }
 
      if (longopts[longopt] > 0) {
        // This option takes an argument
        if (++i < size) {
          // If we're not at the end
          options[longopt] = args[i];
        } else {  
          // Ran out of args
          badopts += ({ longopt });
        }        
      } else { 
        if (longopts[longopt] < 0) { 
          // This option takes an argument that can be used more than once.
          if (++i < size) {
            // If we're not at the end
            if (!pointerp(options[longopt])) {
              options[longopt] = ({ });
            }
            options[longopt] += ({ args[i] });
          } else { 
            // Ran out of args
            badopts += ({ longopt });
          }        
        } else {
          // Doesn't take an argument
          options[longopt] = 0;
        }
      }
      continue;      
    } // End of longopts
    
    if (anyopt) {
      options[opt] = 0;
      continue;
    }
    
    int pos = member(validopts, opt);
    if (pos > -1) { 
      // Valid option
      if (validopts[pos + 1] == ':') {
        // This option takes an argument
        if (++i < size) {
          // If we're not at the end
          options[opt] = args[i];
          continue;
        }
      } else {
        if (validopts[pos+1] == '-') { 
          // This option takes an argument and can be repeated.
          if (++i < size) {
            // If we're not at the end
            if (!pointerp(options[opt])) {
              options[opt] = ({ });
            }
            options[opt] += ({ args[i] });
            continue;
          }
        } else {
          options[opt] = 0;
          continue;
        }
      }
    }
 
    // If we made it this far, the option is either invalid or requires and 
    // arg and wasn't given one.
    badopts += ({ opt });
  }
 
  return ({ args[i..], options, badopts });
}
 
