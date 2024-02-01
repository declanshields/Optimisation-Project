# Optimisation Project
 
This project was a University module aimed at optimising a supplied program.
To optimise this project, I implemented Memory and Time profilers that track memory allocation and execution time of certain functions.
I also implemented a Quadtree with varying depths, and multithreading.
This project then needed to be ported to Linux, this code is provided to inspect, but will not run on a Windows machine.

The project is currently configured to run with 1000 cubes, a quadtree of depth 3 (or 32 even quadrants) and 16 threads.

Controls:
<ul>To output memory information to the console, press '1'</ul>
<ul>To output time information to the console, press '2'</ul>

Data analysis:
On average, Linux performs better than Windows, this is probably because Linux is a customisable system, as such there are a lot less programs installed when Linux is installed. Due to this, there are less processes running in the background, allowing Linux to have more resources available to handle the project compared to Windows.
