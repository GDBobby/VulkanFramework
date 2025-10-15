# Vulkan Framework (Untitled)

The goal of this project is to provide a modern, expandable platform for vulkan development.

## Features
* Shader reflection, along with utilities that automate VkPipelineLayout creation

## Planned
* I want a 1 step build process. Currently, I believe its 2 step, install vulkan, and run cmake (cmake pulls glfw and spirvcross)
* Ideally, I'd like this to be usable as a C API with other languages, but I have no idea what that looks like at this moment
* and more!

## Dependencies
The goal is to be as bare minimum as possible. 
* Vulkan
* GLFW
* Spirv-Cross

## Structure
This is very much a work in progress

<img width="733" height="375" alt="image" src="https://github.com/user-attachments/assets/4936849d-3bca-474b-b8d3-1f797bdd1d11" />

The plan right now is to have objects include a reference to what is required to make them. Poorly worded, but for example, a Command Buffer is made from a Command Pool. Therefore, a CommandBuffer is made from a Command Pool. A Command Pool is made for a queue with a logical device, so it'll include both a queue and a logical device.  and so on. I'm not sure on the big picture yet, I'll adjust the image as I go.
