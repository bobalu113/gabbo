---
published: true
category: blog
layout: blog
---

I tried to break the coding deadlock today by implementing the move\_object driver hook in master.c. And I did to a certain point. I picked move\_object because it's simple and figured I could bag it in a day. And I was right, to a certain point. I was all committed and pushed to origin and started to write about it and I just then realized that I did it wrong and I'm gonna have to rewrite a large amount of it tomorrow.
<!-- more -->

Basically, I started with the implementation on EotL. Some bright people have worked on it and I should have just stuck to what EotL does, but I decided to get fancy. Basically I decided to let people move\_object() something to a null environment; set\_environment() allows you do this and I see no reason why move\_object() shouldn't. Since now the destination could be 0, it changes the types of values the signal functions need to handle. To keep things consistent, I also made it so the item or destination could be dested along the way and the 0 would still be passed along to the signal functions. Then, since I didn't have to worry about null checks except for as call\_other() targets, I could line up my code all pretty with a chain of #'&& blocks instead of the nested #'? blocks that EotL uses. I realize that's a dummy reason to redesign the whole thing but lambda closures are just so, so ugly.


Well anyway, as it turns out, the driver won't even let you pass a 0 value as arg 2 to move\_object(), so none of the code I changed even mattered. Now there was no real benefit to making the signal functions have to check for 0 in the case of dested objects when there's no way they'd get a 0 under normal conditions. Also when I started thinking about it, reporting things for dested objects doesn't even really make sense. The whole thing was pretty stupid and that's what I get for taking most of the last 3 weeks off to play gamez.


However, as far as why I'd want to be able to move something to null environment, I do have my reasons. First, that's where things start out and to not be able to put them back there is silly to me. Double silly is that set\_environment() lets you do this but move\_object() doesn't. But moreover, there are times where you need to pull an object out of it's environment and stash it somewhere else temporarily, and most of the other options for doing that suck. I'm also not always a fan of putting people in The Void when things go wrong. Yeah, sometimes you want to dump them in an airlock and let them exit back out to the game, but probably more often you want to preserve their state as much as possible, to try and figure out what went wrong. 


There will be none of that for now, though. I tip my hat to you H_MOVE_OBJECT0, you have gotten the best of me. I'll post again when I finish it for real. If you want to see the pretty (stupid) version, [here it is](https://github.com/bobalu113/gabbo/blob/17143a80a4af403630a70bc9186684ef3881a61d/mudlib/secure/master.c).
