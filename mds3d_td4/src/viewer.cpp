#include "viewer.h"
#include "camera.h"

using namespace Eigen;

Viewer::Viewer()
  : _winWidth(0), _winHeight(0)
{
}

Viewer::~Viewer()
{
}

////////////////////////////////////////////////////////////////////////////////
// GL stuff

// initialize OpenGL context
void Viewer::init(int w, int h){
    loadShaders();

    // if(!_mesh.load(DATA_DIR"/models/quad.off")) exit(1);
    if(!_mesh.load(DATA_DIR"/models/sphere.obj")) exit(1);
    _mesh.initVBA();

    reshape(w,h);
    _trackball.setCamera(&_cam);
    _cam.setPerspective(M_PI/2, 0.1, 100000);
    _cam.lookAt(Vector3f(0, 2, 3), Vector3f(0, 0, 0), Vector3f(0, 1, 0));

    // ------ Earth ------ //
    earth.setIdentity();
    Affine3f t_earth = Translation3f(3, 0, 0) * AngleAxisf(23.44 * M_PI/180 , Vector3f::UnitZ()) * Scaling(Vector3f(0.35, 0.35, 0.35));
    earth = t_earth.matrix();

    // ------ Moon ------ //
    moon.setIdentity();
    Affine3f t_moon = Translation3f(3.5, 0, 0) * AngleAxisf(23.44 * M_PI/180 , Vector3f::UnitZ()) * Scaling(Vector3f(0.1, 0.1, 0.1));
    moon = t_moon.matrix();

    // ------ Earth - Moon ------ //
    Vector3f t_moon_earth = Affine3f(earth) * Vector3f(0, 0, 0);
    t_moon =  Translation3f(t_moon_earth) * AngleAxisf(5.14 * M_PI/180,  Vector3f::UnitX()) * Translation3f(-t_moon_earth) ;
    moon = t_moon.matrix() * moon;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
}

void Viewer::reshape(int w, int h){
    _winWidth = w;
    _winHeight = h;
    _cam.setViewport(w,h);
}


/*!
   callback to draw graphic primitives
 */
void Viewer::drawScene()
{
  _shader.activate();
  glViewport(0, 0, _winWidth, _winHeight);
  glClearColor(0.5, 0.5, 0.5, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);

  glUniform1i(_shader.getUniformLocation("wireframe"), 0);

  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  // ------ Sun ------ //
  sun.setIdentity();
  Affine3f t_sun = Translation3f(0,0,0) * Scaling(Vector3f(1.1, 1.1, 1.1));
  sun = sun * t_sun.matrix();

  glUniformMatrix4fv(_shader.getUniformLocation("obj_mat"), 1, GL_FALSE, sun.data());
  glUniformMatrix4fv(_shader.getUniformLocation("view_mat"), 1, GL_FALSE, _cam.viewMatrix().data());
  glUniformMatrix4fv(_shader.getUniformLocation("pspt_mat"), 1, GL_FALSE, _cam.projectionMatrix().data());

  glUniform3f(_shader.getUniformLocation("light_pos"), 10, 20, 1);
  Eigen::Matrix3f normal_sun;
  normal_sun = ((sun.topLeftCorner(3,3) * _cam.viewMatrix().topLeftCorner(3,3)).inverse()).transpose();
  glUniformMatrix3fv(_shader.getUniformLocation("normal_mat"), 1, GL_FALSE, normal_sun.data());
  _mesh.draw(_shader);


  // ------ Earth ------ //
  Vector3f t_earth = Affine3f(earth) * Vector3f(0,0,0);
  Affine3f a_earth = Translation3f(t_earth) * AngleAxisf(0.20, Vector3f::UnitY()) * Translation3f(-t_earth);  
  earth = a_earth.matrix() * earth;

  a_earth = AngleAxisf(0.01, Vector3f::UnitY());
  earth = a_earth.matrix() * earth;

  glUniformMatrix4fv(_shader.getUniformLocation("obj_mat"), 1, GL_FALSE, earth.data());
  glUniformMatrix4fv(_shader.getUniformLocation("view_mat"), 1, GL_FALSE, _cam.viewMatrix().data());
  glUniformMatrix4fv(_shader.getUniformLocation("pspt_mat"), 1, GL_FALSE, _cam.projectionMatrix().data());
  
  glUniform3f(_shader.getUniformLocation("light_pos"), 0, 0, 0);
  Eigen::Matrix3f normal_earth;
  normal_earth = ((_cam.viewMatrix().topLeftCorner(3,3) * earth.topLeftCorner(3,3)).inverse()).transpose();
  glUniformMatrix3fv(_shader.getUniformLocation("normal_mat"), 1, GL_FALSE, normal_earth.data());
  _mesh.draw(_shader);


  // ------ Moon ------ //
  Vector3f t_moon = Affine3f(moon) * Vector3f(0,0,0);
  Affine3f a_moon = Translation3f(t_moon) * AngleAxisf(0.20,Vector3f::UnitY()) * Translation3f(-t_moon);
  moon = a_moon.matrix() * moon;

  a_moon = AngleAxisf(0.01, Vector3f::UnitY());
  moon = a_moon.matrix() * moon;

  //Around Earth
  t_moon = Affine3f (earth) * Vector3f(0,0,0);
  a_moon = Translation3f(t_moon) * AngleAxisf(0.1,Vector3f::UnitY()) * Translation3f(-t_moon);
  moon = a_moon.matrix() * moon;

  glUniformMatrix4fv(_shader.getUniformLocation("obj_mat"), 1, GL_FALSE, moon.data());
  glUniformMatrix4fv(_shader.getUniformLocation("view_mat"), 1, GL_FALSE, _cam.viewMatrix().data());
  glUniformMatrix4fv(_shader.getUniformLocation("pspt_mat"), 1, GL_FALSE, _cam.projectionMatrix().data());
  
  glUniform3f(_shader.getUniformLocation("light_pos"), 0, 0, 0);
  Eigen::Matrix3f normal_moon;
  normal_moon = ((_cam.viewMatrix().topLeftCorner(3,3) * moon.topLeftCorner(3,3)).inverse()).transpose();
  glUniformMatrix3fv(_shader.getUniformLocation("normal_mat"), 1, GL_FALSE, normal_moon.data());
  _mesh.draw(_shader);


  //TD4 partie 2 - Lemming + cams
  // Matrix4f M;
  //   M <<  _zoom, 0, 0, _offset.x(),
  //         0, _zoom, 0, _offset.y(),
  //         0, 0,     1, 0,
  //         0, 0,     0, 1;
  // glUniformMatrix4fv(_shader.getUniformLocation("obj_mat"), 1, GL_FALSE, M.data());
  // glUniformMatrix4fv(_shader.getUniformLocation("pspt_mat"), 1, GL_FALSE, _cam.viewMatrix().data());
  // glUniformMatrix4fv(_shader.getUniformLocation("view_mat"), 1, GL_FALSE, _cam.projectionMatrix().data());

  // _mesh.draw(_shader);
  //-----------------------------------------------


  //TD4 partie 2 - Chaises
  //-----------------------------------------------
  // Matrix4f M;
  // M <<  0.5, 0, 0, -0.5,
  //         0, 0.5, 0, -1,
  //         0, 0, 1, 0,
  //         0, 0, 0, 1;
  // glUniformMatrix4fv(_shader.getUniformLocation("obj_mat"), 1, GL_FALSE, M.data());

  // _mesh.draw(_shader);

  //   M <<  -0.5, 0, 0, 0.5,
  //         0, 0.5, 0, -1,
  //         0, 0, 1, 0,
  //         0, 0, 0, 1;
  // glUniformMatrix4fv(_shader.getUniformLocation("obj_mat"), 1, GL_FALSE, M.data());

  // _mesh.draw(_shader);


  // Affine3f A;
  //           A = Translation3f(0, 0.5, 0)
  //               * AngleAxisf(_theta, Vector3f::UnitY())
  //               * Translation3f(0, -0.5, 0);
  // glUniformMatrix4fv(_shader.getUniformLocation("obj_mat"), 1, GL_FALSE, A.matrix().data());

  // _mesh.draw(_shader);
  //-----------------------------------------------

  if(_wireframe)
  {
    glUniform1i(_shader.getUniformLocation("wireframe"), 1);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glDepthFunc(GL_LEQUAL);
    _mesh.draw(_shader);
    
    glUniform1i(_shader.getUniformLocation("wireframe"), 0);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDepthFunc(GL_LESS);
    _mesh.draw(_shader);

    _shader2.activate();
    glUniformMatrix4fv(_shader2.getUniformLocation("obj_mat"), 1, GL_FALSE, sun.data());
    glUniformMatrix4fv(_shader2.getUniformLocation("view_mat"), 1, GL_FALSE, _cam.viewMatrix().data());
    glUniformMatrix4fv(_shader2.getUniformLocation("pspt_mat"), 1, GL_FALSE, _cam.projectionMatrix().data());
    _mesh.draw(_shader2);

    glUniformMatrix4fv(_shader2.getUniformLocation("obj_mat"), 1, GL_FALSE, earth.data());
    glUniformMatrix4fv(_shader2.getUniformLocation("view_mat"), 1, GL_FALSE, _cam.viewMatrix().data());
    glUniformMatrix4fv(_shader2.getUniformLocation("pspt_mat"), 1, GL_FALSE, _cam.projectionMatrix().data());
    _mesh.draw(_shader2);

    glUniformMatrix4fv(_shader2.getUniformLocation("obj_mat"), 1, GL_FALSE, moon.data());
    glUniformMatrix4fv(_shader2.getUniformLocation("view_mat"), 1, GL_FALSE, _cam.viewMatrix().data());
    glUniformMatrix4fv(_shader2.getUniformLocation("pspt_mat"), 1, GL_FALSE, _cam.projectionMatrix().data());
    _mesh.draw(_shader2);

    _shader2.deactivate();

  }

  _shader.deactivate();
}

void Viewer::updateAndDrawScene()
{
    drawScene();
}

void Viewer::loadShaders()
{
    // Here we can load as many shaders as we want, currently we have only one:
    _shader.loadFromFiles(DATA_DIR"/shaders/simple.vert", DATA_DIR"/shaders/simple.frag");
    _shader2.loadFromFiles(DATA_DIR"/shaders/simple.vert", DATA_DIR"/shaders/simple.frag");
    checkError();
}

////////////////////////////////////////////////////////////////////////////////
// Events

/*!
   callback to manage keyboard interactions
   You can change in this function the way the user
   interact with the application.
 */
void Viewer::keyPressed(int key, int action, int /*mods*/)
{
  if(key == GLFW_KEY_R && action == GLFW_PRESS)
  {
    loadShaders();
  }

  if(action == GLFW_PRESS || action == GLFW_REPEAT )
  {
    if (key==GLFW_KEY_UP)
    {
      _offset.y() += 0.05f;
    }
    else if (key==GLFW_KEY_DOWN)
    {
      _offset.y() -= 0.05f;
    }
    else if (key==GLFW_KEY_LEFT)
    {
      _offset.x() -= 0.05f;
    }
    else if (key==GLFW_KEY_RIGHT)
    {
      _offset.x() += 0.05f;
    }
    else if (key==GLFW_KEY_PAGE_UP)
    {
      _zoom *= 1.1f;
    }
    else if (key==GLFW_KEY_PAGE_DOWN)
    {
      _zoom /= 1.1f;
    }
    else if (key==GLFW_KEY_E)
    {
      _wireframe = !_wireframe;
    }
    else if (key==GLFW_KEY_T)
    {
      _theta += 0.1;
    }
    else if (key==GLFW_KEY_Y)
    {
      _theta -= 0.1;
    }

  }
}

/*!
   callback to manage mouse : called when user press or release mouse button
   You can change in this function the way the user
   interact with the application.
 */
void Viewer::mousePressed(GLFWwindow */*window*/, int /*button*/, int action)
{
  if(action == GLFW_PRESS)
  {
      _trackingMode = TM_ROTATE_AROUND;
      _trackball.start();
      _trackball.track(_lastMousePos);
  }
  else if(action == GLFW_RELEASE)
  {
      _trackingMode = TM_NO_TRACK;
  }
}


/*!
   callback to manage mouse : called when user move mouse with button pressed
   You can change in this function the way the user
   interact with the application.
 */
void Viewer::mouseMoved(int x, int y)
{
    if(_trackingMode == TM_ROTATE_AROUND)
    {
        _trackball.track(Vector2i(x,y));
    }

    _lastMousePos = Vector2i(x,y);
}

void Viewer::mouseScroll(double /*x*/, double y)
{
  _cam.zoom(-0.1f*y);
}

void Viewer::charPressed(int /*key*/)
{
}
