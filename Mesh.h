/*
* CSI 4341 Computer Graphics
* Programming Assignment 6 ChessMate II the Sqeual
* Author: Yong Shui, Peter Smrcek
* Due: Dec 9th,2014
*/
#ifndef __MESH_H__
#define __MESH_H__

#include "Geometry.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
using namespace std;
//
// Representation for a polygon mesh model.
//
class Mesh {
 public:
    // Make a new mesh, with mesh data populated from the given file.
    Mesh( char const *filename );

    // Destroy this mesh.
    virtual ~Mesh();
  
    /** Draw all the polygon faces in this mesh. */
    void draw();

 private:
     vector<Vector> *vlist;
     vector<Vector> *nlist;
     vector<vector<pair<int,int> > > *flist;
     vector<pair<float,float> > *tlist;
     vector<vector<int> > *tmap;
};

#endif

