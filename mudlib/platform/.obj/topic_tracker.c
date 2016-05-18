/**
 * A service object to catalog message topic metadata.
 *
 * @author devo@eotl
 * @alias TopicTracker
 */

private variables private functions inherit FileLib;
private variables private functions inherit ObjectLib;

// ([ str topic_id :  map topic_definition ])
mapping topics;

public object get_renderer(string topic_id, string term) {
  if (member(topics, topic_id)) {
    return load_object(topics[topic_id]["renderer"]);
  }
  return load_object(DefaultRenderer);
}

public string get_controller_topic(object controller) {
  return sprintf("%s.%s", 
    get_flavor(controller), 
    basename(program_name(controller)));
}

void create() {
  topics = ([ 
    "system.login" : ([ "renderer" : LoginRenderer ]),
    "system.welcome" : ([ "renderer" : LoginRenderer ]),
  ]);
}

