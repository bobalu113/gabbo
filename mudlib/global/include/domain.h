#ifndef _DOMAIN_H
#define _DOMAIN_H

#define DOMAIN_FILE       "etc/domain.xml"

struct DomainConfig {
  string id, parent, domain_id;
  string root;
  closure allow_read, allow_write;
};

#endif  // _DOMAIN_H
