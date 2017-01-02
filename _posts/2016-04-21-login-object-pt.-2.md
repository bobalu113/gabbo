---
published: true
category: blog
layout: blog
---

This weekend I made a dent in command processing. The code I wrote is pretty boring, just marshalling an XML document into an LPC data structure, but some of the decisions that went into are kind of interesting.

I decided a while ago that I would be writing my own command router, as opposed to using the one built into the driver. The whole add_action() and init() thing isn't super elegant; I'd like more accountability between command givers, command providers, and command controllers, to promote command modularity and uniformity, and to make more metadata available to players and developers about commands and their availability.

<!-- more -->

Right now the only commands that can be added to an object are internal commands, ones made available to the command giver by the command giver itself, through mixins and other modules they inherit. Each mixin can initialize a global variable _command_imports to a collection of "command spec" files. When the CommandGiverMixin is initialized, it combs through variable_list() of THISO, looking for variables with that name and building a master list of all imported commands from inherited objects. This is a little bit of a cheat since it's not easy to do this by function calls alone. Calls to functions in inherited programs are validated at compile time; you can't specify the inherited object with a variable as in: <code>string program = "my_mixin"; program::get_commands();</code>. This means you can't use inherit_list() to do the aggregation, you'd have to manually hardcode a call to get_commands() for every inherited program, or split it up into different functions using some kind of naming convention. The variable_list() method isn't ideal, but it's better than an awkward an error-prone naming convention.

Other than the use of this global variable, commands aren't actually grouped by module, they're grouped by command spec, though I imagine there will be a one-to-one relationship between modules and command specs in most situations. The command spec for more than just putting commands into logical groups. It maps verbs to their command objects, which I'm now calling command "controllers", and maps command line options and arguments to what I'm calling "fields". It lets you declare one or more "syntaxes" for a command, which specifies how the command line should be parsed; for instance using explode_args(), sscanf(), regexp() or parse_command(). Fields make up the "model," which is just a mapping of field names to their values and is passed to the execute() function of the controller. You declare them with a data type, and a type conversion is done from the string values parsed from the command line before they are added to the model. The command spec file also lets you declare any field- or form-level validation that needs to be done first, and specify how and when the user should be prompted for values using input_to().

Here's an example from avatar.xml, for the register command:

<code><pre>&lt;command primaryVerb="register" controller="register"&gt;
  &lt;validate validator="passwords_match"&gt;
    &lt;fail&gt;Passwords do not match.&lt;/fail&gt;
  &lt;/validate&gt;
  &lt;fields&gt;
    &lt;field id="username" type="string" required="prompt"&gt;
      &lt;validate validator="not_empty"&gt;&lt;/validate&gt;
      &lt;validate validator="max_length"&gt;
        &lt;param value="12"/&gt;
        &lt;fail&gt;Maximum username length is 12 characters.&lt;/fail&gt;
      &lt;/validate&gt;
      &lt;validate validator="available_username"&gt;
        &lt;fail&gt;That username already exists.&lt;/fail&gt;
      &lt;/validate&gt;
      &lt;prompt maxRetry="3"&gt;Enter your name: &lt;/prompt&gt;
    &lt;/field&gt;
    &lt;field id="password" type="string" required="prompt"&gt;
      &lt;validate validator="not_empty"&gt;&lt;/validate&gt;
      &lt;prompt noecho="true" maxRetry="3"&gt;Enter your password: &lt;/prompt&gt;
    &lt;/field&gt;
    &lt;field id="confirmPassword" type="string" required="prompt"&gt;
      &lt;validate validator="not_empty"&gt;&lt;/validate&gt;
      &lt;prompt noecho="true" maxRetry="3"&gt;Confirm your password: &lt;/prompt&gt;
    &lt;/field&gt;
  &lt;/fields&gt;
  &lt;syntax minArgs="0" maxArgs="1"&gt;
    &lt;args&gt;
      &lt;arg fieldRef="username"&gt;&lt;/arg&gt;
    &lt;/args&gt;
  &lt;/syntax&gt;
&lt;/command&gt;</pre></code>

This sets up a new command for the verb "register". A verb is just the word the command starts with, a word being a string of characters followed by a space or the end of the string. Right now there's nothing to prevent you from adding multi-word verbs, though I might end limit them to a single word for usability reasons. The command controller will be loaded from "register.c" in the same directory as the command spec, and will be passed three fields: username, password, and confirmPassword. There's only a single possible syntax, taking an optional single string arg, mapped to username. If the arg isn't provided, the user will be prompted for a username. Note that the user will also be prompted for the password and confirmPassword, since it is not possible to pass those as command line arguments or options. This is for security reasons; also note the password prompts have the "noecho" attribute set to true, which specifies that the password input should not be echoed to the screen. In addition to the various field validations, there is a form-level validation which ensures that the two passwords match. All of the validators must be present in the controller code.

The whole thing is very struts-y, and if you're noticing a similarity with how web forms work, it's not a coincidence. I expect that the command spec will also be made available to the web client, allowing client-side validations to be done when possible and ensuring a fluid and consistent experience between the CLI and the GUI. I also expect the command spec will become an integral part of the help system; it tells the user a lot about how they can interact with the game at any moment in time. There's a schema [here](https://github.com/bobalu113/gabbo/blob/master/mudlib/platform/.etc/command.xsd), though I've made some changes since it was last updated and there's no way to synchronously do XML validation from inside the mudlib. This weekend I'll be starting on the core command controller, gathering up all the fields from the command line and input prompts and getting them to the validators.
