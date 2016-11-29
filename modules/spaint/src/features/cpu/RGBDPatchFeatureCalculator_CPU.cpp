/**
 * spaint: RGBDPatchFeatureCalculator_CPU.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "features/cpu/RGBDPatchFeatureCalculator_CPU.h"
#include "features/shared/RGBDPatchFeatureCalculator_Shared.h"

namespace spaint
{
//#################### CONSTRUCTORS ####################
RGBDPatchFeatureCalculator_CPU::RGBDPatchFeatureCalculator_CPU() :
    RGBDPatchFeatureCalculator()
{
}

//#################### PUBLIC MEMBER FUNCTIONS ####################
void RGBDPatchFeatureCalculator_CPU::compute_feature(
    const ITMUChar4Image *rgbImage, const ITMFloatImage *depthImage,
    const Vector4f &intrinsics, Keypoint3DColourImage *keypointsImage,
    RGBDPatchDescriptorImage *featuresImage, const Matrix4f &cameraPose) const
{
  const Vector4u *rgb = rgbImage->GetData(MEMORYDEVICE_CPU);
  const float *depth = depthImage->GetData(MEMORYDEVICE_CPU);

  const Vector4i *offsetsRgb = m_offsetsRgb->GetData(MEMORYDEVICE_CPU);
  const uchar *channelsRgb = m_channelsRgb->GetData(MEMORYDEVICE_CPU);
  const Vector4i *offsetsDepth = m_offsetsDepth->GetData(MEMORYDEVICE_CPU);

  Vector2i inDims = rgbImage->noDims;
  // The output images have one pixel per each element of the sampling grid.
  Vector2i outDims(rgbImage->noDims.x / m_featureStep,
      rgbImage->noDims.y / m_featureStep);

  // Resize the output images as needed
  // (typically this happens only once per run of the program if the images are properly cached).
  keypointsImage->ChangeDims(outDims);
  featuresImage->ChangeDims(outDims);

  Keypoint3DColour *keypoints = keypointsImage->GetData(MEMORYDEVICE_CPU);
  RGBDPatchDescriptor *features = featuresImage->GetData(MEMORYDEVICE_CPU);

#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
  for (int yOut = 0; yOut < outDims.height; ++yOut)
  {
    for (int xOut = 0; xOut < outDims.width; ++xOut)
    {
      const Vector2i xyOut(xOut, yOut);
      const Vector2i xyIn(xOut * m_featureStep, yOut * m_featureStep);

      compute_colour_patch_feature(keypoints, features, rgb, depth, offsetsRgb,
          channelsRgb, inDims, outDims, intrinsics, cameraPose, m_normalizeRgb,
          xyIn, xyOut);

      compute_depth_patch_feature(keypoints, features, depth, offsetsDepth,
          inDims, outDims, m_normalizeDepth, xyIn, xyOut);
    }
  }
}

}