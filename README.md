# MegaGraph

A GPU accelerated tool for visualizing large data sets.

![10000 images](/screenshot2.jpg?raw=true "10 000 images rendered in real time on a cheap laptop")

## Dependencies

Before compiling, please make sure you have the following dependencies installed:

```
libglfw3
libvips
libcurl
```

### Building on Debian

```
$ sudo apt-get install libglfw3-dev libvips-dev pkg-config libgsf-1-dev libcurl4-openssl-dev
$ make
```


### Installing dependencies on OS X

```
$ brew install glfw vips libxml2
$ make
```

## Authors

* Emil Romanus <emil.romanus@teorem.se>


## License

Copyright (c) 2017 Teorem AB

MegaGraph is free and licensed under the MIT license.
