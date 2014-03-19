#include "SceneViewer.h"
#include <stdlib.h>

#define FOREACH(I,ITEMS) I=ITEMS[0]; for (int i = 0 ; i < sizeof(ITEMS)/sizeof(ITEMS[0]) ; i++,I=ITEMS[i])

#define LOG(fmt, ...) do {  \
   Logger::log(Logger::LEVEL_INFO, "line %d: ", __LINE__); \
   Logger::log(Logger::LEVEL_INFO, fmt, __VA_ARGS__); \
   Logger::log(Logger::LEVEL_INFO, "\n"); \
} while (0)

inline int mod(int x) { if (x < 0) return -x; else return x; }

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

    int argc; 
    char **argv;
    this->getArguments(&argc, &argv);
    if (argc<2) {
       _scene = Scene::load("res/scene.gpb");
    } else {
        std::string path = std::string("res/")+std::string(argv[1])+std::string(".gpb");
        puts(path.c_str());
        _scene = Scene::load(path.c_str());
    }

   _shift_key = false;
   _control_key = false;
   _rotate_active = true;
   _pan_active = false;
   _zoom_active = false;
   _touch_x = 0;
   _touch_y = 0;
   _grid_on = 1;


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

   _gridCheckBox = static_cast<CheckBox*>(_form->getControl("gridCheckBox"));



   Slider * sliders[] = { _sliderLightVectorX, _sliderLightVectorY, _sliderLightVectorZ,
                        _sliderClearColorRed, _sliderClearColorGreen, _sliderClearColorBlue }, *s;
   FOREACH(s, sliders) {
      s->addListener(this, Listener::VALUE_CHANGED);
   }

   _bPlayAll->addListener(this, Listener::CLICK);
   _bPlaySelected->addListener(this, Listener::CLICK);

   _gridCheckBox->addListener(this, Control::Listener::VALUE_CHANGED);
   _gridCheckBox->setEnabled(true);

   // Set the metarial lighting
   _scene->visit(this, &SceneViewer::setLights);

   _clearColor = Vector4::zero();

   // let add a node to have the Camera looking at
   Node *rootNode = _scene->addNode("root");
   rootNode->setTranslation(0, 0, 0);
   Node *cameraNode = _scene->getActiveCamera()->getNode();
   rootNode->addChild(cameraNode);

   // Create the grid and add it to the scene.
   Model* model = createGridModel();
   _scene->addNode("grid")->setModel(model);
   model->release();
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
   //LOG("nodel id=%s", id.c_str());
   if (model && (id.compare(0, 5, "grid") == 0)) {
      if (_gridCheckBox->isChecked())
         model->draw();
      //LOG("nodel id=%s", id.c_str());
   }
   else if (model/* && id.compare(0,1,"_")*/)
   {
      model->draw();
   }
   return true;
}

// Key Mapping for view rotation, pan and zoom
// ===========================================
//
// Ideally, to match Blender, we should have:
//     Rotation:  KEY_ALT                 (LMB + drag)
//     Zoom:      KEY_CTRL  + KEY_ALT     (LMB + drag)
//     PAN:       KEY_SHIFT + KEY_ALT     (LMB + drag)
//
// However, as KEY_ALT for Win8 event seems to not be working. So the mapping will be:
//     Rotation:                          (LMB + drag)
//     PAN:       KEY_SHIFT               (LMB + drag)
//     Zoom:      KEY_CTRL                (LMB + drag)

void SceneViewer::keyEvent(Keyboard::KeyEvent evt, int key)
{
   if (evt == Keyboard::KEY_PRESS)
   {
      switch (key)
      {
      case Keyboard::KEY_ESCAPE:
         exit();
         break;
      case Keyboard::KEY_CTRL:
         _control_key = true;
         break;
      case Keyboard::KEY_SHIFT:
         _shift_key = true;
         break;
      default: break;

      }
   }
   else if (evt == Keyboard::KEY_RELEASE) {
      switch (key)
      {
      case Keyboard::KEY_CTRL:
         _control_key = false;
         break;
      case Keyboard::KEY_SHIFT:
         _shift_key = false;
         break;
      default: break;
      }
   }
   // update view rotation, pan and zoom flags
/*   if (_control_key && _shift_key) {
      _rotate_active = false;
      _zoom_active = true;
      _pan_active = false;
   }
   else */ if (_control_key) {
      _rotate_active = false;
      _zoom_active = true;
      _pan_active = false;
   }
   else if (_shift_key) {
      _rotate_active = false;
      _zoom_active = false;
      _pan_active = true;
   }
   else {
      _rotate_active = true;
      _zoom_active = false;
      _pan_active = false;
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

void SceneViewer::rotateCameraLocal(float pitch, float yaw)
{
   Node * cameraNode = _scene->getActiveCamera()->getNode();
   cameraNode->setRotation(Quaternion::identity());
   cameraNode->rotateY(yaw);
   cameraNode->rotateX(pitch);
}

void SceneViewer::rotateCameraView(float x, float y)
{
   Node *rootNode = _scene->findNode("root");
   rootNode->rotateX(y);
   rootNode->rotateY(x);
}
void SceneViewer::zoomCameraView(float x, float y)
{
   Node * cameraNode = _scene->getActiveCamera()->getNode();
   cameraNode->translateForward(-y);
}

void SceneViewer::panCameraView(float x, float y)
{
   Node *rootNode = _scene->findNode("root");
   float location_x = rootNode->getTranslationX();
   float location_y = rootNode->getTranslationY();
   float inv_x = location_x - x;
   rootNode->setTranslationX( inv_x );
   rootNode->setTranslationY(location_y + y);
}

void SceneViewer::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
   switch (evt)
   {
   case Touch::TOUCH_PRESS:
      if (contactIndex == 0) {
         _touch_x = x;
         _touch_y = y;
         //LOG("snap: x=%d, Y=%d", x, y);
      }
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

      if (contactIndex == 0) {
         int sx = 1;
         int sy = 1;
         int  delta_x, delta_y;
         Node * cameraNode = _scene->getActiveCamera()->getNode();

         delta_x = x - _touch_x;
         delta_y = y - _touch_y;
         _touch_x = x;
         _touch_y = y;
         if (_zoom_active) {
            zoomCameraView(-delta_x / 200.f, -delta_y / 200.f);
         }

         if (_rotate_active) {
            rotateCameraView(-delta_x / 500.f, delta_y / 500.f);
         }

         if (_pan_active) {
            panCameraView(-delta_x / 500.f, delta_y / 500.f);
         }
      }


      break;
   }
}

void SceneViewer::lookAt(Node* node, const Vector3& eye, const Vector3& target, const Vector3& up)
{
   // Create lookAt matrix
   Matrix matrix;
   Matrix::createLookAt(eye, target, up, &matrix);
   matrix.transpose();

   // Pull rotation out of matrix
   Quaternion rotation;
   matrix.getRotation(&rotation);

   // Set rotation on node
   node->setRotation(rotation);
}

Model* SceneViewer::createGridModel(unsigned int lineCount)
{
   // There needs to be an odd number of lines
   lineCount |= 1;
   const unsigned int pointCount = lineCount * 4;
   const unsigned int verticesSize = pointCount * (3 + 3);  // (3 (position(xyz) + 3 color(rgb))

   std::vector<float> vertices;
   vertices.resize(verticesSize);

   const float gridLength = (float)(lineCount / 2);
   float value = -gridLength;
   for (unsigned int i = 0; i < verticesSize; ++i)
   {
      // Default line color is dark grey
      Vector4 color(0.3f, 0.3f, 0.3f, 1.0f);

      // Very 10th line is brighter grey
      if (((int)value) % 10 == 0)
      {
         color.set(0.45f, 0.45f, 0.45f, 1.0f);
      }

      // The Z axis is blue
      if (value == 0.0f)
      {
         color.set(0.15f, 0.15f, 0.7f, 1.0f);
      }

      // Build the lines
      vertices[i] = value;
      vertices[++i] = 0.0f;
      vertices[++i] = -gridLength;
      vertices[++i] = color.x;
      vertices[++i] = color.y;
      vertices[++i] = color.z;

      vertices[++i] = value;
      vertices[++i] = 0.0f;
      vertices[++i] = gridLength;
      vertices[++i] = color.x;
      vertices[++i] = color.y;
      vertices[++i] = color.z;

      // The X axis is red
      if (value == 0.0f)
      {
         color.set(0.7f, 0.15f, 0.15f, 1.0f);
      }
      vertices[++i] = -gridLength;
      vertices[++i] = 0.0f;
      vertices[++i] = value;
      vertices[++i] = color.x;
      vertices[++i] = color.y;
      vertices[++i] = color.z;

      vertices[++i] = gridLength;
      vertices[++i] = 0.0f;
      vertices[++i] = value;
      vertices[++i] = color.x;
      vertices[++i] = color.y;
      vertices[++i] = color.z;

      value += 1.0f;
   }
   VertexFormat::Element elements[] =
   {
      VertexFormat::Element(VertexFormat::POSITION, 3),
      VertexFormat::Element(VertexFormat::COLOR, 3)
   };
   Mesh* mesh = Mesh::createMesh(VertexFormat(elements, 2), pointCount, false);
   if (mesh == NULL)
   {
      return NULL;
   }
   mesh->setPrimitiveType(Mesh::LINES);
   mesh->setVertexData(&vertices[0], 0, pointCount);

   Model* model = Model::create(mesh);
   model->setMaterial("res/scene_viewer.material#grid");
   SAFE_RELEASE(mesh);
   return model;
}
