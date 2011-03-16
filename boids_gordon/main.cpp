//#include <Array3D.h>

#include <vector>
#include "glincludes.h"
//#include "struct.h"
#include <domain/IV.h>
#include "boids.h"

using namespace rtps;
using namespace std;

typedef vector<float4> VF;
typedef vector<int> VI;

/****
TODO: 
-- cone of vision
-- external velocity field
****/

// one global variable
Boids* boids;

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
Boids* initBoids()
{
	float4 center = float4(-75.,0.,0.,1.);
	float radius = 30.;
	float spacing = 10.0f;
	float scale = 1.f;

	float edge = 100.f;
	float offsetx = 0.f;
	float offsety = 0.f;
	float4 min = float4(-edge+offsetx, -edge+offsety, 0., 0.);
	float4 max = float4( edge+offsetx,  edge+offsety, 0., 0.);

	//int num = 4;// 2024;
	int num = 2024;
	VF pos(num);
	//pos = addCircle(num, center, radius, spacing, scale);
	GE_addRect(num, min, max, spacing, scale, pos);
	#if 0
	pos[0] = float4(-edge, -edge, 0., 1.);
	pos[1] = float4( edge, -edge, 0., 1.);
	pos[2] = float4( edge,  edge, 0., 1.);
	pos[3] = float4(-edge,  edge, 0., 1.);
	#endif

	VF vel, acc;
	acc.resize(pos.size());
	vel.resize(pos.size());

	//printf("before constructor: pos.size= %d\n", pos.size());
	Boids* boids = new Boids(pos);

	for (int i=0; i < vel.size(); i++) {
		vel[i] = float4(0.,0.,0.,1.);
		acc[i] = float4(0.,0.,0.,1.);
	}

	boids->set_ic(pos, vel, acc);
	return boids;
}
//----------------------------------------------------------------------
void display()
{
	static int count=0;

   boids->update();
   count++;
   //if (count > 1) for (;;) ;
      //exit(0);

   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // grid overlay based on desired min boid separation
   glBegin(GL_LINES);
   	glColor3f(.2,.2,.2);
	float dim = boids->getDomainSize();
	float sep = boids->getDesiredSeparation();
	int nb = 2*dim/sep;
	for (int j=0; j < nb; j++) {
	for (int i=0; i < nb; i++) {
		glVertex2f(-dim+i*sep, -dim+j*sep);
		glVertex2f(-dim+(i+1)*sep, -dim+j*sep);
	}}
	for (int i=0; i < nb; i++) {
	for (int j=0; j < nb; j++) {
		glVertex2f(-dim+i*sep, -dim+j*sep);
		glVertex2f(-dim+i*sep, -dim+(j+1)*sep);
	}}
   glEnd();

   VF& pos = boids->getPos();
   glBegin(GL_POINTS);
   	  glColor3f(1.,1.,1.);
   	  for (int i=0; i < pos.size(); i++) {
	  	glVertex2f(pos[i].x, pos[i].y);
	  }
   glEnd();

   glutSwapBuffers();
}
//----------------------------------------------------------------------
void idleFunc()
{
   glutPostRedisplay();
}
//----------------------------------------------------------------------
void reshapeFunc(int w, int h) 
{
  float dim = 300.;

  glViewport (0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-dim, dim, -dim, dim, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  boids->setDomainSize(dim);
}
//----------------------------------------------------------------------
int main(int argc, char** argv)
{
	// initialize GL graphics

   glutInit(&argc, argv);	// added by Myrna Merced

   glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowPosition(200, 0);
   glutInitWindowSize(512, 512);
   //glutInitWindowSize(gWindowWidth, gWindowHeight);
   glutCreateWindow("Gordon's Flocking");

   glutDisplayFunc(display);
   //glutKeyboardFunc(KeyboardFunc);
   //glutSpecialFunc(KeyboardSpecialFunc);
   glutReshapeFunc(reshapeFunc);
   glutIdleFunc(idleFunc);
  
   boids = initBoids();

   //glutSwapBuffers();
   glutMainLoop();

   return 0;
}
//----------------------------------------------------------------------