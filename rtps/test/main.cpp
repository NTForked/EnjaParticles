#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

//#include <utils.h>
//#include <string.h>
//#include <string>
#include <sstream>
#include <iomanip>

#include <GL/glew.h>
#if defined __APPLE__ || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
//OpenCL stuff
#endif

#include <RTPS.h>
//#include "timege.h"
using namespace rtps;

int window_width = 1200;
int window_height = 600;
float fov = 65.;
int glutWindowHandle = 0;
/*
float translate_x = -.5f;
float translate_y = 0.f;//-200.0f;//300.f;
float translate_z = 1.5f;//200.f;
*/
float translate_x = -2.00f;
float translate_y = -3.00f;//300.f;
float translate_z = 3.00f;

// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
std::vector<Triangle> triangles;
//std::vector<Box> boxes;

// offsets into the triangle list. tri_offsets[i] corresponds to the 
// triangle list for box[i]. Number of triangles for triangles[i] is
//    tri_offsets[i+1]-tri_offsets[i]
// Add one more offset so that the number of triangles in 
//   boxes[boxes.size()-1] is tri_offsets[boxes.size()]-tri_offsets[boxes.size()-1]
//std::vector<int> tri_offsets;


void init_gl();

void appKeyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void appRender();
void appDestroy();

void appMouse(int button, int state, int x, int y);
void appMotion(int x, int y);
void resizeWindow(int w, int h);

void timerCB(int ms);

void drawString(const char *str, int x, int y, float color[4], void *font);
void showFPS(float fps, std::string *report);
void *font = GLUT_BITMAP_8_BY_13;

rtps::RTPS* ps;

//#define NUM_PARTICLES 524288
//#define NUM_PARTICLES 262144
//#define NUM_PARTICLES 65536
//#define NUM_PARTICLES 16384
//#define NUM_PARTICLES 10000
#define NUM_PARTICLES 8192
//#define NUM_PARTICLES 4096
//#define NUM_PARTICLES 2048
//#define NUM_PARTICLES 1024
//#define NUM_PARTICLES 256
#define DT .001f





//timers
//GE::Time *ts[3];

//================
//#include "materials_lights.h"

//----------------------------------------------------------------------
float rand_float(float mn, float mx)
{
    float r = rand() / (float) RAND_MAX;
    return mn + (mx-mn)*r;
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
int main(int argc, char** argv)
{

    //initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - window_width/2, 
                            glutGet(GLUT_SCREEN_HEIGHT)/2 - window_height/2);


    std::stringstream ss;
    ss << "Real-Time Particle System: " << NUM_PARTICLES << std::ends;
    glutWindowHandle = glutCreateWindow(ss.str().c_str());

    glutDisplayFunc(appRender); //main rendering function
    glutTimerFunc(30, timerCB, 30); //determin a minimum time between frames
    glutKeyboardFunc(appKeyboard);
    glutMouseFunc(appMouse);
    glutMotionFunc(appMotion);
    glutReshapeFunc(resizeWindow);

    //define_lights_and_materials();

    // initialize necessary OpenGL extensions
    glewInit();
    GLboolean bGLEW = glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"); 
    printf("GLEW supported?: %d\n", bGLEW);


    printf("before we call enjas functions\n");


    //default constructor
    //rtps::RTPSettings settings;
    //rtps::Domain grid = Domain(float4(-5,-.3,0,0), float4(2, 2, 12, 0));
    rtps::Domain grid = Domain(float4(0,0,0,0), float4(5, 5, 5, 0));
    //rtps::Domain grid = Domain(float4(0,0,0,0), float4(2, 2, 2, 0));
    rtps::RTPSettings settings(rtps::RTPSettings::SPH, NUM_PARTICLES, DT, grid);

    settings.setRadiusScale(1.0);
    //settings.setRenderType(RTPSettings::SCREEN_SPACE_RENDER);
    settings.setRenderType(RTPSettings::RENDER);
    //settings.setRenderType(RTPSettings::SPRITE_RENDER);
    //settings.setRenderType((RTPSettings::RenderType)1);
    settings.setBlurScale(1.0);
    settings.setUseGLSL(1);
    settings.setUseAlphaBlending(1);    

    ps = new rtps::RTPS(settings);


    printf("about to make hose\n");
    float4 center(2., 2., 2., 1.);
    float4 velocity(.6, -.6, -.6, 0);

    //sph sets spacing and multiplies by radius value
    ps->system->addHose(2048, center, velocity, 5);




    //initialize the OpenGL scene for rendering
    init_gl();

    glutMainLoop();
    return 0;
}



void init_gl()
{
    // default initialization
    //glDisable(GL_DEPTH_TEST);

    // viewport
    glViewport(0, 0, window_width, window_height);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(60.0, (GLfloat)window_width / (GLfloat) window_height, 0.1, 100.0);
    gluPerspective(fov, (GLfloat)window_width / (GLfloat) window_height, 0.3, 100.0);
    //gluPerspective(90.0, (GLfloat)window_width / (GLfloat) window_height, 0.1, 10000.0); //for lorentz

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-90, 1.0, 0.0, 0.0);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
    glTranslatef(translate_x, translate_z, translate_y);
    ps->system->getRenderer()->setWindowDimensions(window_width,window_height);
    //glTranslatef(0, 10, 0);
    /*
    gluLookAt(  0,10,0,
                0,0,0,
                0,0,1);
    */


    //glTranslatef(0, translate_z, translate_y);
    //glRotatef(-90, 1.0, 0.0, 0.0);

    return;

}

void appKeyboard(unsigned char key, int x, int y)
{
    int nn;
    float4 min;
    float4 max;
    switch (key)
    {
        case 'e': //dam break
            nn = 16384;
            min = float4(.1, .1, .1, 1.0f);
            max = float4(3.9, 3.9, 3.9, 1.0f);
            ps->system->addBox(nn, min, max, false);
            return;
        case 'p': //print timers
            ps->system->printTimers();
            return;
        case '\033': // escape quits
        case '\015': // Enter quits    
        case 'Q':    // Q quits
        case 'q':    // q (or escape) quits
            // Cleanup up and quit
            appDestroy();
            return;
        case 'm':
            //spray hose
            printf("about to spray\n");
            ps->system->sprayHoses();
            return;


        case 't': //place a cube for collision
            {
                nn = 512;
                float cw = .25;
                float4 cen = float4(cw, cw, cw-.1, 1.0f);
                make_cube(triangles, cen, cw);
                cen = float4(1+cw, 1+cw, cw-.1, 1.0f);
                make_cube(triangles, cen, cw);
                cen = float4(1+3*cw, 1+3*cw, cw-.1, 1.0f);
                make_cube(triangles, cen, cw);
                cen = float4(3.5, 3.5, cw-.1, 1.0f);
                make_cube(triangles, cen, cw);
                ps->system->loadTriangles(triangles);
                return;
            }
        case 'r': //drop a rectangle
            {
                nn = 2048;
                min = float4(.2, .2, .2, 1.0f);
                max = float4(2., 2., 2., 1.0f);
                ps->system->addBox(nn, min, max, false);
                return;
            }
        case 'o':
            ps->system->getRenderer()->writeBuffersToDisk();
            return;
        case 'c':
            ps->system->getRenderer()->setDepthSmoothing(Render::NO_SHADER);
            return;
        case 'C':
            ps->system->getRenderer()->setDepthSmoothing(Render::BILATERAL_GAUSSIAN_SHADER);
            return;
        case 'w':
            translate_z -= 0.1;
            break;
        case 'a':
            translate_x += 0.1;
            break;
        case 's':
            translate_z += 0.1;
            break;
        case 'd':
            translate_x -= 0.1;
            break;
        case 'z':
            translate_y += 0.1;
            break;
        case 'x':
            translate_y -= 0.1;
            break;
        default:
            return;
    }

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-90, 1.0, 0.0, 0.0);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
    glTranslatef(translate_x, translate_z, translate_y);
}

void appRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ps->update();

    glEnable(GL_DEPTH_TEST);

    ps->render();
    glColor4f(0,0,1,.5);

    //glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLES);
    //printf("num triangles %zd\n", triangles.size());
    for (int i=0; i < triangles.size(); i++)
    {
        //for (int i=0; i < 20; i++) {
        Triangle& tria = triangles[i];
        glNormal3fv(&tria.normal.x);
        glVertex3f(tria.verts[0].x, tria.verts[0].y, tria.verts[0].z);
        glVertex3f(tria.verts[1].x, tria.verts[1].y, tria.verts[1].z);
        glVertex3f(tria.verts[2].x, tria.verts[2].y, tria.verts[2].z);
    }
    glEnd();

    glDisable(GL_BLEND);
    //showFPS(enjas->getFPS(), enjas->getReport());
    glutSwapBuffers();

    //glDisable(GL_DEPTH_TEST);
}

void appDestroy()
{

    delete ps;


    if (glutWindowHandle)glutDestroyWindow(glutWindowHandle);
    printf("about to exit!\n");

    exit(0);
}

void timerCB(int ms)
{
    glutTimerFunc(ms, timerCB, ms);
    glutPostRedisplay();
}


void appMouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        mouse_buttons |= 1<<button;
    }
    else if (state == GLUT_UP)
    {
        mouse_buttons = 0;
    }

    mouse_old_x = x;
    mouse_old_y = y;

    //glutPostRedisplay();
}

void appMotion(int x, int y)
{
    float dx, dy;
    dx = x - mouse_old_x;
    dy = y - mouse_old_y;

    if (mouse_buttons & 1)
    {
        rotate_x += dy * 0.2;
        rotate_y += dx * 0.2;
    }
    else if (mouse_buttons & 4)
    {
        translate_z -= dy * 0.1;
    }

    mouse_old_x = x;
    mouse_old_y = y;

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glTranslatef(-translate_x, -translate_y, -translate_z);
    glRotatef(-90, 1.0, 0.0, 0.0);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
    glTranslatef(translate_x, translate_z, translate_y);
    //glTranslatef(0, translate_z, translate_y);
    //glutPostRedisplay();
}


///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while (*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_LIGHTING);
    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
// display frame rates
///////////////////////////////////////////////////////////////////////////////
void showFPS(float fps, std::string* report)
{
    static std::stringstream ss;

    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, 400, 0, 300);         // set to orthogonal projection

    float color[4] = {1, 1, 0, 1};

    // update fps every second
    ss.str("");
    ss << std::fixed << std::setprecision(1);
    ss << fps << " FPS" << std::ends; // update fps string
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
    drawString(ss.str().c_str(), 15, 286, color, font);
    drawString(report[0].c_str(), 15, 273, color, font);
    drawString(report[1].c_str(), 15, 260, color, font);

    // restore projection matrix
    glPopMatrix();                      // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);         // switch to modelview matrix
    glPopMatrix();                      // restore to previous modelview matrix
}
//----------------------------------------------------------------------
void resizeWindow(int w, int h)
{
    if (h==0)
    {
        h=1;
    }
    glViewport(0, 0, w, h);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, (GLfloat)w / (GLfloat) h, 0.3, 100.0);

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-90, 1.0, 0.0, 0.0);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
    glTranslatef(translate_x, translate_z, translate_y);
    ps->system->getRenderer()->setWindowDimensions(w,h);
    window_width = w;
    window_height = h;
    glutPostRedisplay();
}
