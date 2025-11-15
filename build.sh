#!/bin/bash

# Build and run the OpenGL terrain project

echo "Building the project..."
g++ -Iinclude $(find src -name '*.cpp') -lGL -lGLU -lGLEW -lglfw -o terrain

if [ $? -eq 0 ]; then
    echo "Build successful. Running the application..."
    ./terrain
else
    echo "Build failed."
    exit 1
fi