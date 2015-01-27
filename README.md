m2u UE4 Plugin
===
This is the m2u implementation for [Unreal Engine 4](https://www.unrealengine.com). It works in conjunction with the m2u python scripts for a 3D-authoring-application https://github.com/m2u/m2u .

Once activated in the Plugin Manager of UE4, m2u will wait for a connection request from a running 3D-authoring-application.

To use the Plugin you currently will have to [build it yourself](#build). We may provide binaries soon.

To start m2u for Unreal Engine 4 provide the init script in your Program with `"ue4"` as the second parameter.

Example in Maya:

``` python
import m2u
m2u.core.initialize("maya","ue4")
m2u.core.getProgram().ui.createUI()
```

Some Videos:
- https://vimeo.com/101552170
- https://vimeo.com/102053975
- https://vimeo.com/103139039

<a name="build"></a>
Building the Plugin
---
To build the Plugin yourself, you will have to clone the repository into a folder that is looked into by UnrealBuildTool for potential plugin code. That is generally any folder under the `UnrealEngine/Engine/Plugins` folder. Now execute UBT (compile the Engine) and it should build the plugin.


**NOTE:** For versions lower than 4.7 you will have to make some changes to the Engine source-code. Change line 7 in `/Source/Editor/UnrealEd/Classes/Factories/FbxFactory.h` from

`class UFbxFactory : public UFactory` to `class UNREALED_API UFbxFactory : public UFactory`

*This is not anymore necessary -- or has already been done by Epic -- on v4.7.*

