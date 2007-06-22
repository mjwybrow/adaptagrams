#include "graphlayouttest.h"
#include <libvpsc/rectangle.h>
#include <libcola/straightener.h>
#include <fstream>

#define ARROW_HEAD_BUFFER 3

using namespace cola;
using vpsc::Rectangle;
using std::endl;
using std::ios;
using std::max;
using std::min;
using std::ofstream;
using std::vector;

