/*
* CSI 4341 Computer Graphics
* Programming Assignment 6 ChessMate II the Sqeual
* Author: Yong Shui, Peter Smrcek
* Due: Dec 9th,2014
* version: submission1.0
* The portions we have completed
* 1.	Textured Pieces
* 2.	Reflective Board
* 3.	Piece Animation
* 4.	Piece Movement
*/
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <ctime>
#include <vector>
#include <algorithm>
#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "Geometry.h"
#include "Mesh.h"
#include "Move.h"
#include "Image.h"
#include "Imageloader.h"

using namespace std;

/* Position of the light source.
 * Ugly that it is global, but I want it statically initialized 
 * other arrays are material data
 */
float lightPosition[4] = { 1.0f, 18.0f, 1.0f, 1.0f };
GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };   
GLfloat mat_ambient[] = { 0.7, 0.7, 0.4, 1.0 };   
GLfloat mat_ambient_color[] = { 0.7, 0.7, 0.4, 1.0 };   
GLfloat mat_diffuse1[] = { 0.0, 0.4, 0.2, 1.0 };   
GLfloat mat_diffuse2[] = { 0.8, 0.0, 0.8, 1.0 };  

GLfloat mat_diffuse3[] = { 1, 0.3, 1, 1.0 };   
GLfloat mat_diffuse4[] = { 0, 0.7, 0.2, 1.0 }; 

GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };   
GLfloat no_shininess[] = { 0.0 };   
GLfloat low_shininess[] = { 5.0 };   
GLfloat high_shininess[] = { 100.0 };   
GLfloat mat_emission[] = {0.3, 0.2, 0.2, 0.0};


class ChessBoard {
    /* Different types of pieces, also, indices into meshList */
    enum PieceType { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, 
                     // Not really a piece, but it gets an index anyway.
                     CROSS };

    /** Record for an individual object in our scene. */
    struct Object {
        /** Transformation for the model. */
        Matrix trans;

        /** Side this model is on, zero or one. */
        int side;
    
        /** Discrete column and row position of this piece on the board. */
        int col, row;
    
        /** Indication of what type of piece this is, also an index into
            the mesh list. */
        PieceType type;

        bool drawNow;
        
    };

    /** Linear list of objects in the scene. */
    vector< Object > objectList;
    vector< Mesh * > meshList;
    
    bool mouseDown;
    float alpha;
    
    /*
     * the movement of Knight
     * it can move in borken line
     */
    class Knight_Move:public Move{
    private:
        //the pointer to the chessboard
        ChessBoard *p;
        //(midCol, midRow) is the coordinate the knight' turn point
        int midCol;
        int midRow;
        // the percent of time before the knight turn its direction
        float turn;
    public:
        Knight_Move(int i, int x1, int z1, int x2,int z2, int x3,int z3,MOVE_TYPE type,ChessBoard *p1){
            index = i;
            sCol = x1;
            sRow = z1;
            dCol = x2;
            dRow = z2;
            moveType = type;
            p = p1;
            midCol = x3;
            midRow = z3;
            if(abs(midCol - sCol) == 1)
                turn = 1.0 / 3;
            else
                turn = 2.0 / 3;
        }
        void draw(double t){
            glPushMatrix();
            p->objectList[index].trans.glMult();
            p->meshList[p->objectList[index].type]->draw();
            glPopMatrix();
       }
       //step function
       void step(double t){
           if(t < turn){
               x = sCol +  t * (midCol - sCol);
               z = sRow +  t * (midRow - sRow);
           }else{
               x = midCol +  (t - turn) * (dCol - midCol);
               z = midRow +  (t - turn) * (dRow - midRow);
           }
           Object &obj = p->objectList[index];
           obj.trans = Matrix::translate( x + 0.5, 0, z + 0.5 );

           // Spin around pieces on side 1.
           if ( obj.side == 1 )
               obj.trans = obj.trans * Matrix::rotateY( 180 );
       }
       //setting the for undo movement
       void undo(){
           swap(sCol,dCol);
           swap(sRow,dRow);
           turn  = 1.0 - turn;
       }
       //finish a movement
       void finish(){
           p->objectList[index].col = dCol;
           p->objectList[index].row = dRow;
           p->objectList[index].trans = Matrix::translate( dCol + 0.5, 0, dRow + 0.5 );
           if ( p->objectList[index].side == 1 )
               p->objectList[index].trans = p->objectList[index].trans * Matrix::rotateY( 180 );
           p->dCol = -1;
           p->dRow = -1;
           p->alpha = 1;
       }
       //get trans matrix
       Matrix getMatrix(){
           Object &obj = p->objectList[index];
           
           Matrix m = Matrix::translate( x + 0.5, 0, z + 0.5 );

           // Spin around pieces on side 1.
           if ( obj.side == 1 )
               m = m * Matrix::rotateY( 180 );
            return m;
       }
    };
    /*
    * the movement of other piece excluding knight
    * the movement is a line
    */
    class Other_Move:public Move{
    private:
        ChessBoard *p;
        
    public:
        Other_Move(int i, int x1, int z1, int x2,int z2, MOVE_TYPE type,ChessBoard *p1){
            index = i;
            sCol = x1;
            sRow = z1;
            dCol = x2;
            dRow = z2;
            moveType = type;
            p = p1;
        }
        void draw(double t){
            glPushMatrix();
            p->objectList[index].trans.glMult();
            p->meshList[p->objectList[index].type]->draw();
            glPopMatrix();
       }
        //step for animation
        void step(double t){
            x = sCol +  t * (dCol - sCol);
            z = sRow +  t * (dRow - sRow);
            Object &obj = p->objectList[index];
            obj.trans = Matrix::translate( x + 0.5, 0, z + 0.5 );

            // Spin around pieces on side 1.
            if ( obj.side == 1 )
                obj.trans = obj.trans * Matrix::rotateY( 180 );
        }
        //setting for undomovement
        void undo(){
            swap(sCol,dCol);
            swap(sRow,dRow);
        }
        //finish a movement
        void finish(){
            p->objectList[index].col = dCol;
            p->objectList[index].row = dRow;
            p->objectList[index].trans = Matrix::translate( dCol + 0.5, 0, dRow + 0.5 );
            if ( p->objectList[index].side == 1 )
                p->objectList[index].trans = p->objectList[index].trans * Matrix::rotateY( 180 );
            p->dCol = -1;
            p->dRow = -1;
            p->alpha = 1;
        }
        //get trans matrix
        Matrix getMatrix(){
            Object &obj = p->objectList[index];
            
            Matrix m = Matrix::translate( x + 0.5, 0, z + 0.5 );

            // Spin around pieces on side 1.
            if ( obj.side == 1 )
                m = m * Matrix::rotateY( 180 );
            return m;
        }
    };

    /** List of meshes, one for each piece type, and a bix X mark at the end. */
    
    /** Enum-hacked integer constants */
    enum { 
        /** Size of the board. */
        BOARD_SIZE = 8,

        /** Value for a board cell that's empty. */
        EMPTY_CELL = -1,
    };

    /** 2D representation of the board contents.  Elements of array are
        EMPTY_CELL or they are indices into objectList. */
    vector< vector< int > > board;

    /** set of Move objects that are Undo-able */
    vector< Move * > undoList;

    /** set of Move objects that are currently being applied, along with
        the application time when each operation started being applied. */
    vector< pair< int, Move * > > activeList;

    /** Rotation angle for the view. */
    double camRotation;

    /** View elevation angle. */
    double camElevation;

    /** Mouse location for the last known mouse location, these are used
        for some of the mouse dragging operations. */
    int lastMouseX, lastMouseY;

    /** List of keys currently held down. */
    vector< int > dkeys;

    /** Index into objectList for the currently selected chess piece,
        or -1 if nothing is selected */
    int selection;

    int sCol,sRow,dCol,dRow;

    /** Projection matrix for the current view. */
    Matrix projectionMatrix;
    double projectV[4];
    /** Camera placement matrix for the current view. */
    Matrix cameraMatrix;
    double cameraV[4];
    // the transformation matrix for shadow
    Matrix shadowMatrix;
    //for blending
    GLuint textureId;
    /** Return true if the given key is being held down. */
    bool keyPressed( unsigned char key ) {
        return find( dkeys.begin(), dkeys.end(), key ) != dkeys.end();
    }

    /** Convenience function, return the value x, clamped to the [ low,
        high ] range. */
    static double clamp( double x, double low, double high ) {
        if ( x < low )
            x = low;
        if ( x > high )
            x = high;
        return x;
    }

    /** Utility function to fold camera placement into modelview matrix.
        Caller must pass in the aspect ratio of the viewport.  If
        wipeProjection is true, clear out the projection matrix before
        installing the projection.  This is an accommodationn to the
        object selection code (which I've removed for now).  */
    void placeCamera( double aspect, bool wipeProjection = true ) {
        // Set up a perspective projection based on the window aspect.
        glMatrixMode( GL_PROJECTION );
        if ( wipeProjection )
            glLoadIdentity();
        //gluPerspective( 53.13, aspect, 4, 150 );
        {
            projectionMatrix[0][0] = projectionMatrix[1][1]/aspect;
            projectionMatrix.glMult();
        }
        projectV[0] = 53.13;
        projectV[1] = aspect;
        projectV[2] = 4;
        projectV[3] = 150;

        // Place the camera based on the rotation angle.
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        //gluLookAt( vp.x + halfBoard, vp.y, vp.z + halfBoard,    halfBoard, 0, halfBoard,    0, 1, 0 );
        {
            cameraMatrix = Matrix::translate(0, 0, -12)
            * Matrix::rotateX(camElevation)
            * Matrix::rotateY(camRotation)
            * Matrix::translate(-BOARD_SIZE / 2.0, 0, -BOARD_SIZE / 2.0);
            cameraMatrix.glMult();
        }
    }

    /*
    * draw board pieces, shadow and reflective board
    */
    void drawScene() {
        // Don't use z-buffer while we draw the board and shadows.
        glDisable( GL_DEPTH_TEST );
        
        // draw chesspiece reflections
        glEnable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glStencilFunc(GL_NEVER, 1, 0xFF);
        glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);  // draw 1s on test fail (always)
        
        // draw stencil pattern
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);  // needs mask=0xFF
        
        // define chessboard as the only place to draw something (rest is clipped)
        glBegin( GL_QUADS );
        glVertex3d( 0, 0, 0 );
        glVertex3d( 0, 0, BOARD_SIZE );
        glVertex3d( BOARD_SIZE, 0, BOARD_SIZE );
        glVertex3d( BOARD_SIZE, 0, 0 );
        glEnd();
        
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        //glDepthMask(GL_TRUE);
        glStencilMask(0x00);
        // draw only where stencil's value is 1
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        
        //draw reflective pieces
        glPushMatrix();
        // flip the pieces
        Matrix::scale(1, -1, 1).glMult();
        // draw the pieces
        for ( int i = 0; i < objectList.size(); i++ ) {
            if(i < objectList.size() / 2)
                glColor4f(0.8,0,0.8,0.4);
            else
                glColor4f(0.0,0.4,0.2,0.4);
            
            // Apply the object's transformation.
            glPushMatrix();
            objectList[ i ].trans.glMult();
            
            // Draw the mesh.
            meshList[ objectList[ i ].type ]->draw();
            
            glPopMatrix();
        }
        glPopMatrix();
        
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_LIGHTING);
        
        // Draw the surface of the chess board.
        glEnable(GL_TEXTURE_2D);
	    glBindTexture(GL_TEXTURE_2D, _textureId);
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glBegin( GL_QUADS );
        for ( int x = 0; x < BOARD_SIZE; x++ )
            for ( int z = 0; z < BOARD_SIZE; z++ ) {
                glBegin( GL_QUADS );
                if ( ( x + z ) % 2 == 0 ){
                    glColor4f( 0.7,0.7,0.7,0.7 );
                    glTexCoord2f(0.75,0.75);
                    glVertex3d( x, 0, z );
                    glTexCoord2f(0.75,1);
                    glVertex3d( x, 0, z + 1 );
                    glTexCoord2f(1,1);
                    glVertex3d( x + 1, 0, z + 1 );
                    glTexCoord2f(1,0.75);
                    glVertex3d( x + 1, 0, z );
                }
                else{
                    glColor4f( 0.7,0.7,0.7, 0.7 );
                    glTexCoord2f(0.75,0.25);
                    glVertex3d( x, 0, z );
                    glTexCoord2f(0.75,0.5);
                    glVertex3d( x, 0, z + 1 );
                    glTexCoord2f(1,0.5);
                    glVertex3d( x + 1, 0, z + 1 );
                    glTexCoord2f(1,0.25);
                    glVertex3d( x + 1, 0, z );
                }
                glEnd();
                //draw the destination squere
                if(dRow == x && dCol == z){
                    glBegin( GL_QUADS );
                    glColor4f( 1.0,0.5,0.3,1 - alpha);
                    glTexCoord2f(0.5,0.25);
                    glVertex3d( x, 0, z );
                    glTexCoord2f(0.5,0.5);
                    glVertex3d( x, 0, z + 1 );
                    glTexCoord2f(0.75,0.5);
                    glVertex3d( x + 1, 0, z + 1 );
                    glTexCoord2f(0.75,0.25);
                    glVertex3d( x + 1, 0, z );
                    glEnd();
                }
            }
        // draw shadows
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // test fails when value is not 1
        glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);  // increment where shadow is
        glStencilMask(0xFF);
        
        glPushMatrix();
        shadowMatrix.glMult();
        for ( int i = 0; i < objectList.size(); i++ ) {
            // Apply the object's transformation.
            glPushMatrix();
            objectList[ i ].trans.glMult();
            // Draw the mesh.
            meshList[ objectList[ i ].type ]->draw();
            glPopMatrix();
        }
        glPopMatrix();
        
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // draw only where stencil's value is 2
        glStencilFunc(GL_EQUAL, 2, 0xFF);
        
        glColor4f(0,0,0, 0.5);
        glBegin( GL_QUADS );
        glVertex3d( 0, 0, 0 );
        glVertex3d( 0, 0, BOARD_SIZE );
        glVertex3d( BOARD_SIZE, 0, BOARD_SIZE );
        glVertex3d( BOARD_SIZE, 0, 0 );
        glEnd();
        
        glDisable(GL_STENCIL_TEST);
        glDisable( GL_BLEND );
        
        // Use Z-buffer while we draw the playing pieces.
        glEnable(GL_TEXTURE_2D);
        glEnable( GL_DEPTH_TEST );
        glEnable(GL_LIGHTING);
        // Draw everything on the board.
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);   
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse2);   
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);     
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_diffuse2);
        for ( int i = 0; i < objectList.size(); i++ ) {
            if(i == objectList.size() / 2){
                //the other side
                glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);   
                glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse1);   
                glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);    
                glMaterialfv(GL_FRONT, GL_EMISSION, mat_diffuse1);
            }
            
            if (selection == i) {//this is for the piece that is chosen
                glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess); 
            }else{
                glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess); 
            }

            Object &obj = objectList[ i ];
            glPushName(i);
            glPushMatrix();
            obj.trans.glMult();
            meshList[ objectList[ i ].type ]->draw();
            glPopMatrix();
        }

        /*//I change the object trans, So I don't need these codes
        int ert = glutGet( GLUT_ELAPSED_TIME );
        bool flag = false;
        for ( int i = activeList.size() - 1; i >= 0; i-- ) {
            // Figure out how far this move should be in its animation.
            double t = double( ert - activeList[ i ].first ) / 
                activeList[ i ].second->duration();
      
            // See if this animation operation has run for long enough.
            if ( 0 <= t && t <= 1 ) {
                if(flag == false){
                    flag = true;
                    activeList[ i ].second->draw( t );
                }else{
                    activeList[ i ].first -= activeList[ i ].second->duration();
                    activeList[ i ].second->draw( 1 );
                }
            }
        }
        */
    }

    /** Find any geometry that's near the mouse x, y location,
        and return a copy of the namestack for the closest object
        in depth at that location.  If no object is found, an empty
        vector is returned. */
    vector< GLuint > selectGeometry( int x, int y ) {
        // Copy of the hit record for everything near x, y
        vector< pair< GLuint, vector< GLuint > > > snapshot;

        // Get the window size for camera placement.
        int winWidth = glutGet( GLUT_WINDOW_WIDTH );
        int winHeight = glutGet( GLUT_WINDOW_HEIGHT );

        // Get a copy of the viewport transformation.
        GLint view[ 4 ];
        glGetIntegerv( GL_VIEWPORT, view );
        
        // Set up a tiny little view volume to help with selection, put the
        // pick matrix transformation after the projection.
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        gluPickMatrix( x, winHeight - y, 3, 3, view );
        placeCamera( double( winWidth ) / winHeight, false );

        // Perpare for drawing in selection mode.
        GLuint buffer[ 2048 ];
        glSelectBuffer( sizeof( buffer ) / sizeof( buffer[ 0 ] ), buffer );
        glRenderMode( GL_SELECT );
        
        // Draw the whole scene, and see what hits the pick volume
        drawScene();
        glFlush();
        
        // See if we hit anything.
        int hcount = glRenderMode( GL_RENDER );
        if ( hcount == -1 ) {
            cerr << "Selection buffer overflow!" << endl;
        } else {
            // Find the closest object that was hit by inspecting
            // the list of hit records built duing rasterization.
            int pos = 0;
            while ( hcount ) {
                // Make a copy of this name stack report.
                pair< GLuint, vector< GLuint > > rec;
                rec.first = buffer[ pos + 1 ];
                for ( int i = 0; i < buffer[ pos ]; i++ )
                    rec.second.push_back( buffer[ pos + 3 + i ] );

                // Put the new record on the list of records.
                snapshot.push_back( rec );
            
                // Move to the next item in the selection buffer.
                pos += 3 +  buffer[ pos ];
                hcount--;
            }
        }

        // Find the closest hit record, and return it if there is one.
        sort( snapshot.begin(), snapshot.end() );
        if ( snapshot.size() == 0 )
            return vector< GLuint >();
        return snapshot.front().second;
    }

    /** Convenience function to make a new piece and put it in the
        board. */
    void createPiece( PieceType type, int side, int col, int row ) {
        Object obj;
        // Make it the right kind of piece.
        obj.type = type;
        obj.side = side;

        // Put it in the right place via its transformation.
        obj.trans = Matrix::translate( col + 0.5, 0, row + 0.5 );

        // Spin around pieces on side 1.
        if ( side == 1 )
            obj.trans = obj.trans * Matrix::rotateY( 180 );

        // Put it into the chess board at the right spot.
        obj.col = col;
        obj.row = row;
        obj.drawNow = true;
        objectList.push_back( obj );
        board[ col ][ row ] = objectList.size() - 1;
    }

public:
    
    ~ChessBoard() {
        // Delete all the meshes we loaded.
        while ( meshList.size() ) {
            delete meshList.back();
            meshList.pop_back();
        }
    }

    /** Create output window, initialize OpenGL features for the driver. */
    void init( int &argc, char *argv[] ) {
        mouseDown = false;

        // Load peshes for all the pieces.
        meshList.push_back( new Mesh( "pawn.mesh" ) );
        meshList.push_back( new Mesh( "rook.mesh" ) );
        meshList.push_back( new Mesh( "knight.mesh" ) );
        meshList.push_back( new Mesh( "bishop.mesh" ) );
        meshList.push_back( new Mesh( "queen.mesh" ) );
        meshList.push_back( new Mesh( "king.mesh" ) );
        meshList.push_back( new Mesh( "cross.mesh" ) );

        // Make a new window with double buffering and with Z buffer.
        glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL );
        glutInitWindowSize( 800, 600 );
        glutCreateWindow( "Chess Board" );
    
        // Initialize background color.
        glClearColor( 0.7, 0.7, 0.7, 0 );

        // Prepare our texture image.
        {
            initRendering();
            /*
            // What to do when we run out of texture at the edge.
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
      
            // How to map the discrete texels to the frame buffer.
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
            // Tell opengl to multiply texture by the shade.
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

            // Load an image to use as the texture
            int width, height;

            glGenTextures(1, &textureId); //Make room for our texture
	        glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit

            GLubyte *image = readImage( "texture.bmp", width, height );
            
            // Register the texture with opengl.
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                          GL_RGB, GL_UNSIGNED_BYTE, image );

            // There are constants for these two values, but, my
            // headerfile on windows does not have them.  This tells
            // OpenGL to give the specular component of reflection
            // special treatment in texture mapping.

            // glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, 
            // GL_SEPARATE_SPECULAR_COLOR );
            //glLightModeli( 0x81F8, 0x81FA );
            */
        }

        // Allocate the board representation and start it out empty.
        int v = EMPTY_CELL;
        board = vector< vector< int > >( BOARD_SIZE, 
                                         vector< int >( BOARD_SIZE, v ) );

        // Place all the pieces on the chess board.  This should probably
        // be driven by a data file, rather than hard-coded.
        createPiece( PAWN, 0, 0, 1 );
        createPiece( PAWN, 0, 1, 1 );
        createPiece( PAWN, 0, 2, 1 );
        createPiece( PAWN, 0, 3, 1 );
        createPiece( PAWN, 0, 4, 1 );
        createPiece( PAWN, 0, 5, 1 );
        createPiece( PAWN, 0, 6, 1 );
        createPiece( PAWN, 0, 7, 1 );

        createPiece( ROOK, 0, 0, 0 );
        createPiece( KNIGHT, 0, 1, 0 );
        createPiece( BISHOP, 0, 2, 0 );
        createPiece( QUEEN, 0, 3, 0 );
        createPiece( KING, 0, 4, 0 );
        createPiece( BISHOP, 0, 5, 0 );
        createPiece( KNIGHT, 0, 6, 0 );
        createPiece( ROOK, 0, 7, 0 );

        createPiece( PAWN, 1, 0, 6 );
        createPiece( PAWN, 1, 1, 6 );
        createPiece( PAWN, 1, 2, 6 );
        createPiece( PAWN, 1, 3, 6 );
        createPiece( PAWN, 1, 4, 6 );
        createPiece( PAWN, 1, 5, 6 );
        createPiece( PAWN, 1, 6, 6 );
        createPiece( PAWN, 1, 7, 6 );

        createPiece( ROOK, 1, 0, 7 );
        createPiece( KNIGHT, 1, 1, 7 );
        createPiece( BISHOP, 1, 2, 7 );
        createPiece( QUEEN, 1, 3, 7 );
        createPiece( KING, 1, 4, 7 );
        createPiece( BISHOP, 1, 5, 7 );
        createPiece( KNIGHT, 1, 6, 7 );
        createPiece( ROOK, 1, 7, 7 );

        // Set initial camera configuration
        camRotation = 0;
        camElevation = 30;

        // Nothing is selected yet.
        selection = -1;

        sCol = 0;
        sRow = 1;
        dCol = sCol;
        dRow = sRow;

        initLight();
        
        // build perspective matrix manually
        float zFar = 150;
        float zNear = 4;
        float fov = 53.13;
        float max = zNear * tan(fov * 0.00872664625);
        float depth = zFar - zNear;
        
        float perspective[] = {
            (zNear / max) / (4/3), 0, 0, 0,
            0, zNear / max, 0, 0,
            0, 0, -(zFar + zNear) / depth, -1,
            0, 0, -2 * (zFar * zNear) / depth, 0};
        projectionMatrix = Matrix::glConvert(perspective);
        
        shadowMatrix =
        // undo the movement to light x and z position
        Matrix::translate(lightPosition[0], 0.0, lightPosition[2])
        // undo the look at the chessboard
        * Matrix::rotateX(-90)
        // scale from 2x2x2 box
        * Matrix::scale(lightPosition[1]/2, lightPosition[1]/2, 0)
        // do the projection
        * projectionMatrix
        // look at the plane we project to (chessboard)
        * Matrix::rotateX(90)
        // move to the light position
        * Matrix::translate(-lightPosition[0], -lightPosition[1], -lightPosition[2]);
        alpha = 1;
    }

    //Makes the image into a texture, and returns the id of the texture
    GLuint loadTexture(Image* image) {
	    GLuint textureId;
	    glGenTextures(1, &textureId); //Make room for our texture
	    glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
	    //Map the image to the texture
	    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
				     0,                            //0 for now
				     GL_RGB,                       //Format OpenGL uses for image
				     image->width, image->height,  //Width and height
				     0,                            //The border of the image
				     GL_RGB, //GL_RGB, because pixels are stored in RGB format
				     GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
				                       //as unsigned numbers
				     image->pixels);               //The actual pixel data
	    return textureId; //Returns the id of the texture
    }

    GLuint _textureId; //The id of the texture

    void initRendering() {
        glEnable(GL_DEPTH_TEST);
	    glEnable(GL_NORMALIZE);
	    glEnable(GL_COLOR_MATERIAL);
	
	    Image* image = loadBMP("texture.bmp");
	    _textureId = loadTexture(image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    delete image;
    }

    void initLight(){
        GLfloat ambient[] = { 0.0, 0.0, 0.0, 1.0 };   
        GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };    
        GLfloat position[] = { 1, 4, 1, 0.0 };   
       
        glEnable(GL_DEPTH_TEST); 
       
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);   
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);   
        glLightfv(GL_LIGHT0, GL_POSITION, position);   
       
        glEnable(GL_LIGHTING);   
        glEnable(GL_LIGHT0);  
    }

    void drawTest(){
        if(activeList.size() == 0)
            return;
        int x,z;
        glBegin( GL_QUADS );
                glColor3f( 0, 1, 0 );
                glVertex3d( x, 0, z );
                glVertex3d( x + 1, 0, z );
                glVertex3d( x + 1, 1, z );
                glVertex3d( x, 1, z );
        glEnd();
    }
    /** Redraw the contetns of the display */  
    void display() {
        // Figure out the aspect ratio.
        int winWidth = glutGet( GLUT_WINDOW_WIDTH );
        int winHeight = glutGet( GLUT_WINDOW_HEIGHT );
        
        // Make sure we take camera position into account.
        placeCamera( double( winWidth ) / winHeight );

        // Clear the color and the Z-Buffer components.
        // Cear the screen.
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // Draw everything
        drawScene();
        
        // Show it to the user.
        glutSwapBuffers();
    }

    /** Callback for key down events */
    void keyDown( unsigned char key, int x, int y ) {
        // Add key to the pressed list if it's not already there.
        if ( !keyPressed( key ) )
            dkeys.push_back( key );
        // Remember where the mouse was when this key was pressed.
        lastMouseX = x;
        lastMouseY = y;
    }

    /** Callback for key down events */
    void keyUp( unsigned char key, int x, int y ) {
        // Remove key from the list of down keys, if it's in there.
        vector< int > :: iterator pos;
        while ( ( pos = find( dkeys.begin(), dkeys.end(), key ) ) != dkeys.end() )
            dkeys.erase( pos );

        if(key == 'u' || key == 'U'){
            int ert = glutGet( GLUT_ELAPSED_TIME );
            int size = undoList.size();
            if(size == 0){
                cout<<"No Undo Action"<<endl;
                return;
            }
            undoList[size - 1]->undo();
            undoList[size - 1]->setType(UNDO);
            finishAllMove();
            activeList.push_back(pair<int, Move *>(ert,undoList[ size - 1]));
            sRow = activeList[activeList.size() - 1].second->dCol;
            sCol = activeList[activeList.size() - 1].second->dRow;
            dRow = sRow;
            dCol = sCol;
            board[sRow][sCol] = activeList[activeList.size() - 1].second->index;
            board[activeList[activeList.size() - 1].second->
                sCol][activeList[activeList.size() - 1].second->sRow] = EMPTY_CELL;
            selection = -1;
            undoList.erase(undoList.begin() + size - 1);
        }

        // Remember where the mouse was when this key was released.
        lastMouseX = x;
        lastMouseY = y;
    }

    /** Callback for when the mouse button is pressed or released */
    void mouse( int button, int state, int x, int y ) {
        
        if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
            mouseDown = false;
        }
        
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !mouseDown){
            mouseDown = true;
            vector< GLuint> index = selectGeometry(x,y);
            
            if (index.size() > 0) {
                selection = index[index.size() - 1];
                for(int k = 0; k < BOARD_SIZE * BOARD_SIZE; k++){
                    int i = k / BOARD_SIZE;
                    int j = k % BOARD_SIZE;
                    if(board[i][j] == selection){
                        sRow = i;
                        sCol = j;
                        break;
                    }
                }
                cout << "figure " << selection << " selected!" << endl;
                glutPostRedisplay();
            } else if (selection != -1) {
                // if no figure was clicked, but there is already some selected
                
                GLdouble modelMatrix[16];
                GLdouble projMatrix[16];
                GLint viewport[4];
                
                // I have to do this since I don't want to change Matrix interface, but need to get double array,
                // So I cant use our cameraMatrix and projectionMatrix
                glGetIntegerv(GL_VIEWPORT, viewport);
                glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
                glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
                
                GLdouble nx, ny, nz;
                gluUnProject(x, viewport[1] + viewport[3] - y, 0, modelMatrix, projMatrix, viewport, &nx, &ny, &nz);
                
                GLdouble fx, fy, fz;
                gluUnProject(x, viewport[1] + viewport[3] - y, 1, modelMatrix, projMatrix, viewport, &fx, &fy, &fz);

                // interpolate results
                GLfloat t = ny / (ny - fy);
                
                // so here are the desired (x, z) coordinates
                GLint wx = nx + (fx - nx) * t,
                    wz = nz + (fz - nz) * t;
                
                if (wx < 0 || wz < 0 || wx >= BOARD_SIZE || wz >= BOARD_SIZE) {
                    // clicked outside the board, cancel current selection
                    selection = -1;
                    glutPostRedisplay();
                } else {
                    
                    // PLEASE YONG, FIX IT HERE (I have the correct chessboard coordinates in
                    // wx, wz and there is for sure some piece selected is in "selection")
                    
                    // the click is in the board, make the move!
                    if(board[wx][wz] != EMPTY_CELL){
                        cout << "cell is full" << endl;
                        return;
                    }
                    int i= wx, j = wz;
                    if(objectList[selection].type != KNIGHT){
                        if((i != sRow && j != sCol) || (i == sRow && j == sCol))
                            return;
                        dRow = i;
                        dCol = j;
                        Move *p = new Other_Move(selection,sRow,sCol,dRow,dCol,FIRST,this);
                        int ert = glutGet( GLUT_ELAPSED_TIME );
                        p->setDuration(500 * (abs(sRow - dRow) + abs(sCol -dCol) ));
                    
                        finishAllMove();
                        activeList.push_back(pair<int, Move *>(ert,p));
                        dRow = i;
                        dCol = j;
                        board[sRow][sCol] = EMPTY_CELL;
                        board[dRow][dCol] = selection;
                    
                        sRow = dRow;
                        sCol = dCol;
                    }else{
                        //cout<<"KNIGHT"<<endl;
                        if(abs(i - sRow) + abs(j - sCol) != 3 || (i == sRow || j == sCol) )
                            return;
                    
                        dRow = i;
                        dCol = j;
                        Move *p = new Knight_Move(selection,sRow,sCol,
                                    dRow,dCol, sRow, dCol, FIRST,this);
                        int ert = glutGet( GLUT_ELAPSED_TIME );
                        p->setDuration(500 * 3);
                    
                        finishAllMove();
                        activeList.push_back(pair<int, Move *>(ert,p));
                        dRow = i;
                        dCol = j;
                        board[sRow][sCol] = EMPTY_CELL;
                        board[dRow][dCol] = selection;
                        sRow = dRow;
                        sCol = dCol;
                    }//end for knight section
                }//end for valid(wx,wz)
            }//end for section != -1
        }//end if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !mouseDown)
    }//end mouse()

    /*
    * finish the previous movements
    */
    void finishAllMove(){
        for(int i = 0; i < activeList.size(); i++){
            activeList[i].second->finish();
            activeList[i].first -= activeList[i].second->duration();
        }
    }

    /** Callback for when the user moves the mouse with a button pressed. */
    void motion( int x, int y ) {
    }

    /** Callback for when the mouse is moved without a button being pressed. */
    void passiveMotion( int x, int y ) {
        // If 'a' is being held down, move the camera around.
        if ( keyPressed( 'a' ) ) {
            // Rotate the camera when a is pressed (for angle)
            camElevation = clamp( camElevation + ( y - lastMouseY ) / 2.0, 10, 80 );
            camRotation = camRotation + ( x - lastMouseX ) / 2.0;

            glutPostRedisplay();
        }

        // Snapshot the new mouse location, subsequent moves are handled
        // incrementally.
        lastMouseX = x;
        lastMouseY = y;
    }

    /** Step the state of any running move operations. */
    void step() {
        int ert = glutGet( GLUT_ELAPSED_TIME );
        
        if ( activeList.size() > 0 ) {
            for ( int i = activeList.size() - 1; i >= 0; i-- ) {
                // Figure out how far this move should be in its animation.
                double t = double( ert - activeList[ i ].first ) / 
                    activeList[ i ].second->duration();
                // See if this animation operation has run for long enough.
                if ( t >= 1 ) {
                    // If the operation is done, finish it up and put move it to the
                    // undo list.
                    activeList[ i ].second->finish();
                    if(activeList[i].second->getType() == UNDO){
                        //we can implement redo action there
                        //redoList.push_back( activeList[ i ].second );
                    }
                    else 
                        undoList.push_back( activeList[ i ].second );
                    activeList[i].second->finish();
                    activeList.erase( activeList.begin() + i );
                    
                } else {
                    activeList[ i ].second->step( t );
                    alpha = t;
                }
            }
            
            // If there are running moves, refresh the display.
            glutPostRedisplay();
        }
    }
};

// One static instance of our chess object.
static ChessBoard chessBoard;

// Stub function to intercept the display callback
//
void display() {
    chessBoard.display();
}

// Callback for when keys are pressed down.
void keyDown( unsigned char key, int x, int y ) {
    chessBoard.keyDown( key, x, y );
}

// Callback for when keys are released
void keyUp( unsigned char key, int x, int y ) {
    chessBoard.keyUp( key, x, y );
}

// Callback for when the mouse button is pressed.
void mouse( int button, int state, int x, int y ) {
    chessBoard.mouse( button, state, x, y );
}

// Callback for when the mouse is moved with a button pressed.
void motion( int x, int y ) {
    chessBoard.motion( x, y );
}

// Callback for when the mouse is moved without a button pressed.
void passiveMotion( int x, int y ) {
    chessBoard.passiveMotion( x, y );
}

// Callback called when our timer goes off.  Tell the chess object
// what the application time is in milliseconds.
void timerFunction( int value ) {
    chessBoard.step();
    // Register a new timer.
    glutTimerFunc( 20, timerFunction, 0 );
}

/////////////////////////////////////////////////////////////////
// Glut callback functions.
/////////////////////////////////////////////////////////////////

int main( int argc, char **argv ) {
    // Init glut and make a window.
    glutInit( &argc, argv );
   
    chessBoard.init( argc, argv );

    // Register all our callbacks.
    glutDisplayFunc( display );
    glutIgnoreKeyRepeat( true );
    glutKeyboardFunc( keyDown );
    glutKeyboardUpFunc( keyUp );
    glutMouseFunc( mouse );
    glutMotionFunc( motion );
    glutPassiveMotionFunc( passiveMotion );

    // Register our first timer callback.
    glutTimerFunc( 20, timerFunction, 0 );

    // Let glut handle UI events.
    glutMainLoop();

    return 0;
}

