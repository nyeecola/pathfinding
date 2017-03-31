#ifndef CONSTANTS
#define CONSTANTS

#include <map>

#include <FL/Fl_Double_Window.H>

#include "map.hpp"

#define UPDATE_TICK_DELAY 0.017

// NOTE: this is a terrible way of getting 60-fps, but this library has no better tool for that
// TODO: maybe find a way to better update the screen anyway
#define CAMERA_SPEED 30

#define SPRITE_SIZE 15
#define TERRAIN_BLOCK_RATIO 0.15
#define TERRAIN_COLOR_MAX_WHITENESS 20

using namespace std;

const char background_img_path[] = "./map.png";

#endif
