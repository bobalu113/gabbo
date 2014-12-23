---
published: true
category: blog
layout: blog
---

This weekend I finished [MobileMixin](http://bobalu113.github.io/gabbo/docs/mudlib//modules/mobile.html), a module that provides the capability for objects to move on their own. This is the evolution of EotL's move_player lfun in the player body. The new API provides two distinct functions, one for movement via a room exit, and one movement via teleportation:

<code><pre><!--
-->int exit(string verb, string dir, int flags);
int teleport(mixed dest, int flags);
</pre></code>

There's also an extra argument for some control flags. I'll break those down in detail later, but they're there so you don't have to have esoteric markers like "XX" for direction to muffle the movement messages. You'll also notice that the exit() function does not take a destination. Since we now <i>know</i> that we're using an exit, we can infer the destination from the exit direction. This means you can't do weird things like set the direction to 'into the cave', or to 'west' when the player's really moving east. The teleport function, on the other hand, does not take a direction, it only takes a destination room. No longer can you just make up movement messages willy-nilly, they have to come from some sort of well-formed API.

That's the basic overview, now I'll break down some of the finer points.


#### exit verbs 
One major deviation from EotL's model is that the exit direction itself is no longer the verb. Instead, movement will be done via one of several movement commands, usually of the form &lt;verb&gt; &lt;direction&gt;. Examples could be 'walk &lt;dir&gt;' or 'run &lt;dir&gt;' for movement by foot, or 'drive &lt;dir&gt;' when using a vehicle. There will still be support for the exit directions themselves as valid commands, but a hook in modify command will expand the command from '&lt;dir&gt;' to '&lt;verb&gt; &lt;dir&gt;' based on a player setting for default verb. Right now the only verb I've written is 'walk', but the support for different verbs opens up some interesting possibilities. 

With movement verbs, you can now let different forms of movement do different things. One of the ideas I'm playing with right now is letting you 'walk dir1, dir2, ..., dir n' and the game will advance you automatically through each of those rooms in a single execution. The goal is to remove some of the tedium of moving around medium to large distances inside the game. This doesn't mean you can't still have events take place along the way to affect the player's ability to advance to the next room. It just means that instead of basing the outcome of those events on the player's connection speed and their ability to use telnet, we can base them on discrete movement types. Maybe running through rooms instead of walking reduces your chance to be auto'd by some aggressive monster, but costs fatigue and rolls on endurance or something.

This new feature doesn't come without some cost. First, there is no longer any room for ambiguity when it comes to describing how a player is moving about. By requiring a verb and making it meaningful, the developer may sometimes be confronted with some new choices and challenges. What happens if there's a ladder in a room and a player tries to 'walk up' instead of 'climb up'? Or what if there are stairs? Should you support 'climb stairs' or 'walk stairs' or both? Climbing implies some sort of additional physical labor maybe. But then maybe we want to distinguish running upstairs from walking upstairs to incur the user with some additional risk, so we need to support those verbs. What about creatures that don't walk or run at all? Should there be different verbs built into the race for a snake to slither or a slug to...whatever it is that slugs do? Does there need to be some kind of verb registry so we can apply properties to different verbs that can be accessed globally?

Another challenge is how best to integrate the new verb support into movement messaging. I'd like a player to be able to customize their default movement message with the correct conjugation of the verb without the code doing the movement having to know anything about what's happening on the other end. Right now I've decided to set a standard that the verb should always be passed in its infinitive form (minus to 'to' prefix). Them we can create format specifiers (more on format strings later) that force the necessary conversion (e.g. 'walk' to 'walks' or 'is walking'). Hopefully there will be a webservice we can use in the near future to do the actual verb conjugation, but until then I wouldn't think that maintaining static support for specific verbs should be too cumbersome.


#### movement flags
As I mentioned earlier, instead of muffling movement messages with a special direction string, you'll now use exit flags. There are already support for a number of flags, here they are:

* <strong>MUFFLED_MOVE</strong>: suppress movement messages
* <strong>FOLLOW</strong>: force followers to follow, even when teleporting
* <strong>NO_FOLLOW</strong>: prevent followers from following, even when using an exit
* <strong>CMD_FOLLOW</strong>: following is implemented using command(), instead of follower->exit()
* <strong>FORCE_TELEPORT</strong>: override NoTeleport properties (which aren't actually implemented yet)
* <strong>SUPPRESS_ERRORS</strong>: do not echo error messages to THISP (things like "Sorry this area is incomplete." when a room doesn't load)
* <strong>SUPPRESS_LOOK</strong>: do not execute the 'look' command upon successfully movement

There will be a breakdown of the RoomCode API in a later post, but I'll also mention that MobileMixin also supports some flags on the exits themselves:

* <strong>EXIT_BLOCKED</strong>: the exit is blocked by something
* <strong>EXIT_MUFFLED</strong>: exit messages should muffled
* <strong>ENTRANCE_MUFFLED</strong>: entrance messages should muffled
* <strong>EXIT_NO_FOLLOW</strong>: prevent following through this exit
* <strong>EXIT_PEDESTRIAN</strong>: prevent vehicles from moving through the exit


#### intrazone exits vs. interzone exits 
With MobileMixin comes the introduction of 'zones' as a game concept. It's a much broader idea that can be discussed here, but here's the basic idea. With the new RoomCode, all rooms within a zone must be laid out in a 3d Cartesian grid. That is, every room is given a coordinate in 3d space, and the exits must be laid out in that space accordingly. Only the 10 standard directions are allowed: up, down north, northeast, east, southeast, south, southwest, west, northwest. If roomA exits west to roomB, then roomB must exit east to roomA. The distance between rooms is fixed within the zone.

However, you may have custom exits outside of (and including) the ten standard exits that move a player between different zones. Between different zones there is no referential integrity; that is, there is no guaranteed method for a player that moves through an exit from zoneA to zoneB to find his way back to zoneA. This doesn't mean the code can't make guesses about which exits goes where, it just means there's no expectation of referential integrity.


#### movement messages
The API for movement messages has evolved somewhat as well. Instead of static strings which simply have the exit direction appended onto the end, the movement messages are now format strings which have replacement specifiers. There are currently specifiers for verb (infinitive tense), direction, and display name. I haven't gotten to the conjugation stuff I mentioned earlier, but the place is to let the verb specifier take an argument that denotes verb tense. The display name thing is another concept that's not fully flushed out. It's some configurable derivative of names from NameMixin short from VisibleMixin, with some eventual hooks for things like stealth and invisibility.

There are 6 configurable messages: exit message and entrance message for each of intrazone exit, interzone exit, and teleportation.  Unlike EotL's API, we can now guaranteed a valid direction for entrance messages through intrazone exits, so you can do things like "Devo enters from the south." For interzone entrance messages, we do a best guess at what the direction might be based on the room of origin's exit map, but there's no guarantee that a direction will be supplied. Still not sure what to do about this yet; may possibly support a default direction as an arg to the format specifier, but I'm not sure how useful that would be.

Since I'm giving players the ability to set fancy movement messages for thsemselves like 'From the west, Devo sneaks in quietly.', we will need to resolve the issue of players setting misleading messages to gain an advantage in the game. I haven't really pondered on this much, but I'm not too worried about it. There are things we can do, like highlighting certain terms and allowing message overloads and stuff.


#### multiple follow targets
Finally, I made a slight change to the relationship between follower and followees. Departing from the EotL model, an object using MobileMixin may now follow multiple objects. If you're in the room with two people whom you are both following, you will follow whichever one moves first. I also changed the follow command so that you can initiate following with players not in the same room with you. I see no reason why you can't declare that you want to follow someone, should they enter your environment. I should think this will make moving around in groups much more manageable. I haven't written any code that deals with an infinite follow loop, but I believe there is code I can borrow from EotL for this.


That's about it. All player avatars inherit MobileMixin, but I'm not sure if I want all LivingCode objects to. What if someone wants to code some sort of animated plant? It should probably inherit LivingCode even though MobileMixin wouldn't make much sense. On the other hand, you still might want it to have a teleportation capability with movement message support, et al. The VehicleCode idea is also not really flushed out beyond me wanting an example of something that used ThingCode and MobileMixin together. The basics are all there, though. I also put H_MOVE_OBJECT back to the EotL version, save for a few syntactic changes, so all that seems to be working fine. 
  
