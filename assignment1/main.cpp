#include <stdlib.h>
#include <stdio.h>
//#include <Foundation/Foundation.h>
#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Question 1: In a GLUT program, how is control passed
// back to the programmer?  How is this set up during
// initialization?

int win_width = 512;
int win_height = 512;
int num_branches = 2;

#define BRANCH_DECREASE 0.8;
#define ANGLE 0.2;

//draws the branch using triangle strips
//creates width of strip by offseting y' from y by width
void drawBranch(float length, float width) {
    glPushMatrix();

    glBegin(GL_TRIANGLE_STRIP);

    glColor3f(0.f,1.f,0.f);
    glVertex2f(0.f, 0.f);
    glVertex2f(0.f, 0.f + length);
    glVertex2f(0.f + width, 0.f);
    glVertex2f(0.f + width, 0.f + length);
    glEnd();
    glPopMatrix();
    
    printf("drew");
}


//x and y updates with movement

void tree(float length, int depth)
{
    if (length <0.001 || depth < 0) {
        return;
    } else {
        float angle;
        // Draw
  
        glPushMatrix();
//        glRotatef(-90, 1.0,0.0,0.0);
        drawBranch(length, 0.01f);
        glPopMatrix();
        
        glTranslatef(0.0, length, 0.0);
        
        length = length * BRANCH_DECREASE;
        depth = depth - 1;
        //drawing 3 sets branches
        for(int a= 0; a<3; a++){
            angle = rand()%50+20;
            if(angle >48) angle = -(rand()%50+20);
            glPushMatrix();
            glRotatef(angle,1,0.0,1);
            tree(length, depth);
            glPopMatrix();

        }
    }
}


// callback everytime things need to be redrawn
void display( void )
{
    glClearColor(1.0, 1.0, 1.0, 1.0);   //sets clear color
    glClearDepth(1.0);  //? what is depth
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //sets window to clear color
    
    //need to call these before any projection tranformations
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    //draw trunk here
    //first branches
    glTranslatef(0.5f, 0.2f, 0.0f);

    tree(0.2f,3);
    
    glutSwapBuffers();
}


// new rendering canvas, define new coord system
void reshape( int w, int h )
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    
    // Question 3: What do the calls to glOrtho()
    // and glViewport() accomplish?
    
    //ANS: glOrtho specs coordinate system OpenGL assumes, how image is mapped to screen
    // glViewPort adjusts the pixel rectangle for drawing to be the entire new window.
    glOrtho( 0., 1., 0., 1., -1., 1. );
    glViewport( 0, 0, w, h );   //adjusts the pixel rectangle for drawing to be the entire new window.
    
    win_width = w;
    win_height = h;
    
    glutPostRedisplay();
}

void keyboard( unsigned char key, int x, int y ) {
    switch(key) {
        case '1':// Press number keys to switch the draw mode
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            num_branches=key-'0';
            glutPostRedisplay();
            break;
        case 27: // Escape key
            exit(0);
            break;
    }
}

int main (int argc, char *argv[]) {
    
    glutInit( &argc, argv );
    // Question 2: What does the parameter to glutInitDisplayMode()
    // specify?
    
    //ANS: specifies whether to use an RGBA or color-index color model. You can also specify whether you want a single- or double-buffered window. (If you’re working in color-index mode, you’ll want to load certain colors into the color map; use glutSetColor() to do this.) Finally, you can use this routine to indicate that you want the window to have an associated depth, stencil, multisampling, and/or accumulation buffer. For example, if you want a window with double buffering, the RGBA color model, and a depth buffer, you might call glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH).
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
    glutInitWindowSize( win_width, win_height );
    
    glutCreateWindow( "Intro Graphics Assignment 1" );
    
    glutDisplayFunc( display );   //impt redisplay callback fn
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    
    glutMainLoop(); //called registered disp callback
}



