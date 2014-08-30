---
published: true
category: blog
layout: blog
---

Haven't coded anything in over two weeks now...I'm got hung up on this object parsing stuff. More new questions keep popping up than answers, and I don't feel like I'm any closer to movement on it than I was two weeks ago. Pretty sure I'm gonna have to use my phone-a-friend on this one.

To what degree should the player be able to control how objects on his commandline are being parsed? Is it ever appropriate for me, as a user, to change how objects are searched as targets for a command? Or should the command have tight control over the types of things coming back to it from a search? How does this actually affect how we'd code something, for instance the tell or look commands? Tell is different than most commands in that it can target people in other rooms. Look is different because you can look at all the room detail descriptions but those don't resolve as distinct LPC objects.

Zamboni used this idea of "priorities" but I don't know if I like it. Basically, the client when it passes the ospec to the parser can specify an ordered list of "priority" tokens to say things like "search the command giver's current environment, then the rest of the mud". That feels somewhat constricting, though. What if, temporarily, a player wanted to search for objects using a context that was runtime dependent? As a crazy example, let's say I coded a camera, that the members of a group could wear and broadcast its video to the rest of the group. So then, for a time, I wanted to make it so that the look command could be used to target any object in any of their environments while these cameras are being worn. What say does the client command and the player have in that whole transaction?

Another knot in this board is figuring what I want from the help system. In my head I have this fantasy where a player knows exactly what help arguments to specify, and when the help page comes up they can tell what a command does, what it operates on, how arguments are specified, what's providing the command, in what contexts the command is sensible, and anything else that helps them decide what to type next.

Coding for all these scenarios would, of course, be crazy overengineered for what I'm doing with the game right now, but this is one of those interfaces that could be difficult to change later if done wrong. I just want to have options.
