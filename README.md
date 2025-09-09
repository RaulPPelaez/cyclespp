## Cycles++: A C++ multiplayer game 

**You can find the online documentation [here](https://raulppelaez.github.io/cyclespp/).**

This is a C++ project with documentation using Doxygen+Sphinx+breathe. The project is built using CMake.

### Prerequisites

Install the dependencies:

```bash
conda env create -f environment.yml
```
This will create a conda environment called `cycles` which you can activate with:

```bash
conda activate cycles
```

Check the documentation for platform-specific details.

### Building the project

To build the project, create a build directory and run CMake from there:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

The documentation will be generated in the `build/docs/sphinx/index.html` directory. Open the `index.html` file in a web browser to view it.


### Cleaning the project

To clean the project, just remove the `build` directory:

```bash
rm -rf build
```
