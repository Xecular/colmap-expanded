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

#include <Eigen/Core>
#include <vector>

namespace colmap {

struct SuperGlueConfig {
  float match_threshold = 0.2f;
  int max_keypoints = 1024;
  bool use_mutual_check = true;
  float mutual_threshold = 0.8f;
  bool use_ratio_test = false;
  float ratio_threshold = 0.8f;
  int sinkhorn_iterations = 20;
  float sinkhorn_threshold = 1e-4f;
  bool use_superglue = true;
  float superglue_threshold = 0.2f;
};

struct SuperGlueResult {
  std::vector<std::pair<int, int>> matches;  // (idx1, idx2) pairs
  std::vector<float> match_scores;
  std::vector<bool> mutual_matches;
  double processing_time_ms = 0.0;
  int num_matches = 0;
  float match_ratio = 0.0f;
};

class SuperGlueMatcher : public MLModel {
 public:
  SuperGlueMatcher();
  ~SuperGlueMatcher() override = default;

  // MLModel interface
  bool Load(const MLModelConfig& config) override;
  bool IsLoaded() const override;
  void Unload() override;
  MLModelType GetType() const override { return MLModelType::SUPERGLUE_MATCHER; }
  std::string GetName() const override { return "SuperGlue"; }
  MLBackend GetBackend() const override { return backend_; }
  MLDevice GetDevice() const override { return device_; }

  // SuperGlue specific methods
  SuperGlueResult Match(const std::vector<FeatureKeypoint>& keypoints1,
                       const std::vector<Eigen::VectorXf>& descriptors1,
                       const std::vector<FeatureKeypoint>& keypoints2,
                       const std::vector<Eigen::VectorXf>& descriptors2,
                       const SuperGlueConfig& config = {});
  
  // Configuration
  void SetConfig(const SuperGlueConfig& config);
  SuperGlueConfig GetConfig() const;

 private:
  bool loaded_ = false;
  MLBackend backend_ = MLBackend::PYTORCH;
  MLDevice device_ = MLDevice::CPU;
  SuperGlueConfig config_;
  
  // Model-specific data (placeholder for actual implementation)
  void* model_ptr_ = nullptr;
  
  // Internal processing methods
  bool InitializeModel();
};

}  // namespace colmap
