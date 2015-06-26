/**
 * spaintgui: Renderer.cpp
 */

#include "Renderer.h"

#include <spaint/ogl/QuadricRenderer.h>
#include <spaint/selectiontransformers/interface/VoxelToCubeSelectionTransformer.h>
#ifdef WITH_LEAP
#include <spaint/selectors/LeapSelector.h>
#endif
#include <spaint/selectors/PickingSelector.h>
#ifdef WITH_ARRAYFIRE
#include <spaint/selectors/TouchSelector.h>
#endif
#include <spaint/util/CameraPoseConverter.h>
using namespace spaint;

//#################### LOCAL TYPES ####################

/**
 * \brief An instance of this class can be used to visit selectors in order to render them.
 */
class SelectorRenderer : public SelectionTransformerVisitor, public SelectorVisitor
{
  //~~~~~~~~~~~~~~~~~~~~ TYPEDEFS ~~~~~~~~~~~~~~~~~~~~
private:
  typedef boost::shared_ptr<const ITMUCharImage> ITMUCharImage_CPtr;

  //~~~~~~~~~~~~~~~~~~~~ PRIVATE VARIABLES ~~~~~~~~~~~~~~~~~~~~
private:
  const Renderer *m_base;
  Vector3f m_colour;
  mutable int m_selectionRadius;

  //~~~~~~~~~~~~~~~~~~~~ CONSTRUCTORS ~~~~~~~~~~~~~~~~~~~~
public:
  SelectorRenderer(const Renderer *base, const Vector3f& colour)
  : m_base(base), m_colour(colour)
  {}

  //~~~~~~~~~~~~~~~~~~~~ PUBLIC MEMBER FUNCTIONS ~~~~~~~~~~~~~~~~~~~~
public:
#ifdef WITH_LEAP
  /** Override */
  virtual void visit(const LeapSelector& selector) const
  {
    const Leap::Frame& frame = selector.get_frame();
    if(!frame.isValid() || frame.hands().count() != 1) return;

    const Leap::Hand& hand = frame.hands()[0];
    for(int fingerIndex = 0, fingerCount = hand.fingers().count(); fingerIndex < fingerCount; ++fingerIndex)
    {
      const Leap::Finger& finger = hand.fingers()[fingerIndex];

      const int boneCount = 4;  // there are four bones per finger in the Leap hand model
      for(int boneIndex = 0; boneIndex < boneCount; ++boneIndex)
      {
        const Leap::Bone& bone = finger.bone(Leap::Bone::Type(boneIndex));

        glColor3f(0.8f, 0.8f, 0.8f);
        QuadricRenderer::render_cylinder(
          LeapSelector::from_leap_vector(bone.prevJoint()),
          LeapSelector::from_leap_vector(bone.nextJoint()),
          LeapSelector::from_leap_size(bone.width() * 0.5f),
          LeapSelector::from_leap_size(bone.width() * 0.5f),
          10
        );

        glColor3f(1.0f, 0.0f, 0.0f);
        QuadricRenderer::render_sphere(LeapSelector::from_leap_vector(bone.nextJoint()), LeapSelector::from_leap_size(bone.width() * 0.7f), 10, 10);
      }
    }
  }
#endif

  /** Override */
  virtual void visit(const PickingSelector& selector) const
  {
    boost::optional<Eigen::Vector3f> pickPoint = selector.get_position();
    if(!pickPoint) return;

    render_orb(*pickPoint, m_selectionRadius * m_base->m_model->get_settings()->sceneParams.voxelSize);
  }

#ifdef WITH_ARRAYFIRE
  /** Override */
  virtual void visit(const TouchSelector& selector) const
  {
    const int selectionRadius = 1;
    std::vector<Eigen::Vector3f> touchPoints = selector.get_positions();

    for(size_t i = 0, size = touchPoints.size(); i < size; ++i)
    {
      render_orb(touchPoints[i], selectionRadius * m_base->m_model->get_settings()->sceneParams.voxelSize);
    }

    const ITMUChar4Image *rgb = m_base->m_model->get_view()->rgb;
    const ITMFloatImage *depth = m_base->m_model->get_view()->depth;

    const ITMRGBDCalib calib = *m_base->m_model->get_view()->calib;
    
    float ifx = 1.0f / calib.intrinsics_d.projectionParamsSimple.fx;
    float ify = 1.0f / calib.intrinsics_d.projectionParamsSimple.fy;
    float icx = -1.0f * calib.intrinsics_d.projectionParamsSimple.px / calib.intrinsics_d.projectionParamsSimple.fx;
    float icy = -1.0f * calib.intrinsics_d.projectionParamsSimple.py / calib.intrinsics_d.projectionParamsSimple.fy;

    float fx = calib.intrinsics_rgb.projectionParamsSimple.fx;
    float fy = calib.intrinsics_rgb.projectionParamsSimple.fy;
    float cx = calib.intrinsics_rgb.projectionParamsSimple.px;
    float cy = calib.intrinsics_rgb.projectionParamsSimple.py;

		float r11 = calib.trafo_rgb_to_depth.calib_inv.m00;
		float r12 = calib.trafo_rgb_to_depth.calib_inv.m10;
		float r13 = calib.trafo_rgb_to_depth.calib_inv.m20;
		float t1 = calib.trafo_rgb_to_depth.calib_inv.m30;

		float r21 = calib.trafo_rgb_to_depth.calib_inv.m01;
		float r22 = calib.trafo_rgb_to_depth.calib_inv.m11;
		float r23 = calib.trafo_rgb_to_depth.calib_inv.m21;
		float t2 = calib.trafo_rgb_to_depth.calib_inv.m31;

		float r31 = calib.trafo_rgb_to_depth.calib_inv.m02;
		float r32 = calib.trafo_rgb_to_depth.calib_inv.m12;
		float r33 = calib.trafo_rgb_to_depth.calib_inv.m22;
		float t3 = calib.trafo_rgb_to_depth.calib_inv.m32;

		Matrix4f depthToRgb = Matrix4f(
			fx*r11*ifx + cx*r31*ifx, fy*r21*ifx + cy*r31*ifx, r31*ifx, 0,
			fx*r12*ify + cx*r32*ify, fy*r22*ify + cy*r32*ify, r32*ify, 0,
			fx*(r11*icx + r12*icy + r13) + cx*(r31*icx + r32*icy + r33), fy*(r21*icx + r22*icy + r23) + cy*(r31*icx + r32*icy + r33), r31*icx + r32*icy + r33, 0,
			fx*t1 + cx*t3, fy*t2 + cy*t3, t3, 1);
    std::cout << depthToRgb << std::endl;


#if 0
    const ITMExtrinsics& trafo_rgb_to_depth = calib->trafo_rgb_to_depth;
    std::cout << trafo_rgb_to_depth.calib << std::endl;
#endif

    const ITMUCharImage_CPtr& touchMask = selector.get_touch_mask();

    // Copy the image to the CPU.
    rgb->UpdateHostFromDevice();
    depth->UpdateHostFromDevice();
    touchMask->UpdateHostFromDevice();

    // Create a new RGBA image to hold the texture to be rendered.
    Vector2i imgSize = touchMask->noDims;
    Renderer::ITMUChar4Image_Ptr touchImage(new ITMUChar4Image(imgSize, true, false));
    
    const Vector4u *rgbData = rgb->GetData(MEMORYDEVICE_CPU);
    const unsigned char *mask = touchMask->GetData(MEMORYDEVICE_CPU);
    const float *depthData = depth->GetData(MEMORYDEVICE_CPU);
    Vector4u *touchImageData = touchImage->GetData(MEMORYDEVICE_CPU);

    const int width = imgSize.x;
    const int height = imgSize.y;
    // Copy the rgb and mask into the new image, where the mask values are used to fill in the alpha values.
    for(int i = 0, numPixels = width * height; i < numPixels; ++i)
    {
      if(depthData[i] > 0)
      {
        float depthValue = depthData[i];
        float xScaled = static_cast<float>(i % width) * depthValue;
        float yScaled = static_cast<float>(i / width) * depthValue;
        Vector4f point3D = depthToRgb * Vector4f(xScaled, yScaled, depthValue, 1.0f);

        const int xDirtyHackOffset = 5;
        const int yDirtyHackOffset = -36;
        int trafo_x = static_cast<int>(point3D.x / point3D.z) + xDirtyHackOffset;
        int trafo_y = static_cast<int>(point3D.y / point3D.z) + yDirtyHackOffset;

        int trafo_i = trafo_y * imgSize.x + trafo_x;
        if(trafo_i >= 0 && trafo_i < numPixels)
        {
          touchImageData[i].x = rgbData[trafo_i].x;
          touchImageData[i].y = rgbData[trafo_i].y;
          touchImageData[i].z = rgbData[trafo_i].z;
        }
        touchImageData[i].w = mask[i];
      }
    }

    render_touch(touchImage);
  }
#endif

  /** Override */
  virtual void visit(const VoxelToCubeSelectionTransformer& transformer) const
  {
    m_selectionRadius = transformer.get_radius();
  }

  //~~~~~~~~~~~~~~~~~~~~ PRIVATE MEMBER FUNCTIONS ~~~~~~~~~~~~~~~~~~~~
private:
  /**
   * \brief Renders an orb with a colour denoting the current semantic label.
   *
   * \param centre  The position of the centre of the orb.
   * \param radius  The radius of the orb.
   */
  void render_orb(const Eigen::Vector3f& centre, double radius) const
  {
    glColor3f(m_colour.r, m_colour.g, m_colour.b);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    QuadricRenderer::render_sphere(centre, radius, 10, 10);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  
  /**
   * TODO.
   */
  void render_touch(const Renderer::ITMUChar4Image_CPtr& touchImage) const
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Copy the touch image to a texture.
    glBindTexture(GL_TEXTURE_2D, m_base->m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_base->m_image->noDims.x, m_base->m_image->noDims.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, touchImage->GetData(MEMORYDEVICE_CPU));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    m_base->begin_2d();
      m_base->render_textured_quad(m_base->m_textureID);
    m_base->end_2d();

    glDisable(GL_BLEND);
  }
};

//#################### CONSTRUCTORS ####################

Renderer::Renderer(const spaint::SpaintModel_CPtr& model, const spaint::SpaintRaycaster_CPtr& raycaster)
: m_cameraMode(CM_FOLLOW), m_model(model), m_raycaster(raycaster), m_raycastType(SpaintRaycaster::RT_SEMANTICLAMBERTIAN)
{}

//#################### DESTRUCTOR ####################

Renderer::~Renderer() {}

//#################### PUBLIC MEMBER FUNCTIONS ####################

Renderer::CameraMode Renderer::get_camera_mode() const
{
  return m_cameraMode;
}

void Renderer::set_camera_mode(CameraMode cameraMode)
{
  m_cameraMode = cameraMode;
}

void Renderer::set_raycast_type(SpaintRaycaster::RaycastType raycastType)
{
  m_raycastType = raycastType;
}

//#################### PROTECTED MEMBER FUNCTIONS ####################

void Renderer::begin_2d()
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslated(0.0, 1.0, 0.0);
  glScaled(1.0, -1.0, 1.0);

  glDepthMask(false);
}

void Renderer::destroy_common()
{
  m_image.reset();
  glDeleteTextures(1, &m_textureID);
}

void Renderer::end_2d()
{
  glDepthMask(true);

  // We assume that the matrix mode is still set to GL_MODELVIEW at the start of this function.
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

SpaintModel_CPtr Renderer::get_model() const
{
  return m_model;
}

SDL_Window *Renderer::get_window() const
{
  return m_window.get();
}

void Renderer::initialise_common()
{
  // Create an image into which to temporarily store visualisations of the scene.
  m_image.reset(new ITMUChar4Image(m_model->get_depth_image_size(), true, true));

  // Set up a texture in which to store the reconstructed scene.
  glGenTextures(1, &m_textureID);
}

void Renderer::render_scene(const ITMPose& pose, const spaint::SpaintInteractor_CPtr& interactor, spaint::SpaintRaycaster::RenderState_Ptr& renderState) const
{
  // Set the viewport.
  ORUtils::Vector2<int> depthImageSize = m_model->get_depth_image_size();
  glViewport(0, 0, depthImageSize.width, depthImageSize.height);

  // Clear the frame buffer.
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render the reconstructed scene, then render a synthetic scene over the top of it.
  render_reconstructed_scene(pose, renderState);
  render_synthetic_scene(pose, interactor);
}

void Renderer::set_window(const SDL_Window_Ptr& window)
{
  m_window = window;

  // Create an OpenGL context for the window.
  m_context.reset(
    SDL_GL_CreateContext(m_window.get()),
    SDL_GL_DeleteContext
  );

  // Initialise GLEW (if necessary).
#ifdef WITH_GLEW
  GLenum err = glewInit();
  if(err != GLEW_OK) throw std::runtime_error("Error: Could not initialise GLEW");
#endif
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

void Renderer::render_reconstructed_scene(const ITMPose& pose, spaint::SpaintRaycaster::RenderState_Ptr& renderState) const
{
  // Raycast the scene.
  m_raycaster->generate_free_raycast(m_image, renderState, pose, m_raycastType);

  // Copy the raycasted scene to a texture.
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image->noDims.x, m_image->noDims.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image->GetData(MEMORYDEVICE_CPU));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Render a quad textured with the raycasted scene.
  begin_2d();
    render_textured_quad(m_textureID);
  end_2d();
}

void Renderer::render_synthetic_scene(const ITMPose& pose, const SpaintInteractor_CPtr& interactor) const
{
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  {
    ORUtils::Vector2<int> depthImageSize = m_model->get_depth_image_size();
    set_projection_matrix(m_model->get_intrinsics(), depthImageSize.width, depthImageSize.height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
      // Note: Conveniently, data() returns the elements in column-major order (the order required by OpenGL).
      glLoadMatrixf(CameraPoseConverter::pose_to_modelview(pose).data());

      // Render the axes.
      glBegin(GL_LINES);
        glColor3f(1.0f, 0.0f, 0.0f);  glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
        glColor3f(0.0f, 1.0f, 0.0f);  glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
        glColor3f(0.0f, 0.0f, 1.0f);  glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 1.0f);
      glEnd();

      // Render the current selector to show how we're interacting with the scene.
      Vector3u labelColour = m_model->get_label_manager()->get_label_colour(interactor->get_semantic_label());
      Vector3f selectorColour(labelColour.r / 255.0f, labelColour.g / 255.0f, labelColour.b / 255.0f);
      SelectorRenderer selectorRenderer(this, selectorColour);
      SpaintInteractor::SelectionTransformer_CPtr transformer = interactor->get_selection_transformer();
      if(transformer) transformer->accept(selectorRenderer);
      interactor->get_selector()->accept(selectorRenderer);
    }
    glPopMatrix();
  }
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glDisable(GL_DEPTH_TEST);
}

void Renderer::render_textured_quad(GLuint textureID)
{
  glEnable(GL_TEXTURE_2D);
  {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    {
      glTexCoord2f(0, 0); glVertex2f(0, 0);
      glTexCoord2f(1, 0); glVertex2f(1, 0);
      glTexCoord2f(1, 1); glVertex2f(1, 1);
      glTexCoord2f(0, 1); glVertex2f(0, 1);
    }
    glEnd();
  }
  glDisable(GL_TEXTURE_2D);
}

void Renderer::set_projection_matrix(const ITMIntrinsics& intrinsics, int width, int height)
{
  double nearVal = 0.1;
  double farVal = 1000.0;

  // To rederive these equations, use similar triangles. Note that fx = f / sx and fy = f / sy,
  // where sx and sy are the dimensions of a pixel on the image plane.
  double leftVal = -intrinsics.projectionParamsSimple.px * nearVal / intrinsics.projectionParamsSimple.fx;
  double rightVal = (width - intrinsics.projectionParamsSimple.px) * nearVal / intrinsics.projectionParamsSimple.fx;
  double bottomVal = -intrinsics.projectionParamsSimple.py * nearVal / intrinsics.projectionParamsSimple.fy;
  double topVal = (height - intrinsics.projectionParamsSimple.py) * nearVal / intrinsics.projectionParamsSimple.fy;

  glLoadIdentity();
  glFrustum(leftVal, rightVal, bottomVal, topVal, nearVal, farVal);
}
