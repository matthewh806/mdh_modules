
# VCV Plugins
A dumping ground for my personal [VCV Rack](https://vcvrack.com) plugins.

## Xcode
The biggest challenge here was getting everything working with compiling, debugging, code completion etc in an IDE (Xcode). 

I've removed all hardcoded paths from the include / lib directories, so as long as the Rack-SDK and plugin dir. is of the form `$(PROJECT_DIR)../../Rack-SDK/` it should all compile nicely.

For this reason this repository may also serve as a template for getting an Xcode project up and running. If I write plugins and the project becomes too complex for template purposes, I will split the two off and maintain a bare bones template repo.

Note: The terminal project was just for IDE integration, don't try to build it (because it will fail).

