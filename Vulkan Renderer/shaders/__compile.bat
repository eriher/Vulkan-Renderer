@echo off
echo "Compiling Shaders"
for %%i in (*.frag, *.vert, *.comp) do C:\VulkanSDK\1.2.176.1/Bin/glslc.exe "%%i" -o ../compiledshaders/"%%i".spv
echo "Done"	