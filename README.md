KarmaEngine (MAI)
===========
![Alt Text](/repositoryfiles/MAI.jpeg)

A cross-platform game engine for learning and research purposes. This branch is concerned with the AI and named Mohan Artificial Intelligence or MAI.

This is the very begining of AI inclusion in Karma. At this stage we are mostly adressing the principles with which MAI is being designed to operate. This basically covers the grounds from the fundamental definition to the decision-making-complexities resulting in engaging gameplay.

Clone
------
This repository contains submodules, therefore use ``` git clone -b MAI --recurse-submodules https://github.com/ravimohan1991/KarmaEngine.git``` to recursively download the submodules.

Building and Developing
-----------------------
### Windows ###
On Windows use Visual Studio 2017 or higher. Next install download the Vulkan SDK from [here](https://vulkan.lunarg.com/) (latest version is recommended) and install it in the default directory ```C:\VulkanSDK\<VersionNumber>```. Make note of the ```<VersionNumber>```. Next open the ```premake5.lua``` located in the root directory and edit the following lines as per the ```<VersionNumber>```
```
...
IncludeDir["Vulkan"] = "C:/VulkanSDK/<VersionNumber>/Include"
...
libdirs
		{
			"C:/VulkanSDK/<VersionNumber>/Lib"
		}
```
and save it. In future I will try to automate this (if it is even possible).

Finally double click on ``` GenerateProjects.bat ``` and that will prepare VS solution in the root directory. Refer to [this](https://vulkan-tutorial.com/en/Development_environment) for troubleshooting Vulkan setup.

### Linux ###
Karma has been developed and tested on Ubuntu 16.04 but should work on any Linux platform. To build the binaries go to the repository directory 
```
cd KarmaEngine
```

and type
```
vendor/bin/Premake/Linux/premake5 gmake
make
```
This will create binaries in ``` build/Debug-linux-x86-64/Karma(or Application)```. Make sure to copy ``` libKarma.so ``` from ``` build/Debug-linux-x86-64/Karma ``` to ``` build/Debug-linux-x86-64/Application``` directory. This can be done by typing
```
cp build/Debug-linux-x86_64/Karma/libKarma.so build/Debug-linux-x86_64/Application/
```
Note: I am trying to add postbuildcommand in premake for Linux but I have not succeded yet. If you know how to do that, you are welcome to submit pull requests! You may want to look [here](https://github.com/ravimohan1991/KarmaEngine/commit/cfadad34b94c4c6154fee51ff16f514d3c2b511e#diff-305eff9084f83e9096ab2d18b9815c7b52c4d3603363d3d6a27e1c85f466ec45) (line 118).

Now you are ready to run Karma
```
build/Debug-linux-x86_64/Application/Application 
```

To develop Karma in Linux type
```
./GenerateProjects.sh
```
This will generate [Codelite](https://codelite.org/) workspace in the root directory.

References
-----------

### Books
* [Game Engine Architecture (Second Edition)](https://www.gameenginebook.com/)

### Youtube Channels
* [The Cherno](https://www.youtube.com/user/TheChernoProject)
* [Travis Vroman](https://www.youtube.com/user/barzahd512)

