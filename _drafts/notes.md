I don't have any new code or features to report, but I thought it'd be a good time to do a roundup of what's been done so far.

I began this latest iteration of the project without a lot of expectations. The idea of using a MUD or something MUD-like as the foundation for a gamification engine had been with me for some time, but what really sold me on building an actual LDMud mudlib was that I could theoretically run it inside End of the Line, as a sort-of game-within-a-game. This provides me with a captive, albeit small, audience right from the start. That small audience, to me, is more an asset than a liability. Trying to attract a large number of players upfront is counterintuitive to the kind of community-based game development I'm striving for.

First, you don't want too many cooks in the kitchen, especially at the beginning. From the time the initial model is built and you start introducing game elements such as points and levels, economic and political systems will emerge which need to be examined and codified for the sake of stability. While there's certainly something to be said for a having large sample size, if you can't put systems in place which function for dozens of users, how can you expect to build something that works for hundreds, or thousands?

I'm no industry insider, but there seems to be a lot of attention given to how big a game's launch is as a measure of its success. Even when titles go early access, those sales numbers may be used to retain funding and market the final release of the game. My point is that there seems to be incentive built into the industry to attract as many players as early as possible, and numbers ain't everything. Personally I blame gaming's older brother in entertainment, the film industry, where nearly all movies are made or broken by how they perform opening weekend. The difference is that when you exhibit a film, the audience isn't required to make any choices other than "watch" or "don't watch." They can't affect the narrative, so there's little point in trying to maintain any sort of feedback mechanism. Games like the ones I'm trying build, however, can't do without it.

Also, EotL may not have a lot of players left, but the ones that have remained are dedicated, loyal, and they bring with them the experience and culture that's shaped the game's identity. Even without the legacy of EotL though, the same could be said of many early adopters. Just to be able to find your game before it's built an audience, the early adopter must have a heightened awareness of the gaming landscape. They must also be open to risk and experimentation, and be willing to invest a part of themselves in something new and untested. These qualities amount to a more sophisticated user, which what you want when trying to put sustainable systems in place. Every game, to some degree, is going to make use of common conventions, and the less time spent on introducing these conventions to the uninitiated, the more time can be spent developing the things that make this game unique. You can always go back and fill in the gaps later when you're ready to make the game accessible to a more general audience.

For all these reasons, using EotL as a proving ground for larger design concepts just seemed to make sense. It does produce a technical challenge, though, since I have to integrate the lib into an existing environment. My first step was just to determine the smallest amount of code required to have a working lib, with the estimation that the smaller the footprint, the easier integration should be. So I downloaded the latest version of the LDMud driver and created an empty directory named "mudlib". Once I'd added just enough code to support booting the game and to logging in, I created the github repo and pushed my first commit. That was July 11th.

I'll talk a little bit more about the choice to build a MUD in the next post, as it pertains to the text-based nature of the medium.

-----

Before going on, I want to talk a little bit more about the decision to build a MUD. One of the questions I get from people is, are people really going to play a text-based game? Again, I see the MUD's roots in text as an asset, not a liability.

First, rich content takes time to create. Sentences can be written in more or less real time. [Ninety-three percent of all tweets are text-based](http://www.quicksprout.com/2014/03/05/what-type-of-content-gets-shared-the-most-on-twitter/), and it's not because people like reading them more. [Tweets with images receive 18% more clicks, 89% more favorites, and 150% more retweets than those without](https://blog.bufferapp.com/the-power-of-twitters-new-expanded-images-and-how-to-make-the-most-of-it).



Think about what text is. All English text is comprised of twenty-six letters, two cases, and some various punctuation. That's it. Now think about what a picture or a movie is.


Also, words are going to be there whether we want them to or not


[When Coca-Cola recently offered the option to its corporate employees to drop voicemail in lieu of text messaging, 94% of them chose texts](https://www.mobilecommons.com/blog/2015/01/coca-cola-replaces-corporate-voicemail-text-messaging/).




A few legal speed bumps. First, I won't build anything that EotL isn't allowed to use for free. I'll go to academia if I can't find that deal in the marketplace.


I would like if it it remained open source, but I'm cool with corporate licensing if it's reasonable.


What you're investing in isn't really software, it's participation in a new kind of machine. Remember, this thing is a model, and investors are going to be modeled into the system just like everyone else. If you don't understand the significance of this, it probably doesn't make sense to invest.


I would prefer it if any implementation whose subclasses are all open source also got to use it for free.




A solider class that starts in the barracks; might appeal to devs who are there to follow orders.

A student class that starts in the dormitory; might appeal to devs seeking to learn new things.

A tourist class that starts in a hostel; for contractors or devs who switch teams a lot.

A homeless class that starts by stepping off a bus; for those who got big dreams and don't mind a challenge.
