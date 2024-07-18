#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <iostream>


extern SDL_Window* window;
extern VkInstance instance;

void initWindow();
void initVulkan(VkPhysicalDevice& physicalDevice);
void processInput(bool& running);
void cleanup();