#include "Forms.h"
#include "Utils.h"

SidePanel::SidePanel()
    : _form( NULL ),
      _sliderLightVectorX( NULL ),
      _sliderLightVectorY( NULL ),
      _sliderLightVectorZ( NULL ),
      _sliderClearColorRed( NULL ),
      _sliderClearColorGreen( NULL ),
      _sliderClearColorBlue( NULL ),
      _bPlayAll( NULL ),
      _bPlaySelected( NULL ) {}

void SidePanel::initialize( Control::Listener *game, Scene *scene ) {
    // Setup the form and callbacks
    _form = Form::create( "res/editor.form" );
    _sliderLightVectorX = (Slider *)_form->getControl( "lvx" );
    _sliderLightVectorY = (Slider *)_form->getControl( "lvy" );
    _sliderLightVectorZ = (Slider *)_form->getControl( "lvz" );
    _sliderClearColorRed = (Slider *)_form->getControl( "ccRed" );
    _sliderClearColorGreen = (Slider *)_form->getControl( "ccGreen" );
    _sliderClearColorBlue = (Slider *)_form->getControl( "ccBlue" );
    _bPlayAll = (Button *)_form->getControl( "playAll" );
    _bPlaySelected = (Button *)_form->getControl( "playSelected" );
    _bPlaySelected->setEnabled( false );

    _gridCheckBox = static_cast<CheckBox *>( _form->getControl( "gridCheckBox" ) );
    _cameraGimbelCheckBox = static_cast<CheckBox *>( _form->getControl( "cameraGimbelCheckBox" ) );
    _cameraInvertY = static_cast<CheckBox *>(_form->getControl("cameraInvertY"));
    _cameraFPS = static_cast<CheckBox *>(_form->getControl("cameraFPS"));

    Slider *sliders[] = {_sliderLightVectorX,  _sliderLightVectorY,    _sliderLightVectorZ,
                         _sliderClearColorRed, _sliderClearColorGreen, _sliderClearColorBlue},
           *s;
    FOREACH( s, sliders ) { s->addListener( game, Control::Listener::VALUE_CHANGED ); }

    _bPlayAll->addListener( game, Control::Listener::CLICK );
    _bPlaySelected->addListener( game, Control::Listener::CLICK );

    _gridCheckBox->addListener( game, Control::Listener::VALUE_CHANGED );
    _gridCheckBox->setEnabled( true );

    _cameraGimbelCheckBox->addListener( game, Control::Listener::VALUE_CHANGED );
    _cameraGimbelCheckBox->setEnabled( true );
    _cameraGimbelCheckBox->setChecked( true );

    _cameraInvertY->addListener(game, Control::Listener::VALUE_CHANGED);
    _cameraInvertY->setEnabled(true);
    _cameraInvertY->setChecked(false);

    // TODO
    _cameraFPS->addListener(game, Control::Listener::VALUE_CHANGED);
    _cameraFPS->setEnabled(true);
    _cameraFPS->setChecked(false);
    _cameraFPS->setVisible(false); // hide it for now

}

void SidePanel::finalize( Game *game ) { SAFE_RELEASE( _form ); }

void SidePanel::update( Game *game, float elapsedTime ) {}

void SidePanel::render( Game *game, float elapsedTime ) { _form->draw(); }

void SidePanel::controlEvent( SidePanel::Delegate *delegate, Control *control, Control::Listener::EventType evt ) {
    switch ( evt ) {
        case Control::Listener::CLICK:
            if ( control == _bPlayAll ) {
                delegate->playAllNodes();
            }
            if ( control == _bPlaySelected ) {
                delegate->playSelectedNode();
            }
            break;
        case Control::Listener::VALUE_CHANGED:
            if ( control == _sliderLightVectorX || control == _sliderLightVectorY || control == _sliderLightVectorZ ) {
                Vector3 lightVector( _sliderLightVectorX->getValue(), _sliderLightVectorY->getValue(),
                                     _sliderLightVectorZ->getValue() );
                delegate->setLightVector( lightVector );
            }
            if ( control == _sliderClearColorRed || control == _sliderClearColorGreen ||
                 control == _sliderClearColorBlue ) {
                Vector4 clearColor( _sliderClearColorRed->getValue(), _sliderClearColorGreen->getValue(),
                                    _sliderClearColorBlue->getValue(), 1.0f );
                delegate->setClearColor( clearColor );
            }
            if ( control == _gridCheckBox ) {
                delegate->setGrid( _gridCheckBox->isChecked() );
            }
            if ( control == _cameraGimbelCheckBox ) {
                delegate->setGimbel( _cameraGimbelCheckBox->isChecked() );
            }
            if (control == _cameraInvertY) {
                delegate->setInvertY(_cameraInvertY->isChecked());
            }
            if (control == _cameraFPS) {
                delegate->setFpsCam(_cameraFPS->isChecked());
            }
            break;
    }
}

void SidePanel::setSelectedNode( gameplay::Node *node, bool enable ) {
    if (enable)
        _bPlaySelected->setText(node->getId());
    else
        _bPlaySelected->setText("Play selected");

    _bPlaySelected->setEnabled(enable);
}
