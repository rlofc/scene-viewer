#ifndef TEMPLATEGAME_H_
#define TEMPLATEGAME_H_

#include "gameplay.h"
using namespace gameplay;

#include "Grid.h"
#include "Camera.h"
#include "Forms.h"

class SceneViewer : public Game, Control::Listener, SidePanel::Delegate {
public:
    SceneViewer();

    void keyEvent( Keyboard::KeyEvent evt, int key );
    void touchEvent( Touch::TouchEvent evt, int x, int y, unsigned int contactIndex );
    void controlEvent( Control* control, EventType evt );
    bool checkScene( Node* node, Ray* ray );

public: // SidePanel::Delegate
    virtual void playSelectedNode();
    virtual void playAllNodes();
    virtual void setGrid( bool draw );
    virtual void setGimbel( bool locked );
    virtual void setLightVector( const Vector3 & lightVector );
    virtual void setClearColor( const Vector4 & clearColor );

protected:
    void initialize();
    void finalize();
    void update( float elapsedTime );
    void render( float elapsedTime );

private:
    bool setLights( Node* node );
    bool drawScene( Node* node );
    bool startAnims( Node* node );

    Scene*           _scene;
    Grid*            _grid;
    Vector4          _clearColor;
    Vector3          _directionalLightVector;
    Node*            _selected;
    CameraController _camera;
    SidePanel        _sidePanel;

    bool _isGridOn;
    float _lastDistance;

    void lookAt( Node* node, const Vector3& eye, const Vector3& target, const Vector3& up );
    void playNodeAmination( Node* node );
};

#endif
