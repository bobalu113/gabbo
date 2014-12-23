---
published: true
category: blog
layout: blog
---

I got a couple decisions I made early on that I haven't quite made peace with.

The first is canonical names for directories and files. I went with directories being defined with a leading slash but no trailing slash. So you'll see stuff like this in the global include file:

<pre><code> #define ModulesDir  "/modules"
 #define RoomCode    ModulesDir "/room"</code></pre>

This is usually how I represent directories in code, though I'm too tired right now to try and articulate any reason why. Something about this still smells wrong to me, though, so I dunno.
<!-- more -->

The other thing, and this is probably a bit more precarious, is the matter of curly braces and formatting. Since I started writing Java at WSI, I've grown accustomed to same-line after open, new-line before close, new line after close except when opening an else statement. Actually the else statement thing never quite sat right with me, but nonetheless it's primary what I've been writing my code over the last whatever years so I adopted it mostly out of convenience. The more I look at the code I'm writing though, it feels wrong for LPC, and I find myself wanting to put open brace on its own line like back in the old days. I need to figure this out soon, before I write too much code to change it by hand.

To anyone with opinions on either of these things, your comments are appreciated!
