# My collection of C++ projects

To actually be able to use build.sh or execute binaries, you need to install sfml2.
Install the package named
```smfl2```
Make sure it is installed in
```/opt/sfml2/```
Then you can use build.sh as follows:
```cd Project-you-want```
```../build.sh main.cpp <binaryname>```
To later be able to execute the freshly built binaries, you need to do the following:
```export LD_LIBRARY_PATH=/opt/sfml2/lib:$LD_LIBRARY_PATH```
This is assuming /opt/sfml2/lib is actually the right directory.