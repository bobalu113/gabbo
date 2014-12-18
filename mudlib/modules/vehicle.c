inherit ContainerCode;

inherit MobileMixin;

/**
 * Returns true to designate that this object represents a vehicle.
 * 
 * @return 1 
 */
int is_vehicle() {
  return 1;
}

/**
 * Return a zero-width mapping of the capabilities this program provides.
 * @return a zero-width mapping of capabilities
 */
mapping query_capabilities() {
  return ContainerCode::query_capabilities() 
         + MobileMixin::query_capabilities();
}
