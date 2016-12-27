---
published: true
category: blog
layout: blog
---
I'm getting pretty close to finishing everything that's needed to actually connect to the game, register a new player and get them to their workroom. It doesn't sound like much, but a lot of decision making went into how everything is modeled and I'm confident enough at this point it won't change that I decided to post about it. I wrote a rough outline of the whole process that I'll take you through. The list is organized more by how things happen in my head than how the code might actually be structured.

I won't set it up too much except to point out that I've designed the code to easily support "multiplaying," which I use in quotes because I'm still trying to figure out exactly what that means. The great thing about living in a virtual world is we really can be multiple places at once if we want to be. I can imagine being logged into the game and wanting to have an avatar going at home taking care of the family, meanwhile a separate avatar is going at work, doing whatever. That's one kind of multiplaying, but there are others. For some game-lovers, it's also easy to see wanting to have not just multiple copies of oneself in the game, but multiple actual selves. To support this, I've introduced (at least) two levels of accounts: a "user" account, and a "player" account underneath it. When you login in you get a user avatar with no environment in particular, which can then also control a separate player avatar moving around out in the world. Right now you're only allowed one player per user, but that's easy to change. This separation lends itself well to the idea of encapsulation; the user avatar only depends on platform-level code, while the player avatar is supplied by flavor-level code. The backbone supporting all this is the session graph, which allows you to easily forward, redirect, and suppress commands and messages between sessions, and share sessions between connections.
<!-- more -->
Now to break it all down...

### somehow the vital connection is made

> 1. MasterObject->connect()
>    1. clone(LoginObject)
>    2. setup login commands via CommandGiverMixin (including register command)
>    3. LoginObject->logon()
>       1. ConnectionTracker->new_connection()
>       2. suspend prompt output and command input (and local echo)
>       3. detect terminal type
>       4. display welcome
>       5. prompt and enable commands

When someone connects to the server, the master object immediately hands control over to the platform's login object. There's not really much you can do before this point because you don't have any context about the connection being made. At this point, the login object has a few tasks: it must register the connection, detect your terminal type, welcome you to the game and let you know that it's ready to receive commands. Unlike the login object EotL which guides you through a sequence of input_to()s, the gabbo login object is a full-fledged command giver. In other words, in order to actually login, the user must execute a "login" command. That's getting ahead of things, though.

The first thing the login object does is inform the ConnectionTracker that a new connection has been made. The connection tracker is responsible for maintaining up-to-date information about the connection state, including which interactive object every connection is bound to. This means even if the user's connection is transfered between different avatar objects, information about their connection is preserved. This info includes things like connect and disconnect times, active session (more on sessions later), and pending telnet negotiations.

Once the connection tracker knows about the connection, it can start handling the user's telnet negotiations. At this point, the login object suspends command input, prevents any prompt output, and turns off local echo while it waits for the terminal type negotiation to happen. The terminal type is necessary to know how to display the welcome screen, since different messaging is used for connections via terminal apps vs. connections via the web client. After displaying the welcome, the user's prompt is restored and the login object resumes normal command processing.

### intro to messaging: the welcome screen
> 4. display welcome
>    1. MessageLib::system_msg(login, message, topic)
>    2. PostalService->send_message(login, message, topic)
>       1. LoginObject->try_message(message, topic)
>          1. SensorMixin::try_message(message, topic)
>          2. LoginObject->render_message(message, topic)
>             1. SensorMixin::render_message(message, topic)
>                1. SensorMixin::get_term()
>                2. TopicTracker->get_renderer(topic, term)
>                3. DefaultRenderer->render(term, message, topic)
>       3. efun::tell_object(rendered_message)
>       4. LoginObject->on_message()
>          1. SensorMixin::on_message()

I'm actually writing this section last because I thought a dissection of messaging might be too much for this post, but it's a pretty big omission so I thought I should at least try and summarize. In this section we're backtracking one step to the step where we display the welcome screen (and also the prompt, and future command feedback). There's actually not much to it once you get past the interplay between the different APIs. The de facto method for sending a message to a player is via PostalService->send_message(); and there are some convenience wrapper functions for common message types in MessageLib. To send a message you must specify a destination object, a "topic," and the message itself, which is made up of a message string and a mapping "message context."

When PostalService gets a send_message() call, it first calls try_message() in the destination object -- in our case, the LoginObject. It is up to try_message render the final message or throw an exception if the message can't be delivered. The try_message() function is provided by SensorMixin, a mixin for "sensing" game events via receiving messages. It also provides render_message(), which gets a renderer object from the TopicTracker for the given topic and terminal type. Topics are just a way to group messages together into channels. You can form topics by membership in user groups like guilds or forums, interactions with your environment or specific objects, or ad-hoc topics for on-the-fly group chat. That's a whole other post, though. Currently there's just one renderer for all topics and terminal types, but this should expand rather quickly.

Once the final message string is rendered, the message is actually delivered via efun::tell_object(). This is followed by a call to on_message(), also provided by the SensorMixin. Right now it doesn't do anything but it might come in handy later for things like logging or scripting. Now, onto the register command...

### &#96;register devo&#96;

> 1. HookService->command_hook(command, command_giver)
>    1. LoginObject->do_command(command)
>    2. CommandGiverMixin::do_command(command)
>    3. CommandController->do_command(command, verb, arg)
>       1. parse options and argument list out of arg string
>       2. display syntax suggestion if arg/opt combination doesn't match a valid syntax
>       3. for all fields, validate field value or prompt for corrections
>       4. run form-level validators
>       5. .bin/register->execute(model)

At this point the register command may be used. The command takes one argument, which is the username to register. I could write an entire post on how command processing works (and sort of already have), but I'll try to summarize the important parts of what goes on under the hood. When the driver receives the command from the client, it runs the command hook in HookService passing it the command and the command giver, which in this case will be the login object. HookService then calls do_command() in the login object, which is supplied by the CommandGiverMixin.

CommandGiverMixin parses the command into a verb and the argument string. It then looks at the all the available command for a matching verb. When it finds one, it loads the command controller associated with that verb and hands over control to the controller. Command controllers should be put in a zone's .bin directory and in most cases, should inherit from CommandController. CommandController is an abstract class which provides a basic do_command() function for gathering and validating input from the user, and calls an execute() function which must be implemented by the inheriting program. It first tries to read all command fields provided by command-line options and arguments, but may also prompt the user for field values that are missing from the arg string (like the password field) or simply invalid as input. There's also a feature where commands can specify more than one combination of required args and options to designate different "syntaxes," and the command controller will attempt to match to the best one. If there is no match, we can use the syntax definition to automatically display usage suggestions. Once all required fields have been filled out, the model passed to the execute() function pending any form-level validations that may be required.

### to create a user
> 5. .bin/register->execute(model)
>     1. UserLib::create_user(username, password)
>        1. copy skel dir to user's homedir and apply templates
>        2. UserTracker->new_user(username)
>        3. write password to password file

In register.c proper, the execute function is divided into two parts; the first part is the actual creation of a new user in the system. Remember that all of our validations have passed at this point, so we should have a green light to do everything necessary to setup a new user without conflict. First, copy the skeleton home directory to the user's home directory and apply templates for things like default domain config, flavor config, etc. Next, register the username with the user tracker, which will give us a unique user id that we should used to identify the user in most situations, as the same username could be reused across multiple account creations and deletions. Lastly, write the password (again, already confirmed by the validators) to the user's password file. As of the time of this posting, every user maintains their own password file in their own home directory, though that may change in a future version of the code.

### attach_session pt. 1: setup user session
> 5. .bin/register->execute(model)
>     2. UserLib::attach_session(login, user)
>        1. UserTracker->query_last_session(user) or UserTracker->new_session(user)
>        2. SessionTracker->query_avatar(session) or clone(UserAvatar)
>        3. catch(UserAvatar->try_descend(session))

The second function of the register command, after user creation, is to actually create a new session for our user and get them in the game. This process is a lot more involved, because we'll also be creating a whole new "player" for the user along the way. The first thing we need, though, is a session. The session is how we maintain all the relationships between users and players and connections and avatars. This attach_session() function we're looking at from UserLib can be used for either new users logging in for the first time, or users using the login command for future sessions. To support both, it looks at the user's last session and sees whether it's in a resumable state. There are four session states: new, running, suspended, done. If the last session is in a running or suspended state, it attempts to reconnect. Otherwise, a new user session is created. User sessions are always root-level sessions, e.g. they don't have any supersessions. More on this later.

Once we have a session, we look to see if that session already has an avatar attached to it. If not, we clone a new user avatar. There isn't anything special that makes it a "user" avatar, other than it only depends on platform-level code and thus has a smaller feature set than the player avatar. Specifically, it gets CommandGiverMixin for issuing commands, SensorMixin for receiving messages, ShellMixin in case you need to troubleshoot something at the player level, and SoulMixin because why not.

Assuming all that gets setup okay, the register command will now try to "descend" the command giver into their user avatar. The user avatar can prevent this by throwing an exception in its try_descend() function, otherwise it is expected that the register command pass along the result of try_descend() as extra arguments to the on_descend() function upon successful descension. We will use the on_demand() call to perform a second "inner" descension from the user avatar to the player avatar, but first let's take a closer look at the try_descend() function in the user avatar.

### try_descend: create player and setup player session
> 3. UserAvatar->try_descend(session)
>    1. AvatarMixin::try_descend(session)
>    2. PlayerTracker->query_players(user) or PlayerTracker->new_player()
>    3. if player's last session is resumable, use existing session, start room and avatar; if not then start a new session for subsession, use last room of last session for start room, and clone(FlavorTracker->query_avatar(get_flavor(start_room))) for avatar
>    4. catch(PlayerAvatar->try_descend(session))
>       1. AvatarMixin::try_descend(session)

The try_descend() function is provided by any object inheriting AvatarMixin, but the specific job of the user avatar's try_descend() function is to attempt to setup everything needed to later descend the user into their player avatar, or abort now if that can't be done. At the current time AvatarMixin::try_descend() does nothing, but we should still start by calling it for good measure. The first real step of this process is to get a player for the user. The user to player mapping is maintained by the PlayerTracker; if no existing players are found for the user, a new one is created. Like for the user avatar, we first check the player data for an existing session we can resume. If that exists, then we'll descend to the avatar that's already in the game. If not, we need create a new session, clone a new avatar, and attach the newly cloned avatar to the newly created session.

The path of the avatar to clone is based on the player's last session, which should have recorded the avatar's last room before it was ended. If there is no last session as is the case with registering a new user, the user's workroom is used by default. Based on their start room, we can determine the flavor of the avatar we need to load. FlavorTracker is responsible for mapping flavors to player avatar paths. Currently, there's only one flavor and it uses the same avatar path for everyone.

If we successfully get a player avatar cloned and its start room loaded, then the user avatar must call try_descend() in the player avatar to make sure it's ready for descension on its end. Currently all the player avatar does is direct to AvatarMixin::try_descend(). Assuming that all goes okay, try_desend() returns the player session id, the player id, and the start room to be consumed by on_descend().

### attach session pt. 2: switch the connection
> 2. UserLib::attach_session(login, user)
>    4. SessionTracker->resume_session(session)
>    5. exec from login to user avatar
>    6. ConnectionTracker->set_interactive(connection, avatar)
>    7. SessionTracker->connect_session(session, connection)
>    8. UserAvatar->on_descend(session, subsession, player, start_room)

Now that everything under try_descend() has passed, we can actually perform the descension. This means changing the user session state to "running" and calling the exec efun the transfer interactivity from the login object to the user avatar. Doing that requires we update ConnectionTracker with the change in object interactivity, and we inform SessionTracker that the new user session is now a "connected" session. At this point, I don't see a reason to "connect" any session other than user sessions, and let CommandGiverMixin and SensorMixin route things up and down the avatar path accordingly, but there's nothing stopping you from doing that. Perhaps this feature will be used to do things like permissions, or prompts in future versions of ShellMixin. After all that's done, we'll let the user avatar know descension is complete by calling on_descend().

### descend from login to user avatar
> 8. UserAvatar->on_descend(session, subsession, player, start_room)
>    1. AvatarMixin::on_descend(session)
>    2. SessionTracker->resume_session(subsession)
>    3. SessionTracker->query_avatar(subsession)
>    4. PlayerAvatar->on_descend(subsession, start_room, player)

Since try_descend() did all the work of setting up descension from the user avatar to the player avatar, the on_descend() function can be pretty thin. We start by calling AvatarMixin::on_descend() with the user session, so it can be queried later with AvatarMixin's query_sessions() function. Next we change the session state of the player session to "running," and invoke the on_descend() function in the player avatar with the start room and player id we got from our earlier call to try_descend().

### descend from user avatar to player avatar
> 4. PlayerAvatar->on_descend(subsession, start_room, player)
>    1. AvatarMixin::on_descend(subsession)
>    2. configure mixins
>    3. restore configured prompt
>    4. load user-defined command spec
>    5. move to start room
>    6. run some sorta login script. it'd be cool if you could run descend.cmd and pass last session state as an arg, then the descend script could run login/reconnect or whatever it wanted

This is about as far as I've gotten. The try_descend() in PlayerAvatar has a lot of stuff to do since this is where most of the game's feature set comes from. We can use the player id to restore state or configure mixins, for instance giving the player avatar a nickname in NameMixin that's the same as the player's username. This is where we move the player avatar to their start room. This is also probably where you want to import user defined commands from custom command spec in your home directory, or run a login script.


So let's summarize. We've introduced several "tracker" objects that together begin to form our business model: TopicTracker, ConnectionTracker, SessionTracker, UserTracker, PlayerTracker. We've also mentioned a few mixins that support login objects, and user and player avatars: AvatarMixin, CommandGiverMixin, SensorMixin. We touched upon messaging and the PostalService. We've seen how the try_descend() and on_descend() functions in AvatarMixin can help prepare and coordinate descension between avatars; and how CommandController implementations work with CommandGiverMixin to perform the routine parts of command execution. Lastly we've gotten a peak into how a user's session graph facilitates playing multiple players per user and multiple connections per session.

There's still a big question we haven't really addressed, and that has to do with the actual experience we're trying to create with all this stuff. Maybe a little bit with the coexistent home avatar and office avatar thing, but there's a lot more to explore there. For a later post.

