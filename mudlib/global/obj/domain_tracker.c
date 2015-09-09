/**
 * A service object for tracking domain configurations.
 *
 * @alias DomainTracker
 */
#pragma no_clone
#include <sys/xml.h>
#include <domain.h>

struct DomainConfig {
  string id, parent, domain_id;
  string root;
  closure allow_read, allow_write;
};

// ([ str domain_id : DomainConfig domain ])
mapping domains;

// ([ str domain_id : ([ string child_id, ... ]) ])
mapping children;

// ([ str domain_root : DomainConfig domain ])
mapping domain_roots;

void create();
void reconfig_signal(string file, string func);
private struct DomainConfig parse_config(string domain_file);
private void parse_directive(mixed *tag, struct DomainConfig config,
                             mixed *read_checker, mixed *write_checker);
private void parse_configure_access(mixed *tag, mixed *read_checker,
                                    mixed *write_checker);
private string get_parent_domain_file(string domain_file);
private string get_domain_root(string domain_file);
private int update_domain(struct DomainConfig config);
private int delete_domain(struct DomainConfig config);
string query_domain_id(string path);
struct DomainConfig query_domain(string domain_id);
mapping query_children(string domain_id);

/*
  Initialize global variables and start listening to domain.xml changes.
 */
void create() {
  FileTracker->subscribe("/etc/\\.domain\\.xml$", #'reconfig_signal); //'
  domains = ([ ]);
  children = ([ ]);
  domain_roots = ([ ]);
}

/**
 * Called when a domain.xml file is modified or removed.
 *
 * @param file path to domain.xml file
 * @param func write method (see valid_write())
 */
void reconfig_signal(string file, string func) {
  if (file_exists(file)) {
    struct DomainConfig config = parse_config(file);
    if (!config) {
      logger->warn("unable to parse config %O", file);
      return;
    }
    if (!update_domain(config)) {
      logger->warn("unable to update domain %O", config->domain_id);
      return;
    }
  } else {
    struct DomainConfig config = query_domain(file);
    if (config) {
      if (!delete_domain(config)) {
        logger->warn("unable to delete domain %O", config->domain_id);
        return;
      }
    }
  }
  return;
}

/**
 * Parse the configuration found by specified file and return it.
 *
 * @param  domain_file the domain.xml file to parse
 * @return             the parsed domain configuration
 */
private struct DomainConfig parse_config(string domain_file) {
  object logger = LoggerFactory->get_logger(THISO);
  struct DomainConfig config = (<DomainConfig>);

  // root dir for
  config->root = get_domain_root(domain_file);

  // populate parent (parsing ancestor config(s) if necessary)
  struct DomainConfig parent_config;
  string parent_file = get_parent_domain_file(domain_file);
  if (parent_file) {
    string parent_root = get_domain_root(parent_file);
    if (member(domain_roots, parent_root)) {
      parent_config = domain_roots[parent_root];
    } else {
      parent_config = parse_config(parent_file);
      if (!parent_config) {
        logger->warn("unable to parse parent config %O", parent_file);
        return 0;
      }
      update_config(parent_config);
    }
  }
  if (parent_config) {
    config->parent = parent_config->domain_id;
  }

  mixed *xml = xml_parse(read_file(domain_file));
  if (config[XML_TAG_NAME] != "domain") {
    logger->info("invalid domain.xml: root tag must be <domain>");
    return 0;
  }
  if (!member(config[XML_TAG_ATTRIBUTES], "id")) {
    logger->info("invalid domain.xml: root domain tag missing id attribute");
    return 0;
  }
  config->id = config[XML_TAG_ATTRIBUTES]["id"];
  if (parent_config) {
    config->domain_id = parent_config->domain_id + "." + config->id;
  } else {
    config->domain_id = config->id;
  }

  // initialize checkers
  mixed *read_checker =
    ({ #',,
       ({ #'=, 'result, 0 }) //'
    });
  mixed *write_checker = deep_copy(read_checker);

  // parse config directives
  if (pointerp(config[XML_TAG_CONTENTS])) {
    foreach (mixed *tag : config[XML_TAG_CONTENTS]) {
      parse_directive(tag, config, &read_checker, &write_checker);
    }
  }

  // finalize and compile checkers
  read_checker += ({ ({ #'return, 'result }) });
  write_checker += ({ ({ #'return, 'result }) });
  config->allow_read = lambda(
    ({ 'path, 'user, 'groups, 'domain, 'program })), //'
    read_checker
  );
  config->allow_write = lambda(
    ({ 'path, 'user, 'groups, 'domain, 'program })), //'
    write_checker
  );

  return config;
}

/**
 * Parse a configuration directive.
 *
 * @param tag           the XML tag specifying the directive
 * @param config        the domain config being built
 * @param read_checker  a partial closure which will completed with read
 *                      access logic
 * @param write_checker a partial closure which will be completed with write
 *                      access logic
 */
private void parse_directive(mixed *tag, struct DomainConfig config,
                             mixed *read_checker, mixed *write_checker) {
  if (tag[XML_TAG_NAME] == "configureAccess") {
    parse_configure_access(tag, &read_checker, &write_checker);
  }
  return;
}

/**
 * Parse the "configureAccess" configuration directive.
 *
 * @param tag           the configureAccess tag
 * @param read_checker  a partial closure which will completed with read
 *                      access logic
 * @param write_checker a partial closure which will be completed with write
 *                      access logic
 */
private void parse_configure_access(mixed *tag, mixed *read_checker,
                                    mixed *write_checker) {
  object logger = LoggerFactory->get_logger(THISO);

  // condition blocks for whether to apply this config at all
  mixed *relativePath, *startsWith, *endsWith;
  if (member(tag[XML_TAG_ATTRIBUTES], "relativePath")) {
    relativePath =
      ({ #'==,
         'path,  //'
         tag[XML_TAG_ATTRIBUTES]["relativePath"]
      });
  }
  if (member(tag[XML_TAG_ATTRIBUTES], "startsWith")) {
    startsWith =
      ({ #'==,
         ({ #'[..],
            'path,  //'
            0,
            strlen(tag[XML_TAG_ATTRIBUTES]["startsWith"]) - 1
         }),
         tag[XML_TAG_ATTRIBUTES]["startsWith"]
      });
  }
  if (member(tag[XML_TAG_ATTRIBUTES], "endsWith")) {
    endsWith =
      ({ #'==,
         ({ #'[<..,
            'path,  //'
            strlen(tag[XML_TAG_ATTRIBUTES]["endsWith"])
         }),
         tag[XML_TAG_ATTRIBUTES]["endsWith"]
      });
  }
  // build the condition
  read_checker += ({ #'?, ({ #'&& }) });
  write_checker += ({ #'?, ({ #'&& }) });
  if (relativePath) {
    read_checker[<1] += ({ relativePath });
    write_checker[<1] += ({ relativePath });
  }
  if (startsWith) {
    read_checker[<1] += ({ startsWith });
    write_checker[<1] += ({ startsWith });
  }
  if (endsWith) {
    read_checker[<1] += ({ endsWith });
    write_checker[<1] += ({ endsWith });
  }

  if (pointerp(tag[XML_TAG_CONTENTS])) {
    // configure the target objects this operation applies to
    foreach (mixed *op : tag[XML_TAG_CONTENTS]) {
      mapping users, groups, domains, programs;
      if (member(op[XML_TAG_ATTRIBUTES], "users")) {
        users = mkmapping(filter(explode(
                  op[XML_TAG_ATTRIBUTES]["users"], ","
                ), #'trim)); //'
      }
      if (member(op[XML_TAG_ATTRIBUTES], "groups")) {
        groups = mkmapping(filter(explode(
                  op[XML_TAG_ATTRIBUTES]["groups"], ","
                ), #'trim)); //'
      }
      if (member(op[XML_TAG_ATTRIBUTES], "domains")) {
        domains = mkmapping(filter(explode(
                  op[XML_TAG_ATTRIBUTES]["domains"], ","
                ), #'trim)); //'
      }
      if (member(op[XML_TAG_ATTRIBUTES], "programs")) {
        programs = mkmapping(filter(explode(
                  op[XML_TAG_ATTRIBUTES]["programs"], ","
                ), #'trim)); //'
      }

      // figure out what operation/permission we're configuring
      mixed *checker;
      int perm = -1;
      switch (op[XML_TAG_NAME]) {
      case "allowRead":
        read_checker += ({ 0 });
        checker = read_checker;
        perm = 1;
        break;
      case "allowWrite":
        write_checker += ({ 0 });
        checker = write_checker;
        perm = 1;
        break;
      case "denyRead":
        read_checker += ({ 0 });
        checker = read_checker;
        perm = 0;
        break;
      case "denyWrite":
        write_checker += ({ 0 });
        checker = write_checker;
        perm = 0;
        break;
      default:
        logger->info("invalid domain.xml: unknown configureAccess "
                     "operation: %O", op[XML_TAG_NAME]);
        break;
      }

      // build the condition and set the result if necessary
      if (checker) {
        mixed *cond = ({ #'|| }); //'
        if (users) {
          cond += ({ ({ #'member, users, 'user }) });
        }
        if (groups) {
          cond += ({ ({ #'sizeof, ({ #'-, groups, 'groups }) }) }); //'
        }
        if (domains) {
          cond += ({ ({ #'member, domains, 'domain }) });
        }
        if (programs) {
          cond += ({ ({ #'member, programs, 'program }) });
        }
        checker[<1] = ({ #'?, cond, ({ #'=, 'result, perm }) }); //'
      }

    }
  }

  return;
}

/**
 * For a specified domain file, return the parent domain file, or 0 if no
 * parent domain exists.
 *
 * @param  domain_file the child domain file
 * @return             the parent domain file, or 0
 */
private string get_parent_domain_file(string domain_file) {
  string dir = get_domain_root(domain_file);
  while (dir = dirname(dir)) {
    string file = dir + "/" DOMAIN_FILE;
    if (file_exists(file)) {
      return file;
    }
  }
  return 0;
}

/**
 * For a specified domain file, return the root dir for that domain.
 *
 * @param  domain_file the path to the domain file
 * @return             the domain root
 */
private string get_domain_root(string domain_file) {
  return domain_file[0..<(strlen(DOMAIN_FILE) + 2)];
}

/**
 * Create a new domain or update an existing one.
 *
 * @param  config the domain configuration to update
 * @return        1 if confguration was successfully updated, otherwise 0
 */
private int update_domain(struct DomainConfig config) {
  // make sure incoming domain doesn't cause a id collision
  if (member(domains, config->domain_id)) {
    if (domains[config->domain_id]->root != config->root) {
      log->info("new domain config at root %O conflicts with domain root %O",
               config->root, domains[config->domain_id]->root);
      return 0;
    }
  }

  // removing existing domain at this root
  if (member(domain_roots, config->root)) {
    struct DomainConfig existing = domain_roots[config->root];
    string parent = existing->parent;
    if (parent) {
      m_delete(children[parent], existing->domain_id);
    }
    m_delete(domains, existing->domain_id);
    m_delete(domain_roots, config->root);
  }

  // add new config
  if (config->parent) {
    if (!children[config->parent]) {
      children[config->parent] = ([ ]);
    }
    children[config->parent] += ([ config->domain_id ]);
  }
  domains[config->domain_id] = config;
  domain_roots[config->root] = config;
  return 1;
}

/**
 * Delete a domain.
 *
 * @param  config the domain configuration to delete
 * @return        1 if domain was successfully deleted, otherwise 0
 */
private int delete_domain(struct DomainConfig config) {
  // make sure deleting subdomain won't cause sibling domain collision
  if (member(children, config->domain_id)) {
    string parent = "";
    if (config->parent) {
      parent = config->parent + ".";
    }
    foreach (string child_id : children[config->domain_id]) {
      string id = domains[child_id]->id;
      if (member(domains, parent + id)) {
        log->info("deleting config %O would cause sibling id collision",
                  config->domain_id);
        return 0;
      }
    }
  }

  // remove us from tracked domains
  m_delete(domain_roots, config->root);
  m_delete(domains, config->domain_id);

  // remove us from our parent's children
  if (config->parent) {
    m_delete(children[config->parent], config->domain_id);
  }

  // update our children to now belong to our parent
  if (member(children, config->domain_id)) {
    foreach (string child_id : children[config->domain_id]) {
      struct DomainConfig child = domains[child_id];
      child->parent = config->parent;
      m_delete(domains, child_id);
      if (child->parent) {
        child->domain_id = child->parent + "." + child->id;
      } else {
        child->domain_id = child->id;
      }
      domains[child->domain_id] = child;
      children[config->parent] += ([ child->domain_id ]);
    }
  }

  return 1;
}

/**
 * For a given filesystem path, return the domain id the file belongs to.
 * Will discover new domains from domain.xml files if necessary.
 *
 * @param  path the path of the file for which to get domain
 * @return      the domain id of the file, or 0 if no domain could be found
 */
string query_domain_id(string path) {
  do {
    if (member(domain_roots, path)) {
      return domain_roots[path]->domain_id;
    } else {
      string domain_file = path + "/" DOMAIN_FILE;
      if (file_exists(domain_file)) {
        struct DomainConfig config = parse_config(domain_file);
        if (!config) {
          logger->warn("unable to parse config %O", domain_file);
          return 0;
        }
        if (!update_domain(config)) {
          logger->warn("unable to update config %O", config->domain_id);
          return 0;
        }
        return config->domain_id;
      }
    }
  } while(path = dirname(path));
  return 0;
}

/**
 * Get the domain configuration for a specified domain id.
 *
 * @param  domain_id the domain id to get
 * @return           the DomainConfig of the specified id, or 0 if no domain
 *                   was found
 */
struct DomainConfig query_domain(string domain_id) {
  return deep_copy(domains[domain_id]);
}

/**
 * Get a list of child domains belonging to a specified domain.
 *
 * @param  domain_id the parent domain id
 * @return           a 1-d mapping of child domain ids
 */
mapping query_children(string domain_id) {
  if (!member(children, domain_id)) {
    return ([ ]);
  }
  return copy(children[domain_id]);
}
