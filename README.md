<div align="center"> अहं सर्वस्य प्रभवो मत्त: सर्वं प्रवर्तते | </div>

<div align="center"> इति मत्वा भजन्ते मां बुधा भावसमन्विता: || 8 || </div>

Geta: [Chapter 10, Verse 8](https://www.holy-bhagavad-gita.org/chapter/10/verse/8)

KarmaEngine
===========
<img width="1440" alt="Screenshot 2023-02-15 at 15 45 40" src="https://user-images.githubusercontent.com/2173654/218999401-fe78d0e1-7bff-45ca-a0ab-cb6ab2ce31aa.png">

A cross-platform Game Engine for practical learning and research purposes.

Clone
------
This repository contains submodules, therefore use ``` git clone --recurse-submodules https://github.com/ravimohan1991/KarmaEngine.git``` to recursively download the submodules.

Developing
-----------------------

Karma rocks on ```Linux```, ```MacOS```, and ```Windows```.  The architecture is liberal enough to support Mobile, perhaps, in the future (I won't count stars for that though). 

Make sure to install [CMake](https://cmake.org/) for null nuiances.

#### Global Software Requirements ###
* [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) should be installed depending on your choice of development platform

### Windows ###
#### Software Requirements:
* [Visual Studio](https://visualstudio.microsoft.com/)

Open the command prompt in the root directory and type the following commands
```
mkdir KarmaLightHouse
cd KarmaLightHouse

cmake ../
```

And that is it!  Visual Studio solution shall be generated in the folder ```KarmaLightHouse```.  Also make sure to update the Windows SDK to [latest](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) 


### Linux ###
#### Software Requirements:
* [X11](https://www.glfw.org/docs/3.3/compile.html#compile_deps)
* [Vulkan SDK](https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html)

Karma has been developed and tested on Ubuntu 22.04 but should work on any Linux platform. To build, go to the root directory and type
```
mkdir KarmaLightHouse
cd KarmaLightHouse
cmake ../
cmake --build .
```
I'd recommend using [QTCreator](https://www.qt.io/product/development-tools) for development purposes because not only it is cross-platform IDE but also Industry level heavy code lifter. Furthermore try using OS package managers for Vulkan SDK for proper tracking and clean assignment.


### MacOS ###
#### Software Requirements:
* [Xcode](https://developer.apple.com/xcode/) a must!

Karma has been developed and tested on macOS Monterey and should work on any upper version. To build, go to the root directory and type
```
mkdir KarmaLightHouse
cd KarmaLightHouse

cmake -G Xcode ..
```
This shall generate full-fledged Xcode project file bundle.  Then you are at the mercy of the IDE!

Patreon
-----------
Help me in supporting FOSS by being a member of this [page](https://www.patreon.com/FreeandOpen). No strings attached.

Huge thanks to:
Shiwali Mohan

References
-----------

### Books
* [Game Engine Architecture (Second Edition)](https://www.gameenginebook.com/)
* [Game Coding Complete, 4th Edition](http://www.amazon.com/gp/offer-listing/1133776574/ref=as_li_tl?ie=UTF8&camp=1789&creative=9325&creativeASIN=1133776574&linkCode=am2&tag=gamecodecompl-20&linkId=YTOZOL6OXF45PZSQ) ([Discussions](https://www.mcshaffry.com/GameCode/))
* [Game Programming Patterns](https://gameprogrammingpatterns.com/contents.html)
* [Some Text-Books Curation](https://github.com/kurong00/GameProgramBooks)

### Youtube Channels
* [The Cherno](https://www.youtube.com/user/TheChernoProject)
* [Travis Vroman](https://www.youtube.com/user/barzahd512)
* [ThinMatrix](https://www.youtube.com/watch?v=f3Cr8Yx3GGA)

### Resources and Misc
* [deoxygen integration](https://github.com/satu0king/Github-Documentation-With-Doxygen)
