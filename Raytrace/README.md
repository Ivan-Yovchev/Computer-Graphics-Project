== User Instructions ==

The provided [CMakeLists.txt](CMakeLists.txt) file can to be used with 
`cmake` to compile the code.
This can be used in the following way (on the command line/shell):

```
# cd into the main directory of the framework
mkdir build   # create a new build directory
cd build      # cd into the directory
cmake ..      # creates a Makefile
```

The code can take several .json files as input to generate a .png file of
the scene as specified in the .json.

To render a scene:
```
# cd to the main directory of the frame work
./ray ../Scenes/[scene_name].json
```
A [file_name].png is generated in the main directory.

There are several scenes to choose from located in Scenes directory
- scene01-shadows.json generates a scene with only one light source that
  casts a shadow on the background.
- scene01-lights-shadows.json generates a scene with two light sources that
  cast shadows on the background.
- scene01-reflect-lights-shadows.json generates a scene with two light sources
  and all of the objects in the scene are reflective.
- scene01-ss.json generates a scene where super sampling is implemented.
- scene01-texture-ss-reflect-lights-shadows.json generates a scene with two
  light sources, shadows, super sampling and reflection for all of the objects.
  Additionally, one of the objects has a texture.
- Other scenes corresponding to the above, with different rotations and
supersampling.

Two important files are also located in Scenes:
- bluegrid.png - a textures for showing latitude/longitude lines on a sphere.
- earthmap1k.png - a texture representing the map of the earth.

== Changes ==

* Tracing of the ray was extended to check for objects blocking the light(s),
  which results in shadows.
* The software was extended to trace the vector from the eye up to a certain 
  depth of recursion which results in reflective surfaces.
* Anti-aliasing was implemented for smoother surfices, shadows and edges. 
* Spheres can now have textures specified by a .png file.
* Spheres are now rotatable by specifying a rotation axis and angle in the
corresponding .json file.
