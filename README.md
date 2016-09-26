m2u UE4 Plugin
===
This is the m2u implementation for [Unreal Engine 4](https://www.unrealengine.com). It works in conjunction with the m2u python scripts for a 3D-authoring-application https://github.com/m2u/m2u .

After installing the plugin, go to the Plugin Manager `Edit -> Plugins` and make sure that the **m2u Plugin** is enabled. By default it can be found under the *Editor* category.

Once the **m2u Plugin** is enabled, it will wait for a connection request from the m2u client in a 3D-authoring-application.

To get the Plugin, you will currently have to [build it yourself](#build). 

To connect the m2u client to UE4, provide the init script in your `Program` with `"ue4"` as the second parameter.

Example in Maya:

``` python
import m2u
m2u.core.initialize("maya","ue4")
m2u.core.program.ui.create_ui()
```

<a name="build"></a>
Building the Plugin
---
To build the Plugin yourself, you will have to clone the repository into a folder that is looked into by UnrealBuildTool for potential plugin code. That is generally any folder under the `UnrealEngine/Engine/Plugins` folder. 

    $ git clone https://github.com/m2u/m2uUE4Plugin.git

Then execute UBT (re-compile the Engine) and it should build the plugin.
With a default setup, this means: Regenerate the project-files and instruct your IDE to build the Engine Project.


License
---
MIT
