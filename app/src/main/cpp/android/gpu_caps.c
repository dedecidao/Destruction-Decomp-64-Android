/*
 * MSAA up to 2x on Android. With a 4x multisample framebuffer, SDL_gpu's Vulkan
 * output on the Adreno 740 (Galaxy S23 Ultra, Android 17) is all black: the game's
 * frame (DD64_SHOT) is empty although its textures load right. 1x and 2x render
 * (2x tested with the N64 AA at 60 fps). port/fast3d/gfx_sdlgpu.cpp only uses the
 * sample counts this says the device supports (CMakeLists.txt renames its calls to
 * this), and the Android options page offers off and 2x.
 */
#include <SDL3/SDL.h>

bool DD64A_GPUTextureSupportsSampleCount(SDL_GPUDevice *dev, SDL_GPUTextureFormat fmt, SDL_GPUSampleCount n) {
    return n <= SDL_GPU_SAMPLECOUNT_2 && SDL_GPUTextureSupportsSampleCount(dev, fmt, n);
}