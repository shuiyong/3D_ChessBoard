/*
* CSI 4341 Computer Graphics
* Programming Assignment 6 ChessMate II the Sqeual
* Author: Yong Shui, Peter Smrcek
* Due: Dec 9th,2014
* The parts we have completed
* 1.	Textured Pieces
* 2.	Reflective Board
* 3.	Piece Animation
* 4.	Piece Movement
*/
#ifndef __MOVE_H__
#define __MOVE_H__

#include "Geometry.h"
enum MOVE_TYPE { FIRST, UNDO, REDO };
/** Abstract class for a move operation.  Moves have three states. Unapplied, Intermediate and Applied.   */
class Move {
public:
    int index;// the index in objectlist
    //(sCol, sRow) is the start point of movement in chessboard
    int sCol; 
    int sRow;
    //(dCol,dRow) is the end point of movement in chessboard
    int dCol;
    int dRow;
    //(x,z) is the midcoordinate during the animation
    double x,z;
    MOVE_TYPE moveType;// the movement can be FIRST,  REDO, UNDO

    Move(){}
    Move(int i, int sc, int sr, int dc, int dr,MOVE_TYPE type);
    /** Virtual destructor, since this object has virtual functions. */
    virtual ~Move();
  
    /** Report how many milliseconds this move would like to take to
        apply itself.  The caller may or may not respect this time.  For
        example, if the caller was performing a fast-forward operation, moves
        might be animated with a compressed timeframe. */
    virtual int duration();

    /** Transition from the unapplied state to the intermediate state.
        The default implementation simply calls step( 0.0 ). */
    virtual void start();

    /** Transition from the intermediate state to the applied state.
        The default implementation simply calls step( 1.0 ). */
    virtual void finish();

    /** Transition from the applied state immediately back to the
        unapplied state.  The default implementation simply calls
        step( 0.0 ). */
    virtual void undo();

    /** Change the state of the scene in response to partial application
        of this move.  The t parameter varies from 0 to 1 to indicate
        the degree to which the move should be applied, zero indicating
        the very start of application and one indicating the end.  This
        function will only be called when the move is in the
        intermediate state, but subclasses should not assume that t will
        be strictly increasing from call to call.  The driver may want
        to animate moves backward.  */
    virtual void step( double t ) = 0;

    /** Draw anything this move needs on top of the rest of the
        screen.  The default implementation does nothing. */
    virtual void draw( double t );
    //set the animation time
    void setDuration(int time){
        durationTime = time;
    }
    //return the trans matrix of object
    virtual Matrix getMatrix();
    //return index
    int getIndex();
    //return type of movement
    MOVE_TYPE getType();
    // set type of movement
    void setType(MOVE_TYPE type);
private:
    int durationTime;// the time of animation
};

#endif

