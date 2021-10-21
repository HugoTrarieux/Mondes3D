#ifndef VIEWER_H
#define VIEWER_H

#include "opengl.h"
#include "shader.h"
#include "camera.h"
#include "trackball.h"
#include "mesh.h"

#include <iostream>

class Hit;

class Viewer{
public:
    //! Constructor
    Viewer();
    virtual ~Viewer();

    // gl stuff
    void init(int w, int h);
    void drawScene();
    void updateAndDrawScene();
    void reshape(int w, int h);
    void loadShaders();

    // events
    void mousePressed(GLFWwindow* window, int button, int action, int mods);
    void mouseMoved(int x, int y);
    void mouseScroll(double x, double y);
    void keyPressed(int key, int action, int mods);
    void charPressed(int key);

private:

    bool pickAt(const Eigen::Vector2f &p, Hit &hit) const;
    void setObjectMatrix(const Eigen::Matrix4f &M) const;
    void drawArticulatedArm();

    int _winWidth, _winHeight;

    Camera _cam;
    Shader _shader;
    Shader _shaderCyl;
    Mesh   _meshCyl;
    Mesh   _scene;
    Mesh   _sphere;
    Mesh   _jointMesh;
    Mesh   _segmentMesh;
    float  _theta;
    int    _texid;

    Eigen::Matrix2Xf _jointAngles;
    Eigen::VectorXf _lengths;

    Eigen::Vector3f _IK_target;

    bool _wireframe;


    // Mouse parameters for the trackball
    enum TrackMode
    {
      TM_NO_TRACK=0, TM_ROTATE_AROUND, TM_ZOOM,
      TM_LOCAL_ROTATE, TM_FLY_Z, TM_FLY_PAN
    };
    TrackMode _trackingMode = TM_NO_TRACK;
    Trackball _trackball;
    Eigen::Vector2i _lastMousePos;
};

#endif
