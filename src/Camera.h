#ifndef CAMERA_H_M8TIKJNN
#define CAMERA_H_M8TIKJNN

#include "gameplay.h"
using namespace gameplay;

class CameraController {
public:
    virtual void initialize( Game* game, Scene* scene );
    virtual void finalize( Game* game );

    void keyEvent( Game* game, Scene* scene, Keyboard::KeyEvent evt, int key );
    void touchEvent( Game* game, Scene* scene, Touch::TouchEvent evt, int x, int y, unsigned int contactIndex );
    void controlEvent( Game* game, Scene* scene, Control* control, Control::Listener::EventType evt );

    void update( float elapsedTime );
    void render( float elapsedTime );

    void setGimbel( Scene* scene, bool locked );
    void setInvertY(bool inv) { _invertY = inv; }
    void setFpsCam(bool fps) { _fpsCam = fps; }

private:
    Camera* _sceneCam;
    Camera* _freeCam;

    // camera gimbels
    Node* _gimbelX;
    Node* _gimbelZ;

    bool _invertY, _fpsCam;

    // View control
    void rotateCameraLocal( Scene* scene, float x, float y );
    void rotateCameraView( Scene* scene, float x, float y );
    void zoomCameraView( Scene* scene, float x, float y );
    void panCameraView( Scene* scene, float x, float y );

    bool _shift_key;
    bool _control_key;
    bool _rotate_active;
    bool _pan_active;
    bool _zoom_active;
    int _touch_x;
    int _touch_y;
};

#endif /* end of include guard: CAMERA_H_M8TIKJNN */

