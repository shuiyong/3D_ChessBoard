/*
* CSI 4341 Computer Graphics  
* Programming Assignment 6 ChessMate II the Sqeual
* Author: Yong Shui, Peter Smrcek
* Due: Dec 9th,2014
* The portions we have completed
* 1.	Textured Pieces
* 2.	Reflective Board
* 3.	Piece Animation
* 4.	Piece Movement
*/
#include "Move.h"
#include <iostream>
using namespace std;
Move::Move(int i, int sc, int sr, int dc, int dr,MOVE_TYPE type):
    index(i),sCol(sc),sRow(sc),dCol(dc),dRow(dr),moveType(type){
        x = sc;
        z = sr;
}

Move::~Move() {
}

/** By default, let everything take a second. */
int Move::duration() {
    return durationTime;
}

void Move::start() {
    // Unless the derived class has something more specific to do, just step
    // to the start.
    step( 0.0 );
}

void Move::finish() {
    // Unless the derived class has something more specific to do, just step
    // to the end.
    step( 1.0 );
}

void Move::undo() {
    // Unless the derived class has something more specific to do, just step
    // to the start.
    step( 0.0 );
}

void Move::draw( double t ) {
    // Nothing to draw by default.
}

Matrix Move::getMatrix(){
    Matrix m;
    return m;
}

int Move::getIndex(){
    return index;
}

MOVE_TYPE Move::getType(){
    return moveType;
}

void Move::setType(MOVE_TYPE type){
    moveType = type;
}