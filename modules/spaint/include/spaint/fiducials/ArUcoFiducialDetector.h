/**
 * spaint: ArUcoFiducialDetector.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_SPAINT_ARUCOFIDUCIALDETECTOR
#define H_SPAINT_ARUCOFIDUCIALDETECTOR

#include <boost/optional.hpp>

#include <opencv2/opencv.hpp>

#include "FiducialDetector.h"
#include "../picking/interface/Picker.h"

namespace spaint {

/**
 * \brief An instance of this class can be used to detect ArUco fiducials in a 3D scene.
 */
class ArUcoFiducialDetector : public FiducialDetector
{
  //#################### PRIVATE VARIABLES ####################
private:
  /** The picker used when estimating poses from the scene raycast. */
  mutable Picker_CPtr m_picker;

  /** The settings to use for InfiniTAM. */
  Settings_CPtr m_settings;

  //#################### CONSTRUCTORS ####################
public:
  /**
   * \brief Constructs an ArUco fiducial detector.
   *
   * \param settings  The settings to use for InfiniTAM.
   */
  explicit ArUcoFiducialDetector(const Settings_CPtr& settings);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  /** Override */
  virtual std::map<std::string,FiducialMeasurement> detect_fiducials(const View_CPtr& view, const ORUtils::SE3Pose& pose, const VoxelRenderState_CPtr& renderState,
                                                                     PoseEstimationMode poseEstimationMode) const;

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  /**
   * \brief TODO
   */
  std::vector<boost::optional<FiducialMeasurement> > construct_measurements_from_colour(const std::vector<int>& ids, const std::vector<std::vector<cv::Point2f> >& corners,
                                                                                        const View_CPtr& view, const ORUtils::SE3Pose& pose) const;

  /**
   * \brief TODO
   */
  std::vector<boost::optional<FiducialMeasurement> > construct_measurements_from_depth(const std::vector<int>& ids, const std::vector<std::vector<cv::Point2f> >& corners,
                                                                                       const View_CPtr& view, const ORUtils::SE3Pose& pose) const;

  /**
   * \brief TODO
   */
  std::vector<boost::optional<FiducialMeasurement> > construct_measurements_from_raycast(const std::vector<int>& ids, const std::vector<std::vector<cv::Point2f> >& corners,
                                                                                         const VoxelRenderState_CPtr& renderState) const;

  /**
   * \brief Tries to determine the 3D point in eye space that corresponds to a fiducial corner in the live colour image
   *        by back-projecting into 3D using the depth value from the live depth image.
   *
   * \param corner  The fiducial corner in the live colour image.
   * \param view    The view of the scene containing the live images.
   * \return        The 3D point in eye space corresponding to the fiducial corner (if any), or boost::none otherwise.
   */
  boost::optional<Vector3f> pick_corner_from_depth(const cv::Point2f& corner, const View_CPtr& view) const;

  /**
   * \brief Tries to determine the 3D point in world space that corresponds to a fiducial corner in the live colour image
   *        by looking it up in a raycast of the scene from the pose of the depth camera.
   *
   * \param corner      The fiducial corner in the live colour image.
   * \param renderState The render state containing the scene raycast.
   * \return            The 3D point in world space corresponding to the fiducial corner (if any), or boost::none otherwise.
   */
  boost::optional<Vector3f> pick_corner_from_raycast(const cv::Point2f& corner, const VoxelRenderState_CPtr& renderState) const;

  //#################### PRIVATE STATIC MEMBER FUNCTIONS ####################
private:
  /**
   * \brief TODO
   */
  static boost::optional<ORUtils::SE3Pose> make_pose(const boost::optional<Vector3f>& v0, const boost::optional<Vector3f>& v1, const boost::optional<Vector3f>& v2);
};

}

#endif
