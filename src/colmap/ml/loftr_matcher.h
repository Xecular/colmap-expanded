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

struct LoFTRConfig {
  float match_threshold = 0.2f;
  int max_keypoints = 2048;
  bool use_mutual_check = true;
  float mutual_threshold = 0.8f;
  int coarse_window_size = 8;
  int fine_window_size = 2;
  int coarse_level = 4;
  int fine_level = 2;
  float coarse_threshold = 0.2f;
  float fine_threshold = 0.1f;
  int num_heads = 8;
  int feature_dim = 256;
  bool use_positional_encoding = true;
  float temperature = 0.1f;
};

struct LoFTRResult {
  std::vector<std::pair<int, int>> matches;  // (idx1, idx2) pairs
  std::vector<float> match_scores;
  std::vector<bool> mutual_matches;
  std::vector<Eigen::Vector2f> keypoints1;  // Dense keypoints
  std::vector<Eigen::Vector2f> keypoints2;  // Dense keypoints
  double processing_time_ms = 0.0;
  int num_matches = 0;
  float match_ratio = 0.0f;
};

class LoFTRMatcher : public MLModel {
 public:
  LoFTRMatcher();
  ~LoFTRMatcher() override = default;

  // MLModel interface
  bool Load(const MLModelConfig& config) override;
  bool IsLoaded() const override;
  void Unload() override;
  MLModelType GetType() const override { return MLModelType::LOFTR_MATCHER; }
  std::string GetName() const override { return "LoFTR"; }
  MLBackend GetBackend() const override { return backend_; }
  MLDevice GetDevice() const override { return device_; }

  // LoFTR specific methods
  LoFTRResult Match(const Bitmap& image1, const Bitmap& image2, const LoFTRConfig& config = {});
  LoFTRResult Match(const std::string& image_path1, const std::string& image_path2, const LoFTRConfig& config = {});
  
  // Configuration
  void SetConfig(const LoFTRConfig& config);
  LoFTRConfig GetConfig() const;

 private:
  bool loaded_ = false;
  MLBackend backend_ = MLBackend::PYTORCH;
  MLDevice device_ = MLDevice::CPU;
  LoFTRConfig config_;
  
  // Model-specific data (placeholder for actual implementation)
  void* model_ptr_ = nullptr;
  
  // Internal processing methods
  bool InitializeModel();
  bool PreprocessImages(const Bitmap& image1, const Bitmap& image2, 
                       std::vector<float>& input_tensor1, std::vector<float>& input_tensor2);
  bool PostprocessOutput(const std::vector<float>& output_tensor,
                        const LoFTRConfig& config,
                        LoFTRResult& result);
  std::vector<std::pair<int, int>> ApplyMutualCheck(const std::vector<std::pair<int, int>>& matches,
                                                    const std::vector<float>& scores,
                                                    const LoFTRConfig& config);
  std::vector<Eigen::Vector2f> ExtractDenseKeypoints(const Bitmap& image, const LoFTRConfig& config);
};

}  // namespace colmap
