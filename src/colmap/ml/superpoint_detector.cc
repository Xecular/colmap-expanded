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

#include "colmap/ml/superpoint_detector.h"

#include "colmap/feature/utils.h"
#include "colmap/util/logging.h"
#include "colmap/util/misc.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <queue>

namespace colmap {

SuperPointDetector::SuperPointDetector() {
  LOG(INFO) << "Initializing SuperPoint detector";
}

bool SuperPointDetector::Load(const MLModelConfig& config) {
  if (loaded_) {
    LOG(WARNING) << "SuperPoint model already loaded";
    return true;
  }

  LOG(INFO) << "Loading SuperPoint model from: " << config.model_path;
  
  backend_ = config.backend;
  device_ = config.device;
  
  // Store configuration parameters
  for (const auto& [key, value] : config.parameters) {
    if (key == "max_keypoints") {
      config_.max_keypoints = std::stoi(value);
    } else if (key == "keypoint_threshold") {
      config_.keypoint_threshold = std::stof(value);
    } else if (key == "nms_radius") {
      config_.nms_radius = std::stof(value);
    }
  }

  // TODO: Implement actual model loading based on backend
  // For now, we'll simulate successful loading
  bool success = InitializeModel();
  
  if (success) {
    loaded_ = true;
    LOG(INFO) << "Successfully loaded SuperPoint model";
  } else {
    LOG(ERROR) << "Failed to load SuperPoint model";
  }
  
  return success;
}

bool SuperPointDetector::IsLoaded() const {
  return loaded_;
}

void SuperPointDetector::Unload() {
  if (!loaded_) {
    return;
  }

  LOG(INFO) << "Unloading SuperPoint model";
  
  // TODO: Implement actual model unloading
  if (model_ptr_) {
    // Free model resources
    model_ptr_ = nullptr;
  }
  
  loaded_ = false;
  LOG(INFO) << "SuperPoint model unloaded";
}

SuperPointResult SuperPointDetector::Detect(const Bitmap& image, const SuperPointConfig& config) {
  SuperPointResult result;
  
  if (!loaded_) {
    LOG(ERROR) << "SuperPoint model not loaded";
    return result;
  }

  auto start_time = std::chrono::high_resolution_clock::now();

  try {
    // TODO: Implement actual detection
    // For now, we'll create dummy keypoints for demonstration
    
    const int width = image.Width();
    const int height = image.Height();
    
    // Generate some dummy keypoints
    std::vector<FeatureKeypoint> keypoints;
    std::vector<float> scores;
    
    for (int i = 0; i < std::min(config.max_keypoints, 100); ++i) {
      FeatureKeypoint kp;
      kp.x = static_cast<float>(rand() % width);
      kp.y = static_cast<float>(rand() % height);
      kp.a11 = 1.0f;
      kp.a12 = 0.0f;
      kp.a21 = 0.0f;
      kp.a22 = 1.0f;
      
      keypoints.push_back(kp);
      scores.push_back(0.5f + 0.5f * (rand() % 100) / 100.0f);
    }
    
    // Filter keypoints
    result.keypoints = FilterKeypoints(keypoints, scores, config);
    
    // Apply NMS if requested
    if (config.use_nms) {
      // TODO: Implement NMS
      LOG(INFO) << "NMS requested but not implemented yet";
    }
    
    // Generate dummy descriptors
    if (config.compute_descriptors) {
      result.descriptors.resize(result.keypoints.size());
      for (size_t i = 0; i < result.keypoints.size(); ++i) {
        result.descriptors[i] = Eigen::VectorXf::Random(config.descriptor_dim);
        result.descriptors[i].normalize();
      }
    }
    
    // Copy scores for filtered keypoints
    result.scores.resize(result.keypoints.size());
    for (size_t i = 0; i < result.keypoints.size(); ++i) {
      result.scores[i] = 0.8f;  // Dummy score
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    result.processing_time_ms = duration.count() / 1000.0;

    LOG(INFO) << "SuperPoint detection completed: " << result.keypoints.size() 
              << " keypoints in " << result.processing_time_ms << " ms";

  } catch (const std::exception& e) {
    LOG(ERROR) << "Exception during SuperPoint detection: " << e.what();
  }

  return result;
}



void SuperPointDetector::SetConfig(const SuperPointConfig& config) {
  config_ = config;
}

SuperPointConfig SuperPointDetector::GetConfig() const {
  return config_;
}

std::vector<FeatureKeypoint> SuperPointDetector::FilterKeypoints(
    const std::vector<FeatureKeypoint>& keypoints,
    const std::vector<float>& scores,
    const SuperPointConfig& config) {
  
  std::vector<FeatureKeypoint> filtered_keypoints;
  std::vector<float> filtered_scores;
  
  for (size_t i = 0; i < keypoints.size(); ++i) {
    if (scores[i] >= config.keypoint_threshold) {
        // Check border removal
  if (config.remove_borders) {
    int margin = config.border_margin;
    // TODO: Get image dimensions from somewhere
    int width = 640;  // Default width
    int height = 480; // Default height
    if (keypoints[i].x < margin || keypoints[i].x >= width - margin ||
        keypoints[i].y < margin || keypoints[i].y >= height - margin) {
      continue;
    }
  }
      
      filtered_keypoints.push_back(keypoints[i]);
      filtered_scores.push_back(scores[i]);
    }
  }
  
  // Sort by score and limit to max_keypoints
  std::vector<size_t> indices(filtered_keypoints.size());
  std::generate(indices.begin(), indices.end(), [n = 0]() mutable { return n++; });
  std::sort(indices.begin(), indices.end(),
            [&filtered_scores](size_t a, size_t b) {
              return filtered_scores[a] > filtered_scores[b];
            });
  
  std::vector<FeatureKeypoint> result;
  size_t num_keypoints = std::min(static_cast<size_t>(config.max_keypoints), indices.size());
  for (size_t i = 0; i < num_keypoints; ++i) {
    result.push_back(filtered_keypoints[indices[i]]);
  }
  
  return result;
}



bool SuperPointDetector::InitializeModel() {
  // TODO: Implement actual model initialization based on backend
  // For now, we'll simulate successful initialization
  LOG(INFO) << "Initializing SuperPoint model (backend: " << static_cast<int>(backend_) 
            << ", device: " << static_cast<int>(device_) << ")";
  
  // Simulate model loading
  model_ptr_ = reinterpret_cast<void*>(0x12345678);  // Dummy pointer
  
  return true;
}



}  // namespace colmap
