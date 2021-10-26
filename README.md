# Getting over... Ludwig
This is the full source, code and assets, for my Ludwig Jam entry.

The game is written in lite-C with only one (for now as there's no sound - I ran out of time) very basic XInput DLL written in C++. It's the same file Pogostuck uses just with a different name, here it is called ackXInput.dll. There are multiple other DLLs the engine may or may not use, depending on the system (experience shows this, running games made with this engine on hundreds of thousands of other machines).

Getting over... Ludwig runs on the A8 engine (same as Pogostuck) which is part of the (3D) Gamestudio toolset: http://www.conitec.net/english/gstudio/index.php

Gamestudio A8 can be downloaded here: http://www.conitec.net/english/gstudio/download.php

It's an 80MB setup file. From that setup file you can install a free version, a demo or the full version.
The free version does not support a bunch of features, such as plug-ins. As a result, the free version CANNOT be used to successfully compile and play this game (it will compile but then give errors when the app window opens).

There is a so called Extra version for 25Eur and a Pro version for 199Eur. I use the Pro version. The Extra version is missing Isometric rendering and other things (not sure what they mean with Render-to-texture, in particular if you cannot use render chain setups without this). Therefore, the game will not function right with this version either.
Version comparison: http://www.conitec.net/english/gstudio/order8.php

The setup file allows you to install a demo version. Counting old Laptops and PCs I have 5 machines at my place but sadly no one on which Gamestudio was never installed. I tried to run the demo but no luck ("trial period expired").
This demo might be sufficient to launch this game without issues.

# How to finally compile and run the game

After you have installed let's say the demo version, copy this repository to some subfolder (for example in projects) of your Gamestudio installation folder.
Copy over ackXInput.dll from this repository (located in acknex_plugins) to acknex_plugins of your Gamestudio installation folder.

Now open SED, then load gettingOverLudwig.c (not the WDL file). There are 3 triangle launch buttons at the top bar of the program. Click the left one that says "Run current script" on hover-over.

The game should launch successfully. If you see a mostly black screen with dimly lit objects instead of golden honeycombs filling the screen then the engine did not launch as the Pro version (presumably).

HOWEVER... this is a run-on-the-fly compile which does NOT generate an executable or portable game. For that you need to go to File -> Publish Main Script (I suggest going to Options -> Preferences -> Environment first and checking "Use current script for Run/Publish" first). I bet this feature will be disabled in the demo version though.
So... sadly the only way to compile your own executable of this game (unless I'm overseeing something) would be to buy the Pro version of this engine.

The folder [gettingoverludwig.cd](gettingoverludwig.cd) contains a fully built game including all required files.

I vouch for the already compiled binaries (on itch and included in this repository) with my name, that they do not include malicious code or anything that could be harmful to Ludwig's PC or the stream. Here's my twitter: https://twitter.com/realSuperku

When in doubt and you understandably do not want to spend money on this antiquated engine please ask Ludwig if he wants to play a game made by "Superku" anyway - he knows me to some degree.

Thank you.

DM me on Twitter if you have any questions.
