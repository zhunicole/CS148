
// main.cpp

//
// For this project, we use OpenGL, GLUT
// and GLEW (to load OpenGL extensions)
//
#include "stglew.h"
#include <stdio.h>
#include <string.h>

#include <vector>
#include <math.h>
#include <time.h>
#include "camera.h"

//
// Globals used by this application.
// As a rule, globals are Evil, but this is a small application
// and the design of GLUT makes it hard to avoid them.
//

// Window size, kept for screenshots
static int gWindowSizeX = 0;
static int gWindowSizeY = 0;


using namespace std;
#define PI 3.14159265359
struct SOscillator
{
	GLfloat x,y,z;
	GLfloat nx,ny,nz;  //normal vector
	GLfloat UpSpeed;
	GLfloat newY;
	bool bIsExciter;
	//only in use, if bIsExciter is true:
	float ExciterAmplitude;
	float ExciterFrequency;
};

//Constants:
#define NUM_X_OSCILLATORS		50 //width
#define NUM_Z_OSCILLATORS		300 //length
#define NUM_OSCILLATORS			NUM_X_OSCILLATORS*NUM_Z_OSCILLATORS
#define OSCILLATOR_DISTANCE		.15

#define OSCILLATOR_WEIGHT       0.0002



//vertex data for the waves:
SOscillator * Oscillators;
int NumOscillators;  //size of the vertex array
vector <GLuint> IndexVect;  //we first put the indices into this vector, then copy them to the array below
GLuint * Indices;
int NumIndices;   //size of the index array

float g_timePassedSinceStart = 0.0f;  //note: this need not be the real time
bool  g_bExcitersInUse = true;

///////////////////////////////////////////////////
//Required for calculating the normals:
GLfloat GetF3dVectorLength( SF3dVector * v)
{
	return (GLfloat)(sqrt(v->x*v->x+v->y*v->y+v->z*v->z));
}
SF3dVector CrossProduct (SF3dVector * u, SF3dVector * v)
{
	SF3dVector resVector;
	resVector.x = u->y*v->z - u->z*v->y;
	resVector.y = u->z*v->x - u->x*v->z;
	resVector.z = u->x*v->y - u->y*v->x;
	return resVector;
}
SF3dVector Normalize3dVector( SF3dVector v)
{
	SF3dVector res;
	float l = GetF3dVectorLength(&v);
	if (l == 0.0f) {
		SF3dVector tmp;
		tmp.x = 0.0f;
		tmp.y = 0.0f;
		tmp.z = 0.0f;
		return tmp;
	}
	res.x = v.x / l;
	res.y = v.y / l;
	res.z = v.z / l;
	return res;
}
SF3dVector operator+ (SF3dVector v, SF3dVector u)
{
	SF3dVector res;
	res.x = v.x+u.x;
	res.y = v.y+u.y;
	res.z = v.z+u.z;
	return res;
}
SF3dVector operator- (SF3dVector v, SF3dVector u)
{
	SF3dVector res;
	res.x = v.x-u.x;
	res.y = v.y-u.y;
	res.z = v.z-u.z;
	return res;
}
///////////////////////////////////////////////////


void CreatePool()
{
	NumOscillators = NUM_OSCILLATORS;
	Oscillators = new SOscillator[NumOscillators];
	IndexVect.clear();  //to be sure it is empty
	for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++)
		for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++)
		{
			Oscillators[xc+zc*NUM_X_OSCILLATORS].x = OSCILLATOR_DISTANCE*float(xc);
			Oscillators[xc+zc*NUM_X_OSCILLATORS].y = 0.0f;
			Oscillators[xc+zc*NUM_X_OSCILLATORS].z = OSCILLATOR_DISTANCE*float(zc);
            
			Oscillators[xc+zc*NUM_X_OSCILLATORS].nx = 0.0f;
			Oscillators[xc+zc*NUM_X_OSCILLATORS].ny = 1.0f;
			Oscillators[xc+zc*NUM_X_OSCILLATORS].nz = 0.0f;
            
			Oscillators[xc+zc*NUM_X_OSCILLATORS].UpSpeed = 0;
			Oscillators[xc+zc*NUM_X_OSCILLATORS].bIsExciter = false;
            
			//create two triangles:
			if ((xc < NUM_X_OSCILLATORS-1) && (zc < NUM_Z_OSCILLATORS-1))
			{
				IndexVect.push_back(xc+zc*NUM_X_OSCILLATORS);
				IndexVect.push_back((xc+1)+zc*NUM_X_OSCILLATORS);
				IndexVect.push_back((xc+1)+(zc+1)*NUM_X_OSCILLATORS);
                
				IndexVect.push_back(xc+zc*NUM_X_OSCILLATORS);
				IndexVect.push_back((xc+1)+(zc+1)*NUM_X_OSCILLATORS);
				IndexVect.push_back(xc+(zc+1)*NUM_X_OSCILLATORS);
			}
            
		}
    
	//copy the indices:
	Indices = new GLuint[IndexVect.size()];  //allocate the required memory
	for (int i = 0; i < IndexVect.size(); i++)
	{
		Indices[i] = IndexVect[i];
	}
    
	Oscillators[100+30*NUM_X_OSCILLATORS].bIsExciter = true;
	Oscillators[100+30*NUM_X_OSCILLATORS].ExciterAmplitude = 0.5f; //changed
	Oscillators[100+30*NUM_X_OSCILLATORS].ExciterFrequency = 50.0f;
	Oscillators[30+80*NUM_X_OSCILLATORS].bIsExciter = true;
	Oscillators[30+80*NUM_X_OSCILLATORS].ExciterAmplitude = 0.5f;
	Oscillators[30+80*NUM_X_OSCILLATORS].ExciterFrequency = 50.0f;
	NumIndices = IndexVect.size();
	IndexVect.clear();  //no longer needed, takes only memory
}


void UpdateScene(bool bEndIsFree, float deltaTime, float time)
{
	//********
	// Here we do the physical calculations:
	// The oscillators are moved according to their neighbors.
	// The parameter bEndIsFree indicates, whether the oscillators in the edges can move or not.
	// The new position may be assigned not before all calculations are done!
    
	for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++)
	{
		for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++)
		{
			int ArrayPos = xc+zc*NUM_X_OSCILLATORS;
            
			//check, if oscillator is an exciter (these are not affected by other oscillators)
			if ((Oscillators[ArrayPos].bIsExciter) && g_bExcitersInUse)
			{
				Oscillators[ArrayPos].newY = Oscillators[ArrayPos].ExciterAmplitude*sin(time*Oscillators[ArrayPos].ExciterFrequency);
			}
            
            
			//check, if this oscillator is on an edge (=>end)
			if ((xc==0) || (xc==NUM_X_OSCILLATORS-1) || (zc==0) || (zc==NUM_Z_OSCILLATORS-1))
				;//TBD: calculating oscillators at the edge (if the end is free)
			else
			{
				//calculate the new speed:
                
				//Change the speed (=accelerate) according to the oscillator's 4 direct neighbors:
				float AvgDifference = Oscillators[ArrayPos-1].y				//left neighbor
                +Oscillators[ArrayPos+1].y				//right neighbor
                +Oscillators[ArrayPos-NUM_X_OSCILLATORS].y  //upper neighbor
                +Oscillators[ArrayPos+NUM_X_OSCILLATORS].y  //lower neighbor
                -4*Oscillators[ArrayPos].y;				//subtract the pos of the current osc. 4 times
				Oscillators[ArrayPos].UpSpeed += AvgDifference*deltaTime/OSCILLATOR_WEIGHT;
                
				//calculate the new position, but do not yet store it in "y" (this would affect the calculation of the other osc.s)
				Oscillators[ArrayPos].newY += Oscillators[ArrayPos].UpSpeed*deltaTime;
                
			}
		}
	}
	int xc;
    
	//copy the new position to y:
	for ( xc = 0; xc < NUM_X_OSCILLATORS; xc++)
	{
		for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++)
		{
			Oscillators[xc+zc*NUM_X_OSCILLATORS].y =Oscillators[xc+zc*NUM_X_OSCILLATORS].newY;
		}
	}
	//calculate new normal vectors (according to the oscillator's neighbors):
	for ( xc = 0; xc < NUM_X_OSCILLATORS; xc++)
	{
		for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++)
		{
			///
			//Calculating the normal:
			//Take the direction vectors 1.) from the left to the right neighbor
			// and 2.) from the upper to the lower neighbor.
			//The vector orthogonal to these
            
			SF3dVector u,v,p1,p2;	//u and v are direction vectors. p1 / p2: temporary used (storing the points)
			
			int value = 0;
			SF3dVector tmp;
			if (xc > 0) value = 1;
            
			else
				value = 0;
			tmp.x = Oscillators[xc-value+zc*NUM_X_OSCILLATORS].x;
			tmp.y = Oscillators[xc-value+zc*NUM_X_OSCILLATORS].y;
			tmp.z = Oscillators[xc-value+zc*NUM_X_OSCILLATORS].z;
			p1 = tmp;
			if (xc < NUM_X_OSCILLATORS-1)
				value =  1;
			else
				value = 0;
			tmp.x = Oscillators[xc+value+zc*NUM_X_OSCILLATORS].x;
			tmp.y = Oscillators[xc+value+zc*NUM_X_OSCILLATORS].y;
			tmp.z = Oscillators[xc+value+zc*NUM_X_OSCILLATORS].z;
			p2 = tmp;
			u = p2-p1; //vector from the left neighbor to the right neighbor
			if (zc > 0)
				value = 1;
			else
				value = 0;
			tmp.x = Oscillators[xc+(zc-value)*NUM_X_OSCILLATORS].x;
			tmp.y = Oscillators[xc+(zc-value)*NUM_X_OSCILLATORS].y;
			tmp.z = Oscillators[xc+(zc-value)*NUM_X_OSCILLATORS].z;
			p1 = tmp;
			if (zc < NUM_Z_OSCILLATORS-1)
				value = 1;
			else
				value = 0;
			tmp.x = Oscillators[xc+(zc+value)*NUM_X_OSCILLATORS].x;
            tmp.y = Oscillators[xc+(zc+value)*NUM_X_OSCILLATORS].y;
            tmp.z = Oscillators[xc+(zc+value)*NUM_X_OSCILLATORS].z;
			p2 = tmp;
			v = p2-p1; //vector from the upper neighbor to the lower neighbor
			//calculat the normal:
			SF3dVector normal = Normalize3dVector(CrossProduct(&u,&v));
            
			//assign the normal:
			Oscillators[xc+zc*NUM_X_OSCILLATORS].nx = normal.x;
			Oscillators[xc+zc*NUM_X_OSCILLATORS].ny = normal.y;
			Oscillators[xc+zc*NUM_X_OSCILLATORS].nz = normal.z;
		}
	}
    
}

void DrawScene(void)
{
	glDrawElements(	GL_TRIANGLES, //mode
                   NumIndices,  //count, ie. how many indices
                   GL_UNSIGNED_INT, //type of the index array
                   Indices);;
    
}


// File locations
std::string vertexShader;
std::string fragmentShader;
std::string normalMap;
std::string displacementMap;
std::string meshOBJPiano;
std::string meshOBJShelf;
std::string meshOBJCarpet;
std::string meshOBJWall;
std::string meshOBJWall2;

// Light source attributes
static float specularLight[] = /*{0.949, 0.137, 0.137, 1.0};*/{1.00, 1.00, 1.00, 1.0};
static float ambientLight[]  = {0.10, 0.10, 0.10, 1.0};
static float diffuseLight[]  = {1.00, 1.00, 1.00, 1.0};
float lightPosition[] = {0.f,10.f,10.f};/*{0.f, 10.f, 0.f};*///{10.7367f, 15.5669f, 10.2063f, 1.0f};
float spotDirectionLight[]  = {0.00, 0.00, 0.00};


/*static float specularLightSun[] = {0.9808, 0.1807, 0.1669, 1.0};
 static float diffuseLightSun[]  = {0.9, 0.1807, 0.1669, 1.0};
 static float ambientLightSun[]  = {0.1, 0.1807, 0.1669, 1.0};
 */

static float specularLightSun[] = {0.949, 0.137, 0.137, 1.0};
static float diffuseLightSun[]  = {0.9, 0.1807, 0.1669, 1.0};
static float ambientLightSun[]  = {0.1, 0.1807, 0.1669, 1.0};

float lightPositionSun[] = {20.0f, 50.0f, 20.0f, 1.0f};


STImage   *surfaceNormImg;
STTexture *surfaceNormTex;

STImage   *surfaceDisplaceImg;
STTexture *surfaceDisplaceTex;

STShaderProgram *shader;

STVector3 mPosition;
STVector3 mLookAt;
STVector3 mRight;
STVector3 mUp;

// Stored mouse position for camera rotation, panning, and zoom.
int gPreviousMouseX = -1;
int gPreviousMouseY = -1;
int gMouseButton = -1;
bool mesh = false; // draw mesh
bool smooth = true; // smooth/flat shading for mesh
bool normalMapping = true; // true=normalMapping, false=displacementMapping
bool proxyType=false; // false: use cylinder; true: use sphere

std::vector<STTriangleMesh*> gTriangleMeshes;
std::vector<STTriangleMesh*> gTriangleMeshes2;
std::vector<STTriangleMesh*> gTriangleMeshes3;
std::vector<STTriangleMesh*> gTriangleMeshes4;
std::vector<STTriangleMesh*> gTriangleMeshes5;

STPoint3 gMassCenter;
std::pair<STPoint3,STPoint3> gBoundingBox;

STTriangleMesh* gManualTriangleMesh = 0;

int TesselationDepth = 100;

void SetUpAndRight()
{
	mRight = STVector3::Cross(mLookAt - mPosition, mUp);
	mRight.Normalize();
	mUp = STVector3::Cross(mRight, mLookAt - mPosition);
	mUp.Normalize();
}

void resetCamera()
{
	//here
	mLookAt=STVector3(0.f,5.f,0.f);
	mPosition=STVector3(30.f,4.f,0.f);
	//          mPosition=STVector3(0.f,5.f,30.f);
	mUp=STVector3(0.f,1.f,0.f);
    
	SetUpAndRight();
}

void resetUp()
{
	mUp = STVector3(0.f,1.f,0.f);
	mRight = STVector3::Cross(mLookAt - mPosition, mUp);
	mRight.Normalize();
	mUp = STVector3::Cross(mRight, mLookAt - mPosition);
	mUp.Normalize();
}

void CreateYourOwnMesh()
{
	float leftX   = -2.0f;
	float rightX  = -leftX;
	float nearZ   = -2.0f;
	float farZ    = -nearZ;
    
	gManualTriangleMesh= new STTriangleMesh();
	for (int i = 0; i < TesselationDepth+1; i++){
		for (int j = 0; j < TesselationDepth+1; j++) {
			float s0 = (float) i / (float) TesselationDepth;
			float x0 =  s0 * (rightX - leftX) + leftX;
			float t0 = (float) j / (float) TesselationDepth;
			float z0 = t0 * (farZ - nearZ) + nearZ;
            
			gManualTriangleMesh->AddVertex(x0,(x0*x0+z0*z0)*0.0f,z0,s0,t0);
		}
	}
	for (int i = 0; i < TesselationDepth; i++){
		for (int j = 0; j < TesselationDepth; j++) {
			unsigned int id0=i*(TesselationDepth+1)+j;
			unsigned int id1=(i+1)*(TesselationDepth+1)+j;
			unsigned int id2=(i+1)*(TesselationDepth+1)+j+1;
			unsigned int id3=i*(TesselationDepth+1)+j+1;
			gManualTriangleMesh->AddFace(id0,id2,id1);
			gManualTriangleMesh->AddFace(id0,id3,id2);
		}
	}
	gManualTriangleMesh->Build();
	gManualTriangleMesh->mMaterialAmbient[0]=0.2f;
	gManualTriangleMesh->mMaterialAmbient[1]=0.2f;
	gManualTriangleMesh->mMaterialAmbient[2]=0.6f;
	gManualTriangleMesh->mMaterialDiffuse[0]=0.2f;
	gManualTriangleMesh->mMaterialDiffuse[1]=0.2f;
	gManualTriangleMesh->mMaterialDiffuse[2]=0.6f;
	gManualTriangleMesh->mMaterialSpecular[0]=0.6f;
	gManualTriangleMesh->mMaterialSpecular[1]=0.6f;
	gManualTriangleMesh->mMaterialSpecular[2]=0.6f;
	gManualTriangleMesh->mShininess=8.0f;
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
	//    glEnable(GL_LIGHTING);
	//    glEnable(GL_LIGHT0);
	//    glLightfv(GL_LIGHT0, GL_SPECULAR,  specularLight);
	//    glLightfv(GL_LIGHT0, GL_AMBIENT,   ambientLight);
	//    glLightfv(GL_LIGHT0, GL_DIFFUSE,   diffuseLight);
    
    
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
    
    
    
    
    
	surfaceNormImg = new STImage(normalMap);
	surfaceNormTex = new STTexture(surfaceNormImg);
    
	surfaceDisplaceImg = new STImage(displacementMap);
	surfaceDisplaceTex = new STTexture(surfaceDisplaceImg);
    
	shader = new STShaderProgram();
	shader->LoadVertexShader(vertexShader);
	shader->LoadFragmentShader(fragmentShader);
    
	resetCamera();
    
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
    
	//CARPET
	STTriangleMesh::LoadObj(gTriangleMeshes3,meshOBJCarpet);
	gMassCenter=STTriangleMesh::GetMassCenter(gTriangleMeshes3);
	std::cout<<"Mass Center: "<<gMassCenter<<std::endl;
	gBoundingBox=STTriangleMesh::GetBoundingBox(gTriangleMeshes3);
	std::cout<<"Bounding Box: "<<gBoundingBox.first<<" - "<<gBoundingBox.second<<std::endl;
    
	//WALL
	STTriangleMesh::LoadObj(gTriangleMeshes4,meshOBJWall);
	gMassCenter=STTriangleMesh::GetMassCenter(gTriangleMeshes4);
	std::cout<<"Mass Center: "<<gMassCenter<<std::endl;
	gBoundingBox=STTriangleMesh::GetBoundingBox(gTriangleMeshes4);
	std::cout<<"Bounding Box: "<<gBoundingBox.first<<" - "<<gBoundingBox.second<<std::endl;
    
	//WALL #2
	STTriangleMesh::LoadObj(gTriangleMeshes5,meshOBJWall2);
	gMassCenter=STTriangleMesh::GetMassCenter(gTriangleMeshes5);
	std::cout<<"Mass Center: "<<gMassCenter<<std::endl;
	gBoundingBox=STTriangleMesh::GetBoundingBox(gTriangleMeshes5);
	std::cout<<"Bounding Box: "<<gBoundingBox.first<<" - "<<gBoundingBox.second<<std::endl;
    
	//PIANO AND CHAIR
	STTriangleMesh::LoadObj(gTriangleMeshes,meshOBJPiano);
	gMassCenter=STTriangleMesh::GetMassCenter(gTriangleMeshes);
	std::cout<<"Mass Center: "<<gMassCenter<<std::endl;
	gBoundingBox=STTriangleMesh::GetBoundingBox(gTriangleMeshes);
	std::cout<<"Bounding Box: "<<gBoundingBox.first<<" - "<<gBoundingBox.second<<std::endl;
    
	//SHELF
	STTriangleMesh::LoadObj(gTriangleMeshes2,meshOBJShelf);
	gMassCenter=STTriangleMesh::GetMassCenter(gTriangleMeshes2);
	std::cout<<"Mass Center: "<<gMassCenter<<std::endl;
	gBoundingBox=STTriangleMesh::GetBoundingBox(gTriangleMeshes2);
	std::cout<<"Bounding Box: "<<gBoundingBox.first<<" - "<<gBoundingBox.second<<std::endl;
    
    
	//for(unsigned int id=0;id<gTriangleMeshes.size();id++)
	//    gTriangleMeshes[id]->Recenter(gMassCenter);
    
	//for(unsigned int id=0;id<gTriangleMeshes.size();id++){
	//    if(proxyType) gTriangleMeshes[id]->CalculateTextureCoordinatesViaSphericalProxy();
	//    else gTriangleMeshes[id]->CalculateTextureCoordinatesViaCylindricalProxy(-6700,6700,0,0,2);
	//}
    
	CreateYourOwnMesh();
}

void CleanUp()
{
	for(unsigned int id=0;id<gTriangleMeshes.size();id++)delete gTriangleMeshes[id];
	if(gManualTriangleMesh!=0)
		delete gManualTriangleMesh;
    
	for(unsigned int id=0;id<gTriangleMeshes2.size();id++)delete gTriangleMeshes2[id];
	if(gManualTriangleMesh!=0)
		delete gManualTriangleMesh;
}

/**
 * Camera adjustment methods
 */
void RotateCamera(float delta_x, float delta_y)
{
	float yaw_rate=1.f;
	float pitch_rate=1.f;
    
	mPosition -= mLookAt;
	STMatrix4 m;
	m.EncodeR(-1*delta_x*yaw_rate, mUp);
	mPosition = m * mPosition;
	m.EncodeR(-1*delta_y*pitch_rate, mRight);
	mPosition = m * mPosition;
    
	mPosition += mLookAt;
}

void ZoomCamera(float delta_y)
{
	STVector3 direction = mLookAt - mPosition;
	float magnitude = direction.Length();
	direction.Normalize();
	float zoom_rate = 0.1f*magnitude < 0.5f ? .1f*magnitude : .5f;
	if(delta_y * zoom_rate + magnitude > 0)
	{
		mPosition += (delta_y * zoom_rate) * direction;
	}
}

void StrafeCamera(float delta_x, float delta_y)
{
	float strafe_rate = 0.05f;
    
	mPosition -= strafe_rate * delta_x * mRight;
	mLookAt   -= strafe_rate * delta_x * mRight;
	mPosition += strafe_rate * delta_y * mUp;
	mLookAt   += strafe_rate * delta_y * mUp;
}


void drawPoolBottom(){
    
	//DRAW underside of pool
	glColor3f(0.239f, 0.741f, 0.278f);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	glBegin(GL_QUADS);
	glVertex3f(-1.9f, -0.5f, 6.0f);
	glVertex3f(-1.9f, -0.5f,  17.0f);
	glVertex3f( 1.9f, -0.5f,  17.0f);
	glVertex3f( 1.9f, -0.5f, 6.0f);
	glEnd();
	glPopMatrix();
}

//
// Display the output image from our vertex and fragment shaders
//
void DisplayCallback()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
	SetUpAndRight();
    
	gluLookAt(mPosition.x,mPosition.y,mPosition.z,
              mLookAt.x,mLookAt.y,mLookAt.z,
              mUp.x,mUp.y,mUp.z);
    
    
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPositionSun);
    
    
    
	// Texture 0: surface normal map
	glActiveTexture(GL_TEXTURE0);
	surfaceNormTex->Bind();
    
	// Texture 1: surface normal map
	glActiveTexture(GL_TEXTURE1);
	surfaceDisplaceTex->Bind();
    
	// Bind the textures we've loaded into openGl to
	// the variable names we specify in the fragment
	// shader.
	shader->SetTexture("normalTex", 0);
	shader->SetTexture("displacementTex", 1);
	shader->SetTexture("colorTex", 2);
    
	// Invoke the shader.  Now OpenGL will call our
	// shader programs on anything we draw.
	shader->Bind();
    
	if(mesh)
	{
		shader->SetUniform("normalMapping", -1.0);
		shader->SetUniform("displacementMapping", -1.0);
		shader->SetUniform("colorMapping", 1.0);
        
		glPushMatrix();
		// Pay attention to scale
		STVector3 size_vector=gBoundingBox.second-gBoundingBox.first;
		float maxSize=(std::max)((std::max)(size_vector.x,size_vector.y),size_vector.z);
		glScalef(3.0f/maxSize,3.0f/maxSize,3.0f/maxSize);
		glTranslatef(-gMassCenter.x,-gMassCenter.y,-gMassCenter.z);
        
		//TajMahal
		for(unsigned int id=0;id<gTriangleMeshes.size();id++) {
			gTriangleMeshes[id]->Draw(smooth);
		}
		glPopMatrix();
        
		//		//Shelf
		//		glPushMatrix();
		//		glTranslatef(5.0, 4, -10.0);
		//		glScalef(5,5,5);
		//
		//		for(unsigned int id=0;id<gTriangleMeshes2.size();id++) {
		//			gTriangleMeshes2[id]->Draw(smooth);
		//		}
		//		glPopMatrix();
		//
		//Grass
		for (int i=-3; i<12; i++)
		{
			for (int j=-3; j<12; j++)
			{
				glPushMatrix();
				//glTranslatef(-90+i*10,-3,-30+j*10);
				//glTranslatef(-40+i*10,-3,-30+j*10);
				glTranslatef(-20+i*2.5,-3,-10+j*2.5);
				//glScalef(5,5,5);
				glScalef(1,1,1);
				//glTranslatef(10,20,20);
				for(unsigned int id=0;id<gTriangleMeshes3.size();id++) {
					gTriangleMeshes3[id]->Draw(smooth);
				}
				glPopMatrix();
			}
		}
		//
		//Sky
		glPushMatrix();
		glTranslatef(-80.f,15.f, 0.f);
		glScalef(0.20,0.31,0.37);//5);
		glRotatef(270,0,0,1);
		for(unsigned int id=0;id<gTriangleMeshes4.size();id++) {
			gTriangleMeshes4[id]->Draw(smooth);
		}
		glPopMatrix();
		//
		//Reflection
        //DRAW underside of pool
        /*glColor3f(0.239f, 0.741f, 0.278f);
         glNormal3f(1.0f, 1.0f, 1.0f);
         glPushMatrix();
         glRotatef(90, 0, 1, 0);
         glBegin(GL_QUADS);
         glVertex3f(-1.9f, -0.5f, 6.0f);
         glVertex3f(-1.9f, -0.5f,  17.0f);
         glVertex3f( 1.9f, -0.5f,  17.0f);
         glVertex3f( 1.9f, -0.5f, 6.0f);
         glEnd();
         glPopMatrix();
         */
		glPushMatrix();
		glScalef(0.18,0.18,0.02);
		glTranslatef(30.20f, -2.020, 0.5);
		for(unsigned int id=0;id<gTriangleMeshes4.size();id++) {
			gTriangleMeshes5[id]->Draw(smooth);
		}
		glPopMatrix();
        
	}
	else
	{
		if(normalMapping){
			shader->SetUniform("displacementMapping", -1.0);
			shader->SetUniform("normalMapping", 1.0);
			shader->SetUniform("colorMapping", -1.0);
		}
		else{
			shader->SetUniform("displacementMapping", 1.0);
			shader->SetUniform("normalMapping", -1.0);
			shader->SetUniform("colorMapping", -1.0);
			shader->SetUniform("TesselationDepth", TesselationDepth);
		}
		gManualTriangleMesh->Draw(smooth);
	}
    
	shader->UnBind();
    
	glActiveTexture(GL_TEXTURE0);
	surfaceNormTex->UnBind();
    
	glActiveTexture(GL_TEXTURE1);
	surfaceDisplaceTex->UnBind();
    
	//drawPoolBottom();
    
	//transparency
	glEnable(GL_BLEND);
	GLfloat mat_amb_diff[] = { 0.047,0,0.289,0.5};//0.1, 0.5, 0.8, 0.5 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,
                 mat_amb_diff);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
	glPushMatrix();
	//HERE
	glRotatef(90, 0, 1, 0);
	glTranslatef(-1.9, 0.1, 6.0);
	glScalef(.5, 1, .5);
    
	DrawScene();
	glPopMatrix();
    
	glDisable(GL_BLEND);
    
	glFlush();
    
	glutSwapBuffers();
}

void IdleCallback(void) {
	float dtime = 0.004f;  //if you want to be exact, you would have to replace this by the real time passed since the last frame (and probably divide it by a certain number)
	g_timePassedSinceStart += dtime;
    
	if (g_timePassedSinceStart > 1.7f)
	{
		g_bExcitersInUse = false;  //stop the exciters
	}
    
	//rain effect
	//     int randomNumber = rand();
	//     if (randomNumber < NUM_OSCILLATORS)
	//     {
	//     Oscillators[randomNumber].y = -0.05;
	//     }
    
	UpdateScene(false,dtime,g_timePassedSinceStart);
	DisplayCallback();
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
			StrafeCamera(10,0);
			break;
		case GLUT_KEY_RIGHT:
			StrafeCamera(-10,0);
			break;
		case GLUT_KEY_DOWN:
			StrafeCamera(0,-10);
			break;
		case GLUT_KEY_UP:
			StrafeCamera(0,10);
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
		case 'u':
            resetUp();
            break;
		case 'm': // switch between the mesh you create and the mesh from file
            mesh = !mesh;
            break;
            //case 'p': //switch proxy type between sphere and cylinder
            //	proxyType=!proxyType;
            //	if(proxyType) gTriangleMesh->CalculateTextureCoordinatesViaSphericalProxy();
            //	else gTriangleMesh->CalculateTextureCoordinatesViaCylindricalProxy(-1,1,0,0,1);
            //	break;
		case 'n': // switch between normalMapping and displacementMapping
            normalMapping = !normalMapping;
            break;
		case 'f': // switch between smooth shading and flat shading
            smooth = !smooth;
            break;
            //case 'l': // do loop subdivision
            //    if(mesh){
            //        gTriangleMesh->LoopSubdivide();
            //		if(proxyType) gTriangleMesh->CalculateTextureCoordinatesViaSphericalProxy();
            //		else gTriangleMesh->CalculateTextureCoordinatesViaCylindricalProxy(-1,1,0,0,1);
            //    }
            //    else
            //        gManualTriangleMesh->LoopSubdivide();
            //    break;
            //case 'w':
            //    gTriangleMesh->Write("output.obj");
            //    break;
		case 'a':
            for(unsigned int id=0;id<gTriangleMeshes.size();id++)
                gTriangleMeshes[id]->mDrawAxis=!gTriangleMeshes[id]->mDrawAxis;
            if(gManualTriangleMesh!=0)
                gManualTriangleMesh->mDrawAxis=!gManualTriangleMesh->mDrawAxis;
            for(unsigned int id=0;id<gTriangleMeshes2.size();id++)
                gTriangleMeshes2[id]->mDrawAxis=!gTriangleMeshes2[id]->mDrawAxis;
            for(unsigned int id=0;id<gTriangleMeshes3.size();id++)
                gTriangleMeshes3[id]->mDrawAxis=!gTriangleMeshes3[id]->mDrawAxis;
            for(unsigned int id=0;id<gTriangleMeshes4.size();id++)
                gTriangleMeshes4[id]->mDrawAxis=!gTriangleMeshes4[id]->mDrawAxis;
            for(unsigned int id=0;id<gTriangleMeshes5.size();id++)
                gTriangleMeshes5[id]->mDrawAxis=!gTriangleMeshes5[id]->mDrawAxis;
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
	if (button == GLUT_LEFT_BUTTON
        || button == GLUT_RIGHT_BUTTON
        || button == GLUT_MIDDLE_BUTTON)
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
        
		//orbit, strafe, or zoom
		if (gMouseButton == GLUT_LEFT_BUTTON)
		{
			RotateCamera(deltaX, deltaY);
		}
		else if (gMouseButton == GLUT_MIDDLE_BUTTON)
		{
			StrafeCamera(deltaX, deltaY);
		}
		else if (gMouseButton == GLUT_RIGHT_BUTTON)
		{
			ZoomCamera(deltaY);
		}
        
	} else
	{
		gPreviousMouseX = x;
		gPreviousMouseY = y;
	}
    
}

void usage()
{
	printf("usage: assignment3 vertShader fragShader objMeshFile normalMappingTexture displacementMappingTexture\n");
}





//--------------

//---------------

int main(int argc, char** argv)
{
	if (argc != 5)
		usage();
    
	vertexShader   = argc>1?std::string(argv[1]):std::string("kernels/default.vert");
	fragmentShader = argc>2?std::string(argv[2]):std::string("kernels/phong.frag");
	meshOBJPiano        = argc>3?std::string(argv[3]):std::string("meshes/tajmahal.obj");
	meshOBJShelf        = argc>4?std::string(argv[4]):std::string("meshes/Bookshelf.obj");
    
	normalMap      = argc>5?std::string(argv[5]):std::string("images/normalmap.png");
	displacementMap= argc>6?std::string(argv[6]):std::string("images/displacementmap.jpeg");
	meshOBJCarpet = argc>7?std::string(argv[7]):std::string("meshes/grass.obj");
	meshOBJWall = argc>8?std::string(argv[8]):std::string("meshes/carpet.obj");
	meshOBJWall2 = argc>9?std::string(argv[9]):std::string("meshes/reflection.obj");
    
	//
	// Initialize GLUT.
	//
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(20, 20);
	glutInitWindowSize(640, 480);
	glutCreateWindow("CS148 Assignment 3");
    
	CreatePool();
    
	//Enable the vertex array functionality:
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(	3,   //3 components per vertex (x,y,z)
                    GL_FLOAT,
                    sizeof(SOscillator),
                    Oscillators);
	glNormalPointer(	GL_FLOAT,
                    sizeof(SOscillator),
                    &Oscillators[0].nx);  //Pointer to the first color*/
	glPointSize(2.0);
	glClearColor(0.0,0.0,0.0,0.0);
    
	//Switch on solid rendering:
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
    
	glFrontFace(GL_CCW);   //Tell OGL which orientation shall be the front face
	glShadeModel(GL_SMOOTH);
    
	//initialize generation of random numbers:
	srand((unsigned)time(NULL));
    
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
	glutIdleFunc(IdleCallback);
    
    
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0, 1.0, 1.0, 1.0);
    
    
	glutMainLoop();
    
	// Cleanup code should be called here.
	CleanUp();
    
    
    
    
	return 0;
}

