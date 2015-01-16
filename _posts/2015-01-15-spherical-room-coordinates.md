---
published: true
category: blog
layout: blog
---
In my post about MobileMixin I mentioned that rooms will now be plotted on a 3d Cartesian grid, so every room will have coordinates and needs to enforce some referential integrity to make sure all the exits make sense, etc. I also made an alternate API that uses a spherical coordinate system instead of Cartesian. These work like polar coordinates in 2d space if you remember your highschool math. Every position is expressed as some angle and distance from the origin.

<!-- more -->

It doesn't matter now because there's no content and nothing that makes use of room coordinates, but it provides some interesting options down the road. In the Cartesian system, every room has to snap to the grid and all the exits need to line up just right to make moving in the cardinal directions possible. The spherical system is much more fluid. When rooms can have real spacial relationships with one another, you could actually use the gabbo content as a model for drawing out spaces in graphical games that use 3d engines. 

Maybe more interesting, though, you can take other things that are plotted out in 3d space and model them in gabbo. For instance, you could crawl Google street view, which already plots out streets in discrete blocks like rooms on a mud, and identifies what establishments exit out onto those streets; you could use that data to stub out an entire neighborhood in gabbospace with the click of a button. Then once it's modeled in gabbo, use that to seed models in other games, like Minecraft, or an FPS, or whatever you can think of really.

Spherical coordinates are only possible because people will now be using editors to build content instead of writing all the code by hand. It might be some time before I have an editor to support this alternate system. I need to acquaint myself with WebGL and figure out a convention for naming exits in lieu of the 8 cardinal directions. We also need to figure out how angle in distant is communicated to the user, since developers will now be able to implement game mechanics. As a player, it would suck if something like fatigue was variable based on distance traveled, but then have no way to tell how far you are from your destination.

So yeah, there's a lot of supporting work that needs to be done before we can put a system like this to good use, but I thought I'd post about it anyway since I think it's cool.
