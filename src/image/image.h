#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Load an image in the memory using the SDL library
SDL_Surface* loadImage(char* path);

// Get the color of the pixel (x,y) on an image
Uint32 getPixel(SDL_Surface *image, int x, int y);
