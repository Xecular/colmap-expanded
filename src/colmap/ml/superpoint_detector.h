// Copyright (c), ETH Zurich and UNC Chapel Hill.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of ETH Zurich and UNC Chapel Hill nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "colmap/feature/types.h"
#include "colmap/ml/ml_manager.h"
#include "colmap/sensor/bitmap.h"

#include <Eigen/Core>
#include <vector>

namespace colmap {

struct SuperPointConfig {
  int max_keypoints = 1024;
  float keypoint_threshold = 0.005f;
  bool remove_borders = true;
  int border_margin = 4;
  bool use_nms = true;
  float nms_radius = 4.0f;
  bool compute_descriptors = true;
  int descriptor_dim = 256;
  float descriptor_threshold = 0.1f;
};

struct SuperPointResult {
  std::vector<FeatureKeypoint> keypoints;
  std::vector<Eigen::VectorXf> descriptors;
  std::vector<float> scores;
  double processing_time_ms = 0.0;
};

class SuperPointDetector : public MLModel {
 public:
  SuperPointDetector();
  ~SuperPointDetector() override = default;

  // MLModel interface
  bool Load(const MLModelConfig& config) override;
  bool IsLoaded() const override;
  void Unload() override;
  MLModelType GetType() const override { return MLModelType::SUPERPONT_DETECTOR; }
  std::string GetName() const override { return "SuperPoint"; }
  MLBackend GetBackend() const override { return backend_; }
  MLDevice GetDevice() const override { return device_; }

  // SuperPoint specific methods
  SuperPointResult Detect(const Bitmap& image, const SuperPointConfig& config = {});
  
  // Configuration
  void SetConfig(const SuperPointConfig& config);
  SuperPointConfig GetConfig() const;

  // Utility methods
  std::vector<FeatureKeypoint> FilterKeypoints(const std::vector<FeatureKeypoint>& keypoints,
                                       const std::vector<float>& scores,
                                       const SuperPointConfig& config);

 private:
  bool loaded_ = false;
  MLBackend backend_ = MLBackend::PYTORCH;
  MLDevice device_ = MLDevice::CPU;
  SuperPointConfig config_;
  
  // Model-specific data (placeholder for actual implementation)
  void* model_ptr_ = nullptr;
  
  // Internal processing methods
  bool InitializeModel();
};

}  // namespace colmap
