---
published: false
---

Welp, I was going to try and finish path expansion tonight but I left the stupid plug for my laptop at the office today so here we are. Not looking forward to driving all the way back up to the city tomorrow to get it, but at least this gives me the opportunity to catch the blog up on some of my recent commits. Tonight I will be writing about the APIs for object ids, player names, detail descriptions.

### IdMIxin
With IdMixin, the main purpose here was to augment the lfun id(str) applied by the driver with something that would help create more friendly user-interfaces. The id() lfun is very versitile because it's just a true/false test; you can throw whatever crazy logic in there you want without regard to what, if anything, it has to do with the id string you got passed as an argument. However, it's not super great when you want to ask "hey what ids are valid for this object?" That's the problem the IdMixin attempts to solve.

One of the things I knew I wanted to do from the beginning was provide an interface that would make things like  Valentino's contacts a lot easier to function. The biggest difference from EotL's ThingCode ids list is that there are now two different specifications. A "primary" id string, of which there can be only one value; and an optional list of one or more "secondary" id strings. The notion is that the primary id string should be something that would be use to provide hints to the user about how to use an object. Eventually it'd be nice to get rid of things like "You may <reload gun>." that get hardcoded into object descriptions and instead make it more dynamic: "You may <reload [primary id]>." Actually, what I really want it to do away with having to put that crap in descriptions entirely and come up with some API that exposes actionable characteristics of an item, that could be combined with IdMixin to provide usage hints to the user. But that's another post.

Beyond that, IdMixin is pretty simple, though if people end up wanted to implement more contextual object ids, then it could be complicated. What I really want to avoid is people having to implement their own id() function. So if someone has to put logic in id() that changes how an object is identified based on some other game dynamic, hopefully we can abstract that stuff out into the IdMixin API.

### NameMixin
Before I get into NameMixin, it might be useful to provide a little background on how I see user accounts playing out. There are basically three levels of "youness". When you sign in for the first time, you will be asked to pick a "username". This username must not be currently in use by any other user, so it can be used to uniquely identify you in the game. When you select your username, you will also be assigned a universally unique identifier; usernames may be recycled as accounts get deleted for whatever reason, but this identifier will be unique to each newly created account. Objects which have to persist data for a player for long periods of time should always use the unique identifier, so if the user is deleted and recreated later on, they don't get access to the old user's data. Finally, there is the concept of "player names". This is what NameMixin provides.

NameMixin is for encapsulating the proper names of players and NPCs (Bobby Schaetzle, Bobalu, Zelda), not places (Disneyland) or things (Excalibur). 

