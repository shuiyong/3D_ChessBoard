#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdlib.h>
#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

/**
   Read the image from the given 24-bit BMP file and return it as an 
   array of unsigned bytes.  Set the parameters to reflect the image size.
*/
GLubyte *readImage( const char *filename, int &width, int &height);

#endif
