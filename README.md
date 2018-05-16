
# VCV Plugins
A dumping ground for my personal [VCV Rack](https://vcvrack.com) plugins.

## XCode
The biggest challenge here was getting everything working with compiling, debugging, code completion etc in an IDE (XCode). 

I've removed all hardcoded paths from the include / lib directories, so as long as the Rack-SDK and plugin dir. is of the form `$(PROJECT_DIR)../../Rack-SDK/` it should all compile nicely.

Note: The terminal project was just for IDE integration, don't try to build it (because it will fail).
