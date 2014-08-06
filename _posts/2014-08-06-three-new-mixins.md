---
published: true
---

Welp, I was going to try and finish path expansion tonight but I left the stupid plug for my laptop at the office today so here we are. Not looking forward to driving all the way back up to the city tomorrow to get it, but at least this gives me the opportunity to catch the blog up on some of my recent commits. Tonight I will be writing about the APIs for object ids, player names, detail descriptions.

### IdMIxin
With IdMixin, the main purpose here was to augment the lfun id(str) applied by the driver with something that would help create more friendly user-interfaces. The id() lfun is very versitile because it's just a true/false test; you can throw whatever crazy logic in there you want without regard to what, if anything, it has to do with the id string you got passed as an argument. However, it's not super great when you want to ask "hey what ids are valid for this object?" That's the problem the IdMixin attempts to solve.

One of the things I knew I wanted to do from the beginning was provide an interface that would make things like  Valentino's contacts a lot easier to function. The biggest difference from EotL's ThingCode ids list is that there are now two different specifications. A "primary" id string, of which there can be only one value; and an optional list of one or more "secondary" id strings. The notion is that the primary id string should be something that would be use to provide hints to the user about how to use an object. Eventually it'd be nice to get rid of things like "You may <reload gun>." that get hardcoded into object descriptions and instead make it more dynamic: "You may <reload [primary id]>." Actually, what I really want it to do away with having to put that crap in descriptions entirely and come up with some API that exposes actionable characteristics of an item, that could be combined with IdMixin to provide usage hints to the user. But that's another post.

Beyond that, IdMixin is pretty simple, though if people end up wanted to implement more contextual object ids, then it could be complicated. What I really want to avoid is people having to implement their own id() function. So if someone has to put logic in id() that changes how an object is identified based on some other game dynamic, hopefully we can abstract that stuff out into the IdMixin API.

### NameMixin
Before I get into NameMixin, it might be useful to provide a little background on how I see user accounts playing out. There are basically three levels of "youness". When you sign in for the first time, you will be asked to pick a "username". This username must not be currently in use by any other user, so it can be used to uniquely identify you in the game. When you select your username, you will also be assigned a universally unique identifier; usernames may be recycled as accounts get deleted for whatever reason, but this identifier will be unique to each newly created account. Objects which have to persist data for a player for long periods of time should always use the unique identifier, so if the user is deleted and recreated later on, they don't get access to the old user's data. Finally, there is the concept of "player names". This is what NameMixin provides.

NameMixin is for encapsulating the proper names of players and NPCs (Bobby Schaetzle, Bobalu, Zelda), not places (Disneyland) or things (Excalibur). The short version is:

##### query_first_name(), query_last_name() 
First and last names would most likely be specified when running the game within the context of a corporeal environment, such as a office where people tend to refer to each other by their proper names. But they could also show up in fantasy environments as well, either on NPCs (Jean-Luc Picard) or possibly players (Iffy Bonzoolie). It should be also noted that one may show up without the other, such as in the case of artists who only go by one name.

##### query_full_name()
The default implementation of this function just concatenates the first and last names, but full names have their own interface so other types of names may be provided for. For instance, an NPC may have first and last names of "James" and "Kirk" respectively, but return a full name of "James Tiberius Kirk".

##### query_nickname()
I'm still not entirely sure about this one. It's intended mainly for the use of Internet handles (Devo, Bobalu), and not necessarily for what people would customarily think of as nicknames (Bones). That's the reason why only one nickname may be returned, as opposed to a collection of names. That said, if it becomes useful to provide a mechanism for providing nicknames in the traditional sense, we may want to revisit the issue.

While names are distinct from ids (setting a first name of "Bobby" doesn't automatically grant me the id of "bobby"), they still have an important role to play in object identification. First of all, ids should be set in accordance with how things get named, even if we're using two different APIs to encapsulate that stuff. The IdMixin is specifically for identifying objects with the present() efun, but there are other kinds of object identification. When you "tell devo hi", I'm being identified by name, not id. There may be some contexts in which an object may easily be identified by first name, such as in a small group of players out adventuring together. Or another example could be something like conversation, where a full name is used to initiate converation by in subsequent messages a first name will suffice. In still other contexts, we must use the username to guarantee uniqueness; or it may be preferable to a player to use the username, so that the correct user is targeted no matter which player character they're actually logged in with at the time.

### DetailMixin
Lastly there is DetailMixin. This is for setting those detail descriptions on rooms (or anything, really) where you want a player to be able to look at a more detailed description of one aspect of an object without actually having to create supplemental objects to be targeted by the look command. This API is probably the most divergent of the three from how EotL currently works.

The main difference is that the detail ids and their associated descriptions are now expressed in a hierarchy instead of a flat list. For instance, say you wrote a room that mentioned a painting in its description:
<code><pre>
set_detail("painting", "a painting of a mermaid");
set_detail("mermaid", "she's got no legs!", "painting");
// or
set_detail("painting.mermaid", "she's got no legs!");
</pre></code>

Now if the player is in the room with an actual mermaid, they can either "look mermaid" to look at the NPC, or "look painting.mermaid" to look at the painting. Furthermore, since each detail id is now contextual, we can do sugary things like automatically highlighting the words in descriptions which may be furthur inspected. Again, the whole idea behind the design of this stuff is to make the text-based interface into all this stuff more bearable than it's been in the past.

That's it for now I think. As always, the API docs are in the documentation section if you want a more technical reference. There's also utility functions and stuff in the API docs that I didn't mention here.