---
published: true
category: blog
layout: blog
---

By far the most involved project of the week was building the LPC API documentation generator. Lesse I started programming for EotL in '94 or so, and in that time I've probably had to deal with a dozen different languages at some point. Deal with, as in, making something that does something. That's a dozen different langauges with at least a dozen different ways of doing things (and that's just Perl!). One of the few constants, though, is how important the documentation was when I showed up to the party. When it comes to programming, I've always been more a grab-a-book guy than a take-a-class guy. Hell, pretty much the only reason I was able to land my first coding job was because I grabbed a copy of the Camel book off someone's desk during the interview.

Point is, I knew it'd be a ton of work to make this thing but it's also a pretty damn important thing to have right from day one if I expect anyone else to wanna help build this thing. Not to mention, if other people do come along and start coding stuff with me, I'm going to want quitmud insurance.

Anyway, it's done. You can see the end product if you click on the [documentation link](http://bobalu113.github.io/gabbo/docs/mudlib/) up top (I did that too!). There are still a few fixmes and futures, and I'm sure we'll find more as soon as I start using it, but overall I'd say it came out pretty good. The main challenge was that we're dealing with LPC, and there's not really a standard or any support out there for it anywhere unless you consider other MUDs but come on let's get serious here. Here we go.

1. #### adopt a doc comment spec
	<p>So my first thought as a quick solution was to use some third-party documentation generator and just adopt their spec since I'm starting from scratch, but it became clear pretty quickly that LPC syntax was going to be a problem. I looked at Doxygen briefly and then into maybe hacking LPC support into an open source app, but before I could really get traction on any of that stuff I found myself already moving onto step 2...</p>
2. #### resolve all the preprocessor directives!
	<p>So building something to do this myself would be like doubleplus difficult if I had to deal with the preprocessor myself, so I started digging into that just to see if it was even an option. I first tried cpp on cygwin (I think that's GNU?), but I had some sort of problem with getting it to read the auto include from the mudlib's /include/auto.h and not cygwin's /include/auto.h. There was no chroot option and if I chroot'd myself then I couldn't read the binary or libraries needed to run cpp.</p>
	<p>I also realized even if I got it working, there was no guarantee that it'd be able to parse LPC correctly. Closures make everything pretty messy. If only Amylaar could see what misfortune he hath wraught on would-be developers of LPC editor packages. I first looked into doing this with Perl since it's text processing, but the two CPP modules I found on CPAN weren't doing it. I think one was just a wrapper for native code so hacking it up wasn't going to happen, and the other was umm, very forgettable. Fortunately I found a well-written Java implementation of the preprocessor with source code freely available, so I grabbed that and was off to the races.</p>
3. #### hack up JCPP
	<p>This could have been a nightmare, but the code actually ended up being well written and pretty easy to work with. [It's even on Github!](https://github.com/shevek/jcpp) I'm realizing now I could have forked their source into my repo and been cool like that, but I wasn't Github savvy enough at the time. It's been a pretty long week.</p>
	<p>As I was saying, this was pretty painless. Did the whole thing in [one commit](https://github.com/bobalu113/gabbo/commit/aa0ae4094a892d7f81a2f468886274fe028bc0d8).
A few details for those keeping score:</p>
	* made it keep c style comments but throw out those unwelcome c++ style comments
	* made it skip over the odious quote in closure literals 
	* add support for a new option which specified the root directory to be used when looking up absolute paths, take that chroot!
4. #### parse doc comments and member declarations out of the source
	<p>Basically it's all Perl from here on out. First strip out everything that's not a doc comment, an inherit statement, a variable declaration, or a function declarations. There was one expected challenge which was supporting the 'default' declaration for functions and variables. Didn't even know that was a thing until I started doing this.</p>
5. #### generated the hypertext
	<p>I basically just view-source'd all the different types of pages from the JDK apidocs on oracle.com, then hacked all my newly scraped metadata into em. They're pretty ugly and a couple things don't work, like switching between frames and noframes. Now that I've discovered the magic of Jekyll and Liquid, though, I wanna rebuild everything to fit inside this here website.</p>

<p>And that's it! The code is pretty atrocious...I tried to modularize things where it made sense but it's pretty much all brute force string handling. Besides restyling for Jekyll, it'd also be nice to come up with a plain-text format for reading inside the MUD itself.</p>