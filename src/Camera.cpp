#include "Camera.h"

void CameraController::initialize( Game* game, Scene* scene ) {
    _shift_key = false;
    _control_key = false;
    _rotate_active = true;
    _pan_active = false;
    _zoom_active = false;
    _touch_x = 0;
    _touch_y = 0;

    _invertY = false;
    _fpsCam = false;

    // let add a node to have the Camera looking at
    Node* rootNode = scene->addNode( "root" );
    rootNode->setTranslation( 0, 0, 0 );
    Node* cameraNode = NULL;
    if ( scene->getActiveCamera() == NULL ) // create camera if it doesn't exist
    {
        // Set the aspect ratio for the scene's camera to match the current resolution
        float w = (float)Game::getInstance()->getWidth();
        float h = (float)Game::getInstance()->getWidth();
        Camera* camera = Camera::createPerspective( 45, w / h, 1.0f, 10000.0f );
        cameraNode = scene->addNode( "camera" );
        cameraNode->setCamera( camera );

        cameraNode->setRotation( 0, 0, 0, 0 );
        cameraNode->setTranslation( Vector3::zero() );
        cameraNode->translateForward( -30 );

        // Make this the active camera of the scene.
        scene->setActiveCamera(camera);
        SAFE_RELEASE(camera);
    }
    else
        cameraNode = scene->getActiveCamera()->getNode();

    rootNode->addChild( cameraNode );

    Camera* cam = scene->getActiveCamera();

    _gimbelZ = scene->addNode("gz");
    _gimbelX = scene->addNode("gx");

    if (cam) {
        // Set the aspect ratio for the scene's camera to match the current
        // resolution
        cam->setAspectRatio(game->getAspectRatio());
        cam->getNode()->rotateY(MATH_DEG_TO_RAD(-90));
    }


    // Setup the camera control gimbels
    _freeCam = Camera::createPerspective( cam->getFieldOfView(), cam->getAspectRatio(), cam->getNearPlane(),
                                          cam->getFarPlane() );
    scene->addNode( "freeCam" )->setCamera( _freeCam );

    _freeCam->getNode()->setRotation( 0, 0, 0, 0 );
    _freeCam->getNode()->setTranslation( Vector3::zero() );
    _freeCam->getNode()->translateForward( -30 );

    _gimbelZ->addChild( _gimbelX );
    _gimbelX->addChild( _freeCam->getNode() );
    _sceneCam = cam;
}

void CameraController::finalize( Game* game ) {
    SAFE_RELEASE( _gimbelX );
    SAFE_RELEASE( _gimbelZ );
}

void CameraController::rotateCameraLocal( Scene* scene, float pitch, float yaw ) {
    Node* cameraNode = scene->getActiveCamera()->getNode();
    cameraNode->setRotation( Quaternion::identity() );
    cameraNode->rotateY( yaw );
    cameraNode->rotateX( pitch );
}

void CameraController::rotateCameraView( Scene* scene, float x, float y ) {
    _gimbelZ->rotateY( x );
    _gimbelX->rotateX( y );
}

void CameraController::zoomCameraView( Scene* scene, float x, float y ) {
    Node* cameraNode = scene->getActiveCamera()->getNode();
    cameraNode->translateForward( -y * cameraNode->getTranslation().distance( Vector3::zero() ) );
}

void CameraController::panCameraView( Scene* scene, float x, float y ) {
    Node* rootNode = scene->findNode( "root" );
    float location_x = rootNode->getTranslationX();
    float location_y = rootNode->getTranslationY();
    float inv_x = location_x - x;
    rootNode->setTranslationX( inv_x );
    rootNode->setTranslationY( location_y + y );
}

// Key Mapping for view rotation, pan and zoom
// ===========================================
//
// Ideally, to match Blender, we should have:
//     Rotation:  KEY_ALT                 (LMB + drag)
//     Zoom:      KEY_CTRL  + KEY_ALT     (LMB + drag)
//     PAN:       KEY_SHIFT + KEY_ALT     (LMB + drag)
//
// However, as KEY_ALT for Win8 event seems to not be working. So the mapping
// will be:
//     Rotation:                          (LMB + drag)
//     PAN:       KEY_SHIFT               (LMB + drag)
//     Zoom:      KEY_CTRL                (LMB + drag)

void CameraController::keyEvent( Game* game, Scene* scene, Keyboard::KeyEvent evt, int key ) {
    if ( evt == Keyboard::KEY_PRESS ) {
        switch ( key ) {
            case Keyboard::KEY_CTRL:
                _control_key = true;
                break;
            case Keyboard::KEY_SHIFT:
                _shift_key = true;
                break;
            default:
                break;
        }
    } else if ( evt == Keyboard::KEY_RELEASE ) {
        switch ( key ) {
            case Keyboard::KEY_CTRL:
                _control_key = false;
                break;
            case Keyboard::KEY_SHIFT:
                _shift_key = false;
                break;
            default:
                break;
        }
    }
    // update view rotation, pan and zoom flags
    /*   if (_control_key && _shift_key) {
      _rotate_active = false;
      _zoom_active = true;
      _pan_active = false;
   }
   else */
    if ( _control_key ) {
        _rotate_active = false;
        _zoom_active = true;
        _pan_active = false;
    } else if ( _shift_key ) {
        _rotate_active = false;
        _zoom_active = false;
        _pan_active = true;
    } else {
        _rotate_active = true;
        _zoom_active = false;
        _pan_active = false;
    }
}

void CameraController::touchEvent( Game* game, Scene* scene, Touch::TouchEvent evt, int x, int y,
                                   unsigned int contactIndex ) {
    switch ( evt ) {
        case Touch::TOUCH_PRESS:
            if ( contactIndex == 0 ) {
                _touch_x = x;
                _touch_y = y;
            }
            break;

        case Touch::TOUCH_MOVE:
            if ( contactIndex == 0 && scene->getActiveCamera() == _freeCam ) {
                int sx = 1;
                int sy = 1;
                int delta_x, delta_y;
                Node* cameraNode = scene->getActiveCamera()->getNode();

                delta_x = x - _touch_x;
                delta_y = y - _touch_y;
                _touch_x = x;
                _touch_y = y;
                if ( _zoom_active ) {
                    zoomCameraView( scene, -delta_x / 200.f, -delta_y / 200.f );
                }
                if ( _rotate_active ) {
                    if (_invertY)
                        rotateCameraView(scene, -delta_x / 500.f, -delta_y / 500.f);
                    else
                        rotateCameraView( scene, -delta_x / 500.f, delta_y / 500.f );
                }
                if ( _pan_active ) {
                    panCameraView( scene, -delta_x / 500.f, delta_y / 500.f );
                }
            }
            break;
    }
}

void CameraController::controlEvent( Game* game, Scene* scene, Control* control, Control::Listener::EventType evt ) {}

void CameraController::setGimbel( Scene* scene, bool locked ) {
    scene->setActiveCamera( locked ? _sceneCam : _freeCam );
}
