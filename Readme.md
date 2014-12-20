m2u UE4 Plugin
===
This is the m2u implementation for [Unreal Engine 4](https://www.unrealengine.com). It works in conjunction with the m2u python scripts for a 3D-authoring-application https://github.com/m2u/m2u .

Once activated in the Plugin Manager of UE4, m2u will wait for a connection request from a running 3D-authoring-application.

To use the Plugin you currently will have to [build it yourself](#build). We cannot provide a working binary until a necessary change is made to the Engine Sources by Epic.

Some Videos:
https://vimeo.com/101552170
https://vimeo.com/102053975
https://vimeo.com/103139039

<a name="build"></a>
Building the Plugin
---
To build the Plugin yourself, you will have to clone the repository into a folder that is looked into by UnrealBuildTool for potential plugin code. That is generally any folder under the `UnrealEngine/Engine/Plugins` folder. Now execute UBT (compile the Engine) and it should build the plugin.
**NOTE:** The current code in my repository here is incompatible with Engine versions higher than 4.5! Fixes for compatibility can be found here https://bitbucket.org/Gigantoad/m2uue4plugin/
**NOTE:** On Windows you will have to make a minor change to the Engine Source Code to build successfully. Change line 7 in `/Source/Editor/UnrealEd/Classes/Factories/FbxFactory.h` from
`class UFbxFactory : public UFactory`
to
`class UNREALED_API UFbxFactory : public UFactory`
*There is already a Pull Request issued to EpicGames, so that change will probably not be necessary for future Engine versions.*

