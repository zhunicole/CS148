// main.cpp

//
// For this project, we use OpenGL, GLUT
// and GLEW (to load OpenGL extensions)
//
#include "stglew.h"

#include <stdio.h>
#include <string.h>

//
// Globals used by this application.
// As a rule, globals are Evil, but this is a small application
// and the design of GLUT makes it hard to avoid them.
//

// Window size, kept for screenshots
static int gWindowSizeX = 0;
static int gWindowSizeY = 0;

// File locations
std::string vertexShader;
std::string fragmentShader;
std::string meshOBJ;
std::string meshOBJ2;

// Light source attributes
static float specularLight[] = {1.00, 1.00, 1.00, 1.0};
static float ambientLight[]  = {0.10, 0.10, 0.10, 1.0};
static float diffuseLight[]  = {1.00, 1.00, 1.00, 1.0};
float lightPosition[] = {2.7367f, 2.5669f, 2.2063f, 1.0f};
float spotDirectionLight[]  = {0.00, 0.00, 0.00};


static float specularLightSun[] = {0.9808, 0.1807, 0.1669, 1.0};
static float diffuseLightSun[]  = {0.9, 0.1807, 0.1669, 1.0};
static float ambientLightSun[]  = {0.1, 0.1807, 0.1669, 1.0};

float lightPositionSun[] = {0.0f, 50.0f, 0.0f, 1.0f};


// Material color properties
static float materialAmbient[]  = { 0.2, 0.2, 0.6, 1.0 };
static float materialDiffuse[]  = { 0.2, 0.2, 0.6, 1.0 };
static float materialSpecular[] = { 0.8, 0.8, 0.8, 1.0 };
static float shininess          = 100.0;  // # between 1 and 128.

static float materialAmbientWhite[]  = { 0.95864661654, 0.8984962406, 0.80075187969, 1.0 };
static float materialDiffuseWhite[]  = { 0.95864661654, 0.8984962406, 0.80075187969, 1.0 };
static float materialSpecularWhite[] = { 0.99, 0.9, 0.9, 1.0 };


static float materialAmbientGreen[]  = { 0.0, .5751, 0.0, 1.0 };
static float materialDiffuseGreen[]  = { 0.0, .5751, 0.0, 1.0 };
static float materialSpecularGreen[] = { 0.99, 0.9, 0.9, 1.0 };


STShaderProgram *shader;

// Stored mouse position for camera rotation, panning, and zoom.
int gPreviousMouseX = -1;
int gPreviousMouseY = -1;
int gMouseButton = -1;
STVector3 mCameraTranslation;
float mCameraAzimuth;
float mCameraElevation;
bool mesh = false; // draw mesh
bool smooth = false; // smooth/flat shading for mesh

STTriangleMesh* gTriangleMesh = 0;
STTriangleMesh* gTriangleMesh2 = 0;
STTriangleMesh* gManualTriangleMesh = 0;

void resetCamera()
{
    mCameraTranslation = STVector3(0.0f, 0.f, 13.f);
    mCameraAzimuth = 0.f;
    mCameraElevation = 85.0f;
}

void CreateYourOwnMesh()
{
    float leftX   = -2.0f;
    float rightX  = -leftX;
    float nearZ   = -2.0f;
    float farZ    = -nearZ;

    int XTesselationDepth = 2;
    int ZTesselationDepth = 2;
    gManualTriangleMesh= new STTriangleMesh();
    for (int i = 0; i < XTesselationDepth+1; i++){
        for (int j = 0; j < ZTesselationDepth+1; j++) {
            float s0 = (float) i / (float) XTesselationDepth;
            float x0 =  s0 * (rightX - leftX) + leftX;
            float t0 = (float) j / (float) ZTesselationDepth;
            float z0 = t0 * (farZ - nearZ) + nearZ;

            gManualTriangleMesh->AddVertex(x0,(x0*x0+z0*z0)*0.15f,z0);
        }
    }
    for (int i = 0; i < XTesselationDepth; i++){
        for (int j = 0; j < ZTesselationDepth; j++) {
            unsigned int id0=i*(ZTesselationDepth+1)+j;
            unsigned int id1=(i+1)*(ZTesselationDepth+1)+j;
            unsigned int id2=(i+1)*(ZTesselationDepth+1)+j+1;
            unsigned int id3=i*(ZTesselationDepth+1)+j+1;
            gManualTriangleMesh->AddFace(id0,id2,id1);
            gManualTriangleMesh->AddFace(id0,id3,id2);
        }
    }
    gManualTriangleMesh->Build();
}
//
// Initialize the application, loading all of the settings that
// we will be accessing later in our fragment shaders.
//
void Setup()
{
    // Set up lighting variables in OpenGL
    // Once we do this, we will be able to access them as built-in
    // attributes in the shader (see examples of this in normalmap.frag)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_SPECULAR,  specularLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT,   ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,   diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION,   spotDirectionLight);
    
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_AMBIENT,  ambientLightSun);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specularLightSun);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  diffuseLightSun);

    // Ditto with accessing material properties in the fragment
    // and vertex shaders.
    glMaterialfv(GL_FRONT, GL_AMBIENT,   materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);

    shader = new STShaderProgram();
    shader->LoadVertexShader(vertexShader);
    shader->LoadFragmentShader(fragmentShader);

    resetCamera();
    
    glClearColor(0.0f, 0.0f, 0.3834f, 1.0f);
    
    glEnable(GL_DEPTH_TEST);

    gTriangleMesh=new STTriangleMesh(meshOBJ);
    gTriangleMesh2=new STTriangleMesh(meshOBJ2);

    CreateYourOwnMesh();
}

void CleanUp()
{
    if(gTriangleMesh!=0)
        delete gTriangleMesh;
    if(gTriangleMesh2!=0)
        delete gTriangleMesh2;
    if(gManualTriangleMesh!=0)
        delete gManualTriangleMesh;
}

/**
 * Camera adjustment methods
 */
void AdjustCameraAzimuthBy(float delta)
{
    mCameraAzimuth += delta;
}

void AdjustCameraElevationBy(float delta)
{
    mCameraElevation += delta;
}

void AdjustCameraTranslationBy(STVector3 delta)
{
    mCameraTranslation += delta;
}

//
// Display the output image from our vertex and fragment shaders
//
void DisplayCallback()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(-mCameraTranslation.x, -mCameraTranslation.y, -mCameraTranslation.z);
    
    glRotatef(-mCameraElevation, 1, 0, 0);
    glRotatef(-mCameraAzimuth, 0, 1, 0);

    glRotatef(90.0f, 1, 0, 0);

    glPushMatrix();
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glPopMatrix();
    glLightfv(GL_LIGHT1, GL_POSITION, lightPositionSun);
    //light, name params

    // and vertex shaders
    glMaterialfv(GL_FRONT, GL_AMBIENT,   materialAmbientWhite);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   materialDiffuseWhite);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  materialSpecularWhite);
    glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
    
    // Invoke the shader.  Now OpenGL will call our
    // shader programs on anything we draw
    shader->Bind();

    if(mesh) {

        glPushMatrix();
        gTriangleMesh->Draw(smooth);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(3.0, 0.0, 0.0);
        glScalef(10,10,10);
        
        //sets tree color to green before drawing
        glMaterialfv(GL_FRONT, GL_AMBIENT,   materialAmbientGreen);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   materialDiffuseGreen);
        glMaterialfv(GL_FRONT, GL_SPECULAR,  materialSpecularGreen);
        gTriangleMesh2->Draw(smooth);
        glPopMatrix();

    } else
        gManualTriangleMesh->Draw(smooth);
    
    
    // Draw ground
    glColor3f(0.239f, 0.341f, 0.278f);
    glNormal3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-1500.0f, -8.0f, -1500.0f);
    glVertex3f(-1500.0f, -8.0f,  1500.0f);
    glVertex3f( 1500.0f, -8.0f,  1500.0f);
    glVertex3f( 1500.0f, -8.0f, -1500.0f);
    glEnd();
    
    
    shader->UnBind();
    glutSwapBuffers();
}

//
// Reshape the window and record the size so
// that we can use it for screenshots.
//
void ReshapeCallback(int w, int h)
{
	gWindowSizeX = w;
    gWindowSizeY = h;

    glViewport(0, 0, gWindowSizeX, gWindowSizeY);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	// Set up a perspective projection
    float aspectRatio = (float) gWindowSizeX / (float) gWindowSizeY;
	gluPerspective(30.0f, aspectRatio, .1f, 10000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void SpecialKeyCallback(int key, int x, int y)
{
    switch(key) {
        case GLUT_KEY_LEFT:
            AdjustCameraTranslationBy(STVector3(-0.2,0,0));
            break;
        case GLUT_KEY_RIGHT:
            AdjustCameraTranslationBy(STVector3(0.2,0,0));
            break;
        case GLUT_KEY_DOWN:
            AdjustCameraTranslationBy(STVector3(0,-0.2,0));
            break;
        case GLUT_KEY_UP:
            AdjustCameraTranslationBy(STVector3(0,0.2,0));
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void KeyCallback(unsigned char key, int x, int y)
{
    switch(key) {
    case 's': {
            //
            // Take a screenshot, and save as screenshot.jpg
            //
            STImage* screenshot = new STImage(gWindowSizeX, gWindowSizeY);
            screenshot->Read(0,0);
            screenshot->Save("screenshot.jpg");
            delete screenshot;
        }
        break;
    case 'r':
        resetCamera();
        break;
    case 'm': // switch between the mesh you create and the mesh from file
        mesh = !mesh;
        break;
    case 'f': // switch between smooth shading and flat shading
        smooth = !smooth;
        break;
    case 'l': // do loop subdivision
        if(mesh){
            gTriangleMesh->LoopSubdivide();
            gTriangleMesh2->LoopSubdivide();
        }else
            gManualTriangleMesh->LoopSubdivide();
        break;
    case 'w':
        gTriangleMesh->Write("output.obj");
        gTriangleMesh2->Write("output.obj");
        break;
	case 'q':
		exit(0);
    default:
        break;
    }

    glutPostRedisplay();
}

/**
 * Mouse event handler
 */
void MouseCallback(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON)
    {
        gMouseButton = button;
    } else
    {
        gMouseButton = -1;
    }
    
    if (state == GLUT_UP)
    {
        gPreviousMouseX = -1;
        gPreviousMouseY = -1;
    }
}

/**
 * Mouse active motion callback (when button is pressed)
 */
void MouseMotionCallback(int x, int y)
{
    if (gPreviousMouseX >= 0 && gPreviousMouseY >= 0)
    {
        //compute delta
        float deltaX = x-gPreviousMouseX;
        float deltaY = y-gPreviousMouseY;
        gPreviousMouseX = x;
        gPreviousMouseY = y;
        
        float zoomSensitivity = 0.2f;
        float rotateSensitivity = 0.5f;
        
        //orbit or zoom
        if (gMouseButton == GLUT_LEFT_BUTTON)
        {
            AdjustCameraAzimuthBy(-deltaX*rotateSensitivity);
            AdjustCameraElevationBy(-deltaY*rotateSensitivity);
        } 
        else if (gMouseButton == GLUT_RIGHT_BUTTON)
        {
            STVector3 zoom(0,0,deltaY);
            AdjustCameraTranslationBy(zoom * zoomSensitivity);
        }
        
    } else
    {
        gPreviousMouseX = x;
        gPreviousMouseY = y;
    }
    
}

void usage()
{
	printf("usage: assignment2 vertShader fragShader objMeshFile\n");
	exit(0);
}

int main(int argc, char** argv)
{
	if (argc != 5)
		usage();

	vertexShader   = std::string(argv[1]);
	fragmentShader = std::string(argv[2]);
    meshOBJ        = std::string(argv[3]);
    meshOBJ2        = std::string(argv[4]);
    

    //
    // Initialize GLUT.
    //
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(20, 20);
    glutInitWindowSize(640, 480);
    glutCreateWindow("CS148 Assignment 2");
    
    //
    // Initialize GLEW.
    //
#ifndef __APPLE__
    glewInit();
    if(!GLEW_VERSION_2_0) {
        printf("Your graphics card or graphics driver does\n"
			   "\tnot support OpenGL 2.0, trying ARB extensions\n");

        if(!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader) {
            printf("ARB extensions don't work either.\n");
            printf("\tYou can try updating your graphics drivers.\n"
				   "\tIf that does not work, you will have to find\n");
            printf("\ta machine with a newer graphics card.\n");
            exit(1);
        }
    }
#endif

    // Be sure to initialize GLUT (and GLEW for this assignment) before
    // initializing your application.

    Setup();

    glutDisplayFunc(DisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutSpecialFunc(SpecialKeyCallback);
    glutKeyboardFunc(KeyCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MouseMotionCallback);
    glutIdleFunc(DisplayCallback);

    glutMainLoop();

    // Cleanup code should be called here.
    CleanUp();

    return 0;
}
