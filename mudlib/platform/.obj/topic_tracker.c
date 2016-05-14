/**
 * A service object to catalog message topic metadata.
 *
 * @author devo@eotl
 * @alias TopicTracker
 */

// ([ str topic_id :  map topic_definition ])
mapping topics;

public object get_renderer(string topic_id, string term) {
  if (member(topics, topic_id)) {
    return topics[topic_id]["renderer"];
  }
  return 0;
}

void create() {
  topics = ([ 
    "system.login" : ([ "renderer" : LoginRenderer ]),
    "system.welcome" : ([ "renderer" : LoginRenderer ]),
  ]);
}
