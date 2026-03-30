# Vulkan Framework (Untitled)

The goal of this project is to provide a modern, expandable platform for vulkan development.

## Features
* Shader reflection, along with utilities that automate VkPipelineLayout creation
* Render Graph

## Planned
* I want a 1 step build process. Currently, besides environment installs, it's a 2 step. Git submodules, then cmake
* Ideally, I'd like this to be usable as a C API with other languages, but I have no idea what that looks like at this moment
* I'm plannin on adding WebGPU support, so that the framework can be compiled to wasm and run in a browser.

## Dependencies
The goal is to be as bare minimum as possible. 
* Vulkan
* GLFW
* Spirv-Cross
* vma
