scene-viewer
============

## GamePlay 3D Blender Scene Viewer

After recently found GamePlay 3D, a skillfully crafted and truly cross playform 3D game engine from BlackBerry, I started working on my first project with it. 
To create 3D content, I'm using Blender, but the workflow with GamePlay forces me to repeatedly loose context.
So, to improve the workflow, I wrote a small Blender add-on and a GamePlay app that works together as a scene viewer

## What can it do?

The Scene Viewer addon+app combo gives you a way to directly preview your scene using GamePlay, without leaving Blender.
After configuring the locations for gameplay-encoder and scene-viewer, you can press the preview button and 
your scene file will show-up.
The viewer is simple and has only few controls (for now) but it gets the job done.

## How to use it?
First, you will need to build the scene-viewer project on your platform. 
Clone the repository to the same folder containing your GamePlay repo.

```
├── GamePlay               <- GamePlay repo
└── scene-viewer           <- Your freshly cloned scene-viewer
```

Build the scene-viewer project:

```
cd scene-viewer
mkdir build
cd build
cmake ..
make
```

Next, install the Blender add-on. 
Open Blender and choose File->User Preferences->Addons->Install From File.
The addon file is in `scene-viewer/etc/scene-viewer.py`

You should now see the scene viewer panel in your scene properties sidebar (press N).
Configure your gameplay-encoder path and your scene-viewer path. 
Currently, the addon will write your scene data into the scene-viewer res/ folder. I may change this to work using a temporary folder later on.

When playing animations, the viewer will first attempt to play grouped animations and if this fails, it will play the first available animation of the node.
The viewer will also not render any node with id beginning with an underscore.

I'm currently using forward Z and up -Y as this works okay for me with Blender 2.68a. The FBX settings can be changed in the add-on file itself.

## License
Copyright 2013 Ithai Levi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
