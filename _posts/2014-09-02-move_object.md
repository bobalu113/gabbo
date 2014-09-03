---
published: false
---

I decided I would break my coding deadlock today so I picked somethin to make that I could bang out relatively quickly: the move_object hook in master.c. Up till now I've just had a stub that did a set_environment() without without any signal functions or living initialization. For the most part I just went off EotL's hook which has been functioning without incident for some time. I did make a few changes though...

### allow null destination ###
There are a few times I've wanted to move an object off someone and needed a place to put it, because EotL doesn't let you call move_object with 0 as the second arg.

### implemented 'release' applied lfun ###
I'll take some measurements later about how much weight it actually adds to the move

### ###

### other crap ###
1. Took out any passing of the move_object "mover" to the signal functions. I think this is just some legacy thing because previous_object() accomplishes the same thing.

