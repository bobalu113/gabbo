---
published: true
category: blog
layout: blog
---
A while ago I wrote [a post](/gabbo/blog/2014/08/30/analysis-paralysis/) about some of the problems I was having designing how the command line would be parsed. Well I am pleased to report that a big chunk of that puzzle, how a user refers to objects in command line args, is now ready to be used. It'll need some enhancements down the line to support author mode and alternative ospecs, but I'm not there yet. In this post I'll go over the API, the query language (ospec), and some usage examples. The primary source of inspiration for this project was AcmeOSpec from EotL, but there are enough differences in my design that the code had to be completely rewritten.
<!-- more -->

### OSpec Reference

So first off, what's an ospec? An ospec is just a string that players use to refer to one or more objects in the game. A lot of times, it's something very simple, like the id of an object you are carrying. But what if you want to refer to an object that's in your backpack? Or sometimes you may want to refer to a collection of objects, like all interactive users. The ospec lets you target those things in a sensible way that should work in a variety of command giving situations. And for more extreme situations, it encapsulates the object parsing logic, making it easier to override.

The gabbo ospec is built around a fairly limited set of terms right now, but it's meant to be easily extended. An ospec is made up of one or more terms, separated by a period. The first term is a context-free term which refers directly to some object or set of objects on the mud. The following terms take that initial object pool, and filter out some objects or map the pool to a new set of objects based on some criteria.

For instance, the term "users" by itself will return the collection of all interactive objects in the game. However, the term "users.devo" will return only the user objects which id to "devo". Likewise, the term "users.devo.e" will return the room that Devo is in. The terms the parser currently understands are:

<p>Context free:</p>
  * me: the object performing the parsing, as passed to expand_objects()
  * here: the room that 'me' is currently in
  * users: all interactive objects in the game
  * &lt;oname&gt;: the object named oname, as passed to find_object()
  * &lt;fname&gt;: any object loaded from file named &lt;fname&gt;, wildcards accepted

<p>Contextual:</p>
  * e: an object environment
  * i: an object's inventory
  * living: filter out non-living objects
  * &lt;id&gt;: a present object with the specified id

<p>Special:</p>
  * &lt;detail&gt;: the end of an ospec may include a fully-qualified detail id on the object, as provided by [DetailMixin](/gabbo/blog/2014/08/06/three-new-mixins/); if a matching detail id is found, the object is returned (see below for an explanation of detail handling)

Two or more terms may be grouped together with parenthesis and separated by commas, in which case the union of all terms will be returned. The ospec "(me,here)" evaluates to my avatar and the room I'm in. The escape character '\' is respected, in case you need to use a special character in a term (such as a period in a filename).

One last thing, the context-free terms may also be used contextually, in which case they will return their respective objects only if the object is already in the current context. So an ospec of "~/workroom\\.c.here" will return your current environment, but only if it's your workroom, otherwise no objects will be returned. It should be noted that this is functionally the same thing as "here.~/workroom\\.c".

### Context

Aside from changing the ospec, another major deviation from the Acme solution is that object parsing may now be stateful. All players carry around a current "context," much like a current working directory, and any ospec you use will be parsed relative to that context. The context string is just an ospec itself, which gets prepended to any specs you try and use. In other words, if my current context were "me" and I parsed the ospec "apple", the parser would return any apples I am currently carrying, as if I had typed "me.apple".

You may explicitly update your context with the 'context' command, though some commands may update it for you implicitly. This allows your commands, using very brief specs, to drill deeper and deeper as you chain commands together. One obvious example of this is the "look" command. Each time you successfully look at something, your context is updated with the ospec you used to do the looking so the game remembers what you're currently looking at. We'll also put our current context in our prompt, and make it so you "look here" each time you move to a new room. Your experience might look something like this:

    A Blue Room
         This room is painted blue.
    ---- The only obvious exit is north.
    an apple
    a bag (open)
    here> put apple in bag
    You put an apple in a bag.
    here> look bag
    A bag for apples.
    It is holding:
    an apple
    here.bag> look apple
    A juicy apple. There's still a sticker on it.
    here.bag.apple> look sticker
    "Product of USA."
    here.bag.apple.sticker> walk north
    A Red Room
         This room is painted red.
    ---- The only obvious exit is south.
    here>

### Backtracking and Root Context

You may have noticed a problem with the example above. What happens if our current context is "here.bag.apple" and we want to look at the bag again? Won't that just look for a bag inside the apple? The answer is yes, it will, but if it doesn't find anything, the parser will then walk up the current context looking for objects that match the provided spec until it finds one. This means that "look bag" will eventually find and return the bag the apple is currently being held in, and change the current context back to "here.bag".

Even if the parser backtracks all the way to the beginning of the context path and still doesn't find anything, there's one last hope to get a matching object. The command which is executing the parser may pass a "root context" string, which will be checked in this scenario. This is meant to replace the concept of priority strings in AcmeOSpec, which was how different commands prioritized different possible targets in the search. The 'look' command passes a root context of "(here,me)", so that you can always look at anything in your room or immediate inventory. The 'goto' command passes a root context of "users", to find users in other rooms.

### ObjectExpansionLib API

The object expansion code is housed in a new library, ObjectExpansionLib. There is only one public function:

    varargs mixed *expand_objects(mixed ospecs, object who, string root_context, int flags);

The expand_objects() function parses one or more ospecs, which can be passed in as a string or array of strings. The ospec is essentially a query string that gets tokenized and eventually blown out into a list of target objects. The specifics of the query language will be discussed in the next section.

The second argument provides some context to the parser by providing it the object that is performing the parsing. While this will usually be a player object, it could easily be something else; for instance, the logger allows you to put an ospec in logger.properties files for printing log messages to a player's screen. In the context of logging, there is no command giver, so the logger passes this_object() to expand_objects() instead. Whatever the object is, it will be used to do things things like resolve relative paths in filenames, or query the current context string.

I've already described what the root context does. If there is no root context to look at when the parser finishes backtracking, an empty set will be returned. If, in the course of backtracking, it finds the root context before it's gotten all the way to the beginning, it will stop at the root. I'm still not 100% sure on this design decision. It's meant to allow command objects to constrain what target objects the player may refer to. Since the look command sets the root context to "(here,me)" and try to "look /some/obj#1", the command will fail if /some/obj#1 isn't present. However the player could just clear their current context, in which case it'll backtrack all the way to being used as a context-free token and /some/obj#1 will be returned. So, in this case, only being able to look at objects which are present is more of a suggestion than a constraint.

The flags argument is a bitvector of zero or more of the following flags:

* MATCH_DETAIL: allow detail ids in spec (I might make this the default)
* UPDATE_CONTEXT: update current context with the new spec; if multiple specs are passed, they will be globbed together
* LIMIT_ONE: only return a single object; more optimization around this flag is needed
* MATCH_BLUEPRINTS: when using a filespec to find matching objects, include blueprints in the result; only clones are returned by default
* IGNORE_CLONES: when using a filespec, do not include clones in the result
* STALE_CLONES: when using a filespec, include clones of previous blueprints

The return value of expand_objects() is an array of the form:

     ({ ({ object target, string id, string detail_id }), ... })

where target is the object which matched the spec, id is the term used to perform the match, and detail_id is a matching fully qualified detail_id found on the end of the spec, or 0 if no detail id was provided. Remember that if you do provide a detail id, that id must be found on the target object for anything to be returned.


While I've intended the new object parsing system to be very flexible, is will require more validation to be performed by the command object post-expansion. For instance, I can now do things like "drop backpack.bag.apple" to remove something from a container and drop it in a single command, but the drop command has to know to check all the containers between you and the apple to make sure they're open first.

AcmeOSpec will probably still make a comeback for author mode. This new spec is really designed for people moving around the game and interacting with objects as players. In fact, the object name and file name support in this ospec will most likely go away before the final version, but it comes in handy while debugging stuff. We still need to think about what constraints to put on players that may be using a more advanced ospec as to what they can and can't refer to, since wizards may want some objects in their areas to be "hidden" to players.

Next steps have to do with what happens immediately before and immediately after object expansion. Generally right before object expansion is when you parse the individual ospecs out of the complete argument string. Every command should allow for a standard unix-like usage: 'verb opts arg1, arg2, ...' and args can be classified as an ospec, file spec, literal string, etc. If we can do this, I imagine it'll make scripting a lot easier. But in addition to the standard usage, you may also want to allow a natural language variant. For example, the give command takes two arguments, the objects to give and the object to give them to. The standard variant might look something like: give -q devo "red apple". Note that since args must separated by spaces, any ospecs you use which contain spaces will need to be quoted. This might be confusing to a new user unfamiliar with working on the command line. To make things easier, you also support a natural language variant: quietly give red apple to devo. Now you can infers the quotes for them by the injection of the 'to' arg (notice our magic NL parser also knows how to convert adverbs to command line opts). What you want is for args to know enough about each other that the standard parser and the NL parser can return the same data structure.

The next phase after that is validation. I mentioned earlier that the new flexibility afforded by object expansion also means more validation. Right now all the validation is being done in the command code, but there has to be a way to abstract this out to a library. This does a couple things. First it means people don't have to keep cutting and pasting code between commands. More importantly, though, it standardizes behavior and provides the user with consistent feedback (i.e. messaging) they'll be able recognize even if it's their first time using a particular command. For example, lets say you wrote some validation code for the get and drop commands, and now you need to write a give command. Well some of the that stuff that's true of me dropping something is also true of me giving something: am I carrying it? is it in a closed container? Likewise some of the stuff that's true of you getting something is also true of you being given something. So you extract the validation code out into some common functions, and what you get is that the give command now displays the same error messaging as the get and drop commands. That's huge in an environment where the only surface features objects may share are strings of letters and sometimes color.
