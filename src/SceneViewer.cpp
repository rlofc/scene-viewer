#include "SceneViewer.h"
#include "Utils.h"

#include <stdlib.h>

// Declare our game instance
SceneViewer game;

SceneViewer::SceneViewer() : _scene( NULL ), _selected( NULL ), _isGridOn( false ) {}

void SceneViewer::initialize() {
    int argc;
    char **argv;
    this->getArguments( &argc, &argv );
    if ( argc < 2 ) {
        _scene = Scene::load( "res/scene.gpb" );
        if ( !_scene ) {
            LOG( "Could not load %s", "res/scene.gpb" );
            exit();
        }
    } else {
        std::string path = std::string( "res/" ) + std::string( argv[1] ) + std::string( ".gpb" );
        puts( path.c_str() );
        _scene = Scene::load( path.c_str() );
        if ( !_scene ) {
            LOG( "Could not load %s", path.c_str() );
            exit();
        }
    }

    _camera.initialize( this, _scene );
    _sidePanel.initialize( this, _scene );

    // Try to use any significant light source for the scene light vector.
    // This is just a temporary hack until I manage to tap into the
    // material file from the python addon.
    Node *light = _scene->findNode( "Spot" )
                      ? _scene->findNode( "Spot" )
                      : _scene->findNode( "Area" ) ? _scene->findNode( "Area" )
                                                   : _scene->findNode( "Sun" ) ? _scene->findNode( "Sun" ) : 0;
    if ( light ) _directionalLightVector = light->getForwardVector();

    // Set the metarial lighting
    _scene->visit( this, &SceneViewer::setLights );

    _clearColor = Vector4::zero();

    // Create the grid and add it to the scene.
    _grid = Grid::create();
    _scene->addNode( *_grid );
}

void SceneViewer::finalize() {
    SAFE_RELEASE( _scene );
    SAFE_DELETE( _grid );
    _camera.finalize( this );
    _sidePanel.finalize( this );
}

bool SceneViewer::setLights( Node *node ) {
    if ( node->getModel() ) {
        Material *m = node->getModel()->getMaterial( 0 );
        if ( m != NULL ) {
            m->getTechnique()->getParameter( "u_directionalLightColor[0]" )->setValue( Vector3( 1, 1, 1 ) );
            m->getTechnique()->getParameter( "u_directionalLightDirection[0]" )->setValue( _directionalLightVector );
        }
    }
    return true;
}

void SceneViewer::update( float elapsedTime ) {}

void SceneViewer::render( float elapsedTime ) {
    // Clear the color and depth buffers
    clear( CLEAR_COLOR_DEPTH, _clearColor, 1.0f, 0 );

    // Visit all the nodes in the scene for drawing
    _scene->visit( this, &SceneViewer::drawScene );
    _sidePanel.render( this, elapsedTime );
}

void SceneViewer::playNodeAmination( Node *node ) {
    Animation *anim = node->getAnimation( "animations" );
    if ( anim == NULL ) anim = node->getAnimation();
    if ( anim ) {
        if ( anim->getDuration() > 1 ) anim->play();
    }
}

bool SceneViewer::startAnims( Node *node ) {
    if ( node->getModel() ) {
        this->playNodeAmination( node );
    }
    return false;
}

bool SceneViewer::drawScene( Node *node ) {
    // If the node visited contains a model, draw it
    Model *model = node->getModel();
    if ( model ) {
        if ( strcmp( node->getId(), "grid" ) == 0 ) {
            if ( _isGridOn ) model->draw();
        } else
            model->draw();
    }
    return true;
}

void SceneViewer::keyEvent( Keyboard::KeyEvent evt, int key ) {
    if ( evt == Keyboard::KEY_PRESS ) {
        switch ( key ) {
            case Keyboard::KEY_ESCAPE:
                exit();
                break;
            default:
                break;
        }
    };
    _camera.keyEvent( this, _scene, evt, key );
}

void SceneViewer::controlEvent( Control *control, EventType evt ) {
    _sidePanel.controlEvent( this, control, evt );
    _camera.controlEvent( this, _scene, control, evt );
}

bool SceneViewer::checkScene( Node *node, Ray *ray ) {
    Model *model = node->getModel();
    if ( model ) {
        BoundingBox worldSpaceBoundingBox( model->getMesh()->getBoundingBox() );
        worldSpaceBoundingBox.transform( node->getWorldMatrix() );

        float distance = FLT_MAX;
        if ( ( distance = ray->intersects( worldSpaceBoundingBox ) ) != Ray::INTERSECTS_NONE ) {
            _selected = node;
            _sidePanel.setSelectedNode( node );
            return true;
        }
    }
    return false;
}

void SceneViewer::touchEvent( Touch::TouchEvent evt, int x, int y, unsigned int contactIndex ) {
    switch ( evt ) {
        case Touch::TOUCH_RELEASE: {
            Ray ray;
            Camera *camera = _scene->getActiveCamera();
            if ( camera ) {
                camera->pickRay( getViewport(), x, y, &ray );
                _scene->visit( this, &SceneViewer::checkScene, &ray );
            }
        } break;
    }
    _camera.touchEvent( this, _scene, evt, x, y, contactIndex );
}

void SceneViewer::lookAt( Node *node, const Vector3 &eye, const Vector3 &target, const Vector3 &up ) {
    // Create lookAt matrix
    Matrix matrix;
    Matrix::createLookAt( eye, target, up, &matrix );
    matrix.transpose();

    // Pull rotation out of matrix
    Quaternion rotation;
    matrix.getRotation( &rotation );

    // Set rotation on node
    node->setRotation( rotation );
}

void SceneViewer::playSelectedNode() { this->playNodeAmination( _selected ); }

void SceneViewer::playAllNodes() { _scene->visit( this, &SceneViewer::startAnims ); }

void SceneViewer::setGrid( bool draw ) { _isGridOn = draw; }

void SceneViewer::setGimbel( bool locked ) { _camera.setGimbel( _scene, locked ); }

void SceneViewer::setLightVector( const Vector3 &lightVector ) {
    _directionalLightVector = lightVector;
    _scene->visit( this, &SceneViewer::setLights );
}

void SceneViewer::setClearColor( const Vector4 &clearColor ) { _clearColor = clearColor; }
