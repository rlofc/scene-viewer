#ifndef TEMPLATEGAME_H_
#define TEMPLATEGAME_H_

#include "gameplay.h"

using namespace gameplay;

/**
 * Main game class.
 */
class SceneViewer: public Game,Control::Listener
{
public:

   /**
     * Constructor.
     */
   SceneViewer();

   /**
     * @see Game::keyEvent
     */
   void keyEvent(Keyboard::KeyEvent evt, int key);

   /**
     * @see Game::touchEvent
     */
   void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

   void controlEvent(Control *control, EventType evt);

   bool checkScene(Node* node, Ray* ray);

protected:

   /**
     * @see Game::initialize
     */
   void initialize();

   /**
     * @see Game::finalize
     */
   void finalize();

   /**
     * @see Game::update
     */
   void update(float elapsedTime);

   /**
     * @see Game::render
     */
   void render(float elapsedTime);

private:

   /**
     * Draws the scene each frame.
     */
   bool setLights(Node* node);
   bool drawScene(Node* node);
   bool startAnims(Node* node);

   /**
    * Viewer scene file
    */
   Scene* _scene;

   /**
    * User-defined clear color
    */
   Vector4 _clearColor;

   /**
    * User-defined light color
    */
   Vector3 _directionalLightVector;

   /**
    * Currently selected node (or NULL)
    */
   Node* _selected;

   /**
    * Sidebar Form
    */
   Form* _form;

   /**
    * Form References
    */
   Slider* _sliderLightVectorX;
   Slider* _sliderLightVectorY;
   Slider* _sliderLightVectorZ;
   Slider* _sliderClearColorRed;
   Slider* _sliderClearColorGreen;
   Slider* _sliderClearColorBlue;
   Slider* _acr;
   Slider* _acg;
   Slider* _acb;
   Button* _bPlayAll;
   Button* _bPlaySelected;

   /**
    * @brief Plays animations for the specified node
    * @param node
    */
   void playNodeAmination(Node *node);
};

#endif
