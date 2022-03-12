#!/bin/bash

sudo dnf -y freeglut libjpeg openmpi libxmu libxi boost
cmake .
make

