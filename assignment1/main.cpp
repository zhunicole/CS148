#include <stdlib.h>

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
int draw_mode=8;


//draws the branch using triangle strips
//creates width of strip by offseting y' from y by width
void drawBranch(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2, float width) {
    glBegin(GL_TRIANGLE_STRIP);
    
    glColor3f(0.f,1.f,0.f);
    glVertex2f(x1, y1);
    glVertex2f(x1, y1 + width);
    glVertex2f(x2, y2);
    glVertex2f(x2, y2+width);
    
    glEnd();
    
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
    
    
    //keyboard input changes things
    //drawn using triangle strips
    glLineWidth(4.);
    glLoadIdentity();  //clear the currently modifiable matrix for future transformation commands
    
    drawBranch(0.2f, 0.2f, 0.5f, 0.5f, 0.01f);
    
    
    
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
            draw_mode=key-'0';
            glutPostRedisplay();
            break;
        case '8':
            draw_mode=key-'0';
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


