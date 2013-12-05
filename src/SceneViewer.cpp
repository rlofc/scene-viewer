#include "SceneViewer.h"
#define FOREACH(I,ITEMS) I=ITEMS[0]; for (int i = 0 ; i < sizeof(ITEMS)/sizeof(ITEMS[0]) ; i++,I=ITEMS[i])

// Declare our game instance
SceneViewer game;

SceneViewer::SceneViewer()
   : _scene(NULL),_form(NULL),_selected(NULL),
     _sliderLightVectorX(NULL),_sliderLightVectorY(NULL),_sliderLightVectorZ(NULL),
     _sliderClearColorRed(NULL),_sliderClearColorGreen(NULL),_sliderClearColorBlue(NULL),
     _bPlayAll(NULL),_bPlaySelected(NULL)
{
}

void SceneViewer::initialize()
{
   // Load game scene from file
   _scene = Scene::load("res/scene.gpb");

   Camera * cam = _scene->getActiveCamera();
   if (cam) {
      // Set the aspect ratio for the scene's camera to match the current resolution
      cam->setAspectRatio(getAspectRatio());
      cam->getNode()->rotateY(MATH_DEG_TO_RAD(-90));
   }

   // Try to use any significant light source for the scene light vector.
   // This is just a temporary hack until I manage to tap into the
   // material file from the python addon.
   Node * light = _scene->findNode("Spot") ? _scene->findNode("Spot") :
                                             _scene->findNode("Area") ? _scene->findNode("Area") :
                                                                        _scene->findNode("Sun") ? _scene->findNode("Sun") : 0;
   if (light) _directionalLightVector = light->getForwardVector();

   // Setup the form and callbacks
   _form = Form::create("res/editor.form");
   _sliderLightVectorX = (Slider *)_form->getControl("lvx");
   _sliderLightVectorY = (Slider *)_form->getControl("lvy");
   _sliderLightVectorZ = (Slider *)_form->getControl("lvz");
   _sliderClearColorRed = (Slider *)_form->getControl("ccRed");
   _sliderClearColorGreen = (Slider *)_form->getControl("ccGreen");
   _sliderClearColorBlue = (Slider *)_form->getControl("ccBlue");
   _bPlayAll = (Button *)_form->getControl("playAll");
   _bPlaySelected = (Button *)_form->getControl("playSelected");
   _bPlaySelected->setEnabled(false);

   Slider * sliders[] = { _sliderLightVectorX, _sliderLightVectorY, _sliderLightVectorZ,
                        _sliderClearColorRed, _sliderClearColorGreen, _sliderClearColorBlue }, *s;
   FOREACH(s, sliders) {
      s->addListener(this, Listener::VALUE_CHANGED);
   }

   _bPlayAll->addListener(this, Listener::CLICK);
   _bPlaySelected->addListener(this, Listener::CLICK);

   // Set the metarial lighting
   _scene->visit(this, &SceneViewer::setLights);

   _clearColor = Vector4::zero();
}

void SceneViewer::finalize()
{
   SAFE_RELEASE(_form);
   SAFE_RELEASE(_scene);
}

bool SceneViewer::setLights(Node* node)
{
   if (node->getModel()) {
      Material * m = node->getModel()->getMaterial(0);
      m->getTechnique()->getParameter("u_directionalLightColor[0]")->setValue(Vector3(1,1,1));
      m->getTechnique()->getParameter("u_directionalLightDirection[0]")->setValue(_directionalLightVector);
   }
   return true;
}

void SceneViewer::update(float elapsedTime)
{
}

void SceneViewer::render(float elapsedTime)
{
   // Clear the color and depth buffers
   clear(CLEAR_COLOR_DEPTH, _clearColor, 1.0f, 0);

   // Visit all the nodes in the scene for drawing
   _scene->visit(this, &SceneViewer::drawScene);
   _form->draw();
}

void SceneViewer::playNodeAmination(Node * node)
{
   Animation * anim = node->getAnimation("animations");
   if (anim==NULL) anim = node->getAnimation();
   anim->getDuration();
   if (anim) {
      if (anim->getDuration()>1)
         anim->play();
   }
}

bool SceneViewer::startAnims(Node* node)
{
   if (node->getModel()) {
      this->playNodeAmination(node);
   }
   return false;
}

bool SceneViewer::drawScene(Node* node)
{
   // If the node visited contains a model, draw it
   Model* model = node->getModel();
   std::string id = node->getId();
   if (model && id.compare(0,1,"_"))
   {
      model->draw();
   }
   return true;
}

void SceneViewer::keyEvent(Keyboard::KeyEvent evt, int key)
{
   if (evt == Keyboard::KEY_PRESS)
   {
      switch (key)
      {
      case Keyboard::KEY_ESCAPE:
         exit();
         break;
      }
   }
}

void SceneViewer::controlEvent(Control *control, EventType evt) {
   switch(evt) {
   case Listener::CLICK:
      if (control==_bPlayAll) {
         _scene->visit(this, &SceneViewer::startAnims);
      }
      if (control==_bPlaySelected) {
         this->playNodeAmination(_selected);
      }
      break;
   case Listener::VALUE_CHANGED:
      if (control==_sliderLightVectorX) {
         _directionalLightVector.x =  _sliderLightVectorX->getValue();
         _scene->visit(this, &SceneViewer::setLights);
      }
      if (control==_sliderLightVectorY) {
         _directionalLightVector.y =  _sliderLightVectorY->getValue();
         _scene->visit(this, &SceneViewer::setLights);
      }
      if (control==_sliderLightVectorZ) {
         _directionalLightVector.z =  _sliderLightVectorZ->getValue();
         _scene->visit(this, &SceneViewer::setLights);
      }
      if (control==_sliderClearColorRed) {
         _clearColor.x = _sliderClearColorRed->getValue();
      }
      if (control==_sliderClearColorGreen) {
         _clearColor.y = _sliderClearColorGreen->getValue();
      }
      if (control==_sliderClearColorBlue) {
         _clearColor.z = _sliderClearColorBlue->getValue();
      }
   }
}

bool SceneViewer::checkScene(Node* node, Ray* ray)
{
   Model* model = node->getModel();
   if (model)
   {
      BoundingBox worldSpaceBoundingBox(model->getMesh()->getBoundingBox());
      worldSpaceBoundingBox.transform(node->getWorldMatrix());

      float distance = FLT_MAX;
      if( (distance = ray->intersects(worldSpaceBoundingBox)) != Ray::INTERSECTS_NONE )
      {
         _selected = node;
         _bPlaySelected->setText(node->getId());
         _bPlaySelected->setEnabled(true);
         return true;
      }
   }
   return false;
}

void SceneViewer::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
   switch (evt)
   {
   case Touch::TOUCH_PRESS:
      break;
   case Touch::TOUCH_RELEASE:
      {
         Ray ray;
         Camera* camera = _scene->getActiveCamera();
         if (camera) {
            camera->pickRay(getViewport(), x, y, &ray);
            _scene->visit(this, &SceneViewer::checkScene, &ray);
         }
      }
      break;
   case Touch::TOUCH_MOVE:
      break;
   };
}
