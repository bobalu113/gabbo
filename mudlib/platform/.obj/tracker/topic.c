/**
 * A service object to catalog message topic metadata.
 *
 * @author devo@eotl
 * @alias TopicTracker
 */
#pragma no_clone

inherit FileLib;
inherit ObjectLib;

// ([ str topic_id :  map topic_definition ])
mapping topics;

void setup();
public object get_renderer(string topic_id, string term);
public string get_controller_topic(object controller);

/**
 * Setup the TopicTracker.
 */
void setup() {
  topics = ([ 
    "system.login" : ([ "renderer" : LoginRenderer ]),
    "system.welcome" : ([ "renderer" : LoginRenderer ]),
  ]);
}

/**
 * Get the renderer for a given topic and term.
 * 
 * @param  topic         the topic being queried
 * @param  term          the terminal type
 * @return the renderer object
 */
public object get_renderer(string topic, string term) {
  if (member(topics, topic)) {
    return load_object(topics[topic]["renderer"]);
  }
  return load_object(DefaultRenderer);
}

/**
 * Get a default topic for a given ob's messages.
 * 
 * @param  ob            the object doing the messaging
 * @return a topic
 */
public string get_topic(object ob) {
  return sprintf("%s.%s", 
    get_flavor(ob), 
    basename(program_name(ob)));
}

/**
 * Constructor.
 */
void create() {
  setup();
}

