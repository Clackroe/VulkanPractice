#!/bin/bash

glslc -fshader-stage=vert shaders/shaderVert.glsl -o shaders/vert.spv
glslc -fshader-stage=frag shaders/shaderFrag.glsl -o shaders/frag.spv

ln -sfn ../shaders build/shaders


