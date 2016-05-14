/**
 * Utility library for tracking domains.
 *
 * @author devo@eotl
 * @alias DomainLib
 */

#include <domain.h>

struct DomainConfig {
  string id, parent, domain_id;
  string root;
  closure allow_read, allow_write;
};
