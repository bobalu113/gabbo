---
published: true
category: blog
layout: blog
---
I don't have anything super cool to report, but I have been steadily working on some stuff that's worth updating about. So after getting the demo instance up and running, I started working on permissions and access control. It's kinda funny, I invited some EotLers to log into the server after it went live and the very first thing they attempted to do was 'rm -rf /' Thanks guys!

Everything there is on github, plus rm doesn't have support for a recursive flag, so it's all just harmless fun. But I do want to use the demo instance as a live place where people can start sandboxing stuff, so I'm going to need access controls before I can do any of that. Here's how I think it's going to work.
<!-- more -->
Access lines will be carved up into "domains" and domains are just a collection of zones. Probably to define a domain you'll just put a domain.xml file in the etc/ dir of a zone to group that zone and its subzones into a domain together. This gets tricky when you want to include random dirs on some other branch of the dir tree into a domain, or if a domain gets nested inside itself somehow, so I'm not 100% on all this yet. It kinda depends on how much freedom I think people should have in setting these things up. They're already locked into using dirs to define zones so it might not be so bad.

One thing I am pretty sure on is that an object's uid will be its domain id, and effective uid will be "user@domain" for avatars or just seteuid(getuid()) for everything else. So Chuck the Drunkard might have a euid of "eotl.zone.eternal" and, while you're in the 'eotl' flavor, my euid would be something like "devo@eotl". In these examples, Eternal City defines its domain as a subdomain of 'eotl.zone' and my avatar belongs to the top-level domain of 'eotl' which is maybe applied to everything in the 'eotl' flavor or something. The 'devo@' portion of the euid allows the avatar object to also inherit all my user-level permissions for things like writing to my homedir and zones I control.

Command objects also get some special treatment; command giver objects (e.g. avatars) can lend their euid to a command for a single execution. When the command is done running, its euid will be reset to its domain id. There will need to be support for some security configuration around this stuff; essentially every command added to a command giver could be set to one of: 1) export euid, 2) don't export euid, 3) export euid unless forced. This feature is unique to objects using the CommandCode API, it wouldn't work for ordinary add_action()s supplied by zones (it should be noted that a zone can implement its own commands, though I don't yet know what it looks like for a zone to give a command giver a command on its behalf.

I've put aside the access stuff for time being but I'm still thinking about it. I started working on github integration which I will write about in another post!
