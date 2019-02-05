# experiment-code-sdl2-base
Forked from https://github.com/BLAM-Lab-Projects/experiment-code-sdl2

This code base is designed to run psychophysics experiments, interfacing with an Ascension magnetic tracking system (Flock of Birds or TrakSTAR), or alternatively a Codamotion optical tracking system (Coda branch). When these tracking systems are not available, the code switches to a mouse emulation mode. Stimulus presentation appears on a computer screen (which can be mirrored onto a secondary display), with the capacity to draw various shapes (circles, polygons, line paths), display images, play sounds, or display videos. 

This code is presented in its most generic form, with examples of how to load each of the various types of objects and draw them to the screen. This code is not meant to run any experiment in particular, and hence is likely to contain seemingly non-sensical decisions or impossible stimulus conditions. Code to run specific experiments may be available in other repositories here or on the BLAM Lab github site.

This code is meant to be a work in progress, with new functionality added as needed to support ongoing experiments in the lab. As a disclaimer, there are likely to be bugs or inefficiencies in the code (which might be resolved in other repositories).

This code base currently runs on Visual Studio 2010, using SDL2, OpenGL, and VLC3.06. Long-term goals are to eventually update this code base to run in newer versions of Visual Studio (which will provide the additional flexibility of running them on Windows 10 machines or on machines for which the .NET framework has been updated). However, this is unlikely to happen any time soon unless the need arises. A second long-term goal to develop a dual-display version of this code that will allow for separate experimenter and experimentee experiences; however this likely will require some significant changes in how images are drawn to the screen and is therefore also unlikely to happen anytime soon.

This code is made freely available for public use; if used, please acknowledge the developers of this code: <br />
    Adrian M. Haith <br />
    Aneesh Roy <br />
    Promit Roy <br />
    Aaron L. Wong <br />

