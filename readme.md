# Pathfind UI project

I uploaded the code to github for convenience, but if anyone finds it useful, even as a bad example, good.
NOTE:github is marking the project as C, but it is C++ only.

This project uses one of my implementation of the A* pathfind algoritm (no corner cut), to find the shortest path if several maps available at [http://movingai.com/benchmarks/](http://movingai.com/benchmarks/).
The algorithm found every path tested, although with a minor floating point error, eg: expected 235.0536 and I got 235.0537. After testing, I found that this was a problem with the value for diagonal movement, sqrt(2).

But running the benchmarks on a terminal and reading results is somewhat boring, so I decided to make a UI that show the algorithm working.
I started by making a rudimentary UI with SDL2, it worked, but was not good enough. Then I made my [sdl_gui](https://github.com/N4G170/sdl_gui) and scarped the old ui and made a new one.

To be able to see the progress of the search, the render and search functions cannot be in the same thread, so the search runs in its own. Initially I was using std::thread to call the search and then detach, but it caused some problems with concurrent access to some ui elements (not even a mutex solved the issue), so I changed it to a task, basically an asynchronous function call, then from time to time I check if the result is in.
The rendering of the map is, also, protected with a mutex, because the search algorithm writes on it and the render function reads from it, in separated threads.

As I said, the program uses my sdl_gui for its ui, but on linux, it uses the gui lib as a shared object (think dll), and on windows, the sdl_gui code is integrated on the program, all this because the sdl_gui code is not ready for compilation into a usable dll(next sdl_gui version will support it). But what matters is that it works.

###### Hierarchical search
In addition to the base A* algorithm, I made a small test. Two of the maps (marked on the ui with a red warning), use a different search, namely an hierarchical search.
The objective of this search is not to find the shortest path, but find a path fast.
When a search is requested:
- First we create a new map using each room (both maps are x*x room based) as a node and marking its exit/entry points and neighbours;
- Then we find the shortest number of rooms we need to move through to reach the target
- We then, find the shortest path through the room we found
Like I said, the result may not be the best (eg: expected 856.05, got 877.3), but its fast. You can compare it in the program, a both the hierarchical and non-hierarchical maps have the same benchmarks defined.

Speaking of benchmarks, only the first 10 benchmarks for each map are available, because my ui design is so good that more than that made the ui a little cumbersome.

###### Fast maze
This one in not ready yet, other thing got in the way, but the idea is, for the maze maps, have a new search, that instead of using a search that checks every position, it only checks corners/intersections.
Each corners/intersections is a node in a graph and knows its direct neighbours and how far they are.
Currently the problem I'm facing is in how should select the closest node to the start/target points, because the one closest may not return the shortest path.

## Running the program
If you are using one of the release builds, double click the .exe on windows or run in a terminal on linux.
The provided build represents a demo showcasing the elements present on the project. Use the button on the top to change panels and try the elements.


## Building the program

If you are building this project I recommend using a linux distro, as it is so easier.

### If on Linux

First install all the dependencies:
- SDL2
- SDL2_image
- SDL2_ttf
- cmake (needed for "fast" build)
After that, open terminal window at the base of the project (the folder with this file) and run the commands:
```
- mkdir build
- cd build
- cmake ..
- make -j
```
If no errors appear, the build is complete and you can run the program with the command ./pathfind_ui
I did not configure any install instructions, so if you want to move the build, copy the folder 'data' and the file 'pathfind_ui' from the build folder.
The linux build uses a separate .so lib with only the gui files.
Sadly on windows I was unable to do so, as sdl_gui code is not ready for it, when implementing the ECS I'll take extra care to make the code ready for it.

NOTE: As cmake creates the executable as a shared object (I have yet to find why), you have to run the program through the terminal, rather than double click

### On windows

On windows, building the program is not that easier. As the cmake module used to find SDL2 does not work on windows, the build has to be made manually.

Inside the vs folder there is a Visual studio solution configured to build the program using the current folder structure.
If all goes well, all you have to do is open the solution, and build (only works for x64 builds).

If the solution does not work, or you use another build system, you have to add the files by hand to you project and take care of the dependencies.
For ease of building, you can find the needed dependencies inside the vs folder.
NOTE: I do not own any of the code from the dependencies. Their license allows them to be freely used and shared.

To run the program, you need the data folder next to the .exe as well as all of SDL2, SDL2_image, SDL2_ttf dlls (you can find them in my release or download them from the SDL website).

NOTE: the windows builds does not use sdl_gui as a separate lib, as I was unable to do so, because sdl_gui code is not ready for it, when implementing the ECS I'll take extra care to make the code ready for it. One of the problems of multiplatform development, if you do not expect it.

## TODO

Finish fast maze algorithm and try to use the hierarchical search on more types of map.
After creating the .lib/.dll with sdl_gui, I'll update this windows build to make use of it.

If you find any bug of error, let me know.
