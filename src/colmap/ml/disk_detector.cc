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

#include "colmap/ml/disk_detector.h"

#include "colmap/util/logging.h"
#include "colmap/util/misc.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

namespace colmap {

DISKDetector::DISKDetector() {
  LOG(INFO) << "Initializing DISK detector";
}

bool DISKDetector::Load(const MLModelConfig& config) {
  if (loaded_) {
    LOG(WARNING) << "DISK model already loaded";
    return true;
  }

  LOG(INFO) << "Loading DISK model from: " << config.model_path;
  
  backend_ = config.backend;
  device_ = config.device;
  
  // Store configuration parameters
  for (const auto& [key, value] : config.parameters) {
    if (key == "max_keypoints") {
      config_.max_keypoints = std::stoi(value);
    } else if (key == "keypoint_threshold") {
      config_.keypoint_threshold = std::stof(value);
    } else if (key == "descriptor_dim") {
      config_.descriptor_dim = std::stoi(value);
    }
  }

  // TODO: Implement actual model loading based on backend
  // For now, we'll simulate successful loading
  bool success = InitializeModel();
  
  if (success) {
    loaded_ = true;
    LOG(INFO) << "Successfully loaded DISK model";
  } else {
    LOG(ERROR) << "Failed to load DISK model";
  }
  
  return success;
}

bool DISKDetector::IsLoaded() const {
  return loaded_;
}

void DISKDetector::Unload() {
  if (!loaded_) {
    return;
  }

  LOG(INFO) << "Unloading DISK model";
  
  // TODO: Implement actual model unloading
  if (model_ptr_) {
    // Free model resources
    model_ptr_ = nullptr;
  }
  
  loaded_ = false;
  LOG(INFO) << "DISK model unloaded";
}

DISKResult DISKDetector::Detect(const Bitmap& image, const DISKConfig& config) {
  DISKResult result;
  
  if (!loaded_) {
    LOG(ERROR) << "DISK model not loaded";
    return result;
  }

  auto start_time = std::chrono::high_resolution_clock::now();

  try {
    // TODO: Implement actual DISK detection
    // This would involve:
    // 1. Preprocessing the image
    // 2. Running the DISK neural network
    // 3. Post-processing the results
    // 4. Applying soft NMS and filtering
    
    const int width = image.Width();
    const int height = image.Height();
    
    // Generate dense keypoints and descriptors
    result.dense_keypoints = ExtractDenseKeypoints(image, config);
    result.dense_descriptors = ExtractDenseDescriptors(image, config);
    
    // Create dummy keypoints from dense keypoints
    std::vector<FeatureKeypoint> keypoints;
    std::vector<float> scores;
    
    for (size_t i = 0; i < result.dense_keypoints.size(); ++i) {
      FeatureKeypoint kp;
      kp.x = result.dense_keypoints[i].x();
      kp.y = result.dense_keypoints[i].y();
      kp.a11 = 1.0f;
      kp.a12 = 0.0f;
      kp.a21 = 0.0f;
      kp.a22 = 1.0f;
      
      keypoints.push_back(kp);
      scores.push_back(0.5f + 0.5f * (rand() % 100) / 100.0f);
    }
    
    // Filter keypoints
    result.keypoints = FilterKeypoints(keypoints, scores, config);
    
    // Apply soft NMS if requested
    if (config.use_nms) {
      result.keypoints = ApplySoftNMS(result.keypoints, scores, config);
    }
    
    // Generate descriptors for filtered keypoints
    if (config.compute_descriptors) {
      result.descriptors.resize(result.keypoints.size());
      for (size_t i = 0; i < result.keypoints.size(); ++i) {
        result.descriptors[i] = Eigen::VectorXf::Random(config.descriptor_dim);
        result.descriptors[i].normalize();
      }
      result.descriptors = NormalizeDescriptors(result.descriptors);
    }
    
    // Compute soft scores
    result.soft_scores = ComputeSoftScores(result.descriptors, config);
    
    // Copy scores for filtered keypoints
    result.scores.resize(result.keypoints.size());
    for (size_t i = 0; i < result.keypoints.size(); ++i) {
      result.scores[i] = 0.8f;  // Dummy score
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    result.processing_time_ms = duration.count() / 1000.0;

    LOG(INFO) << "DISK detection completed: " << result.keypoints.size() 
              << " keypoints in " << result.processing_time_ms << " ms";

  } catch (const std::exception& e) {
    LOG(ERROR) << "Exception during DISK detection: " << e.what();
  }

  return result;
}

DISKResult DISKDetector::Detect(const std::string& image_path, const DISKConfig& config) {
  // TODO: Implement file-based detection
  // This would involve:
  // 1. Loading the image from file
  // 2. Converting to Bitmap format
  // 3. Calling the Bitmap-based Detect method
  
  Bitmap bitmap;
  if (!bitmap.Read(image_path)) {
    LOG(ERROR) << "Failed to read image: " << image_path;
    return DISKResult();
  }
  
  return Detect(bitmap, config);
}

void DISKDetector::SetConfig(const DISKConfig& config) {
  config_ = config;
}

DISKConfig DISKDetector::GetConfig() const {
  return config_;
}

std::vector<FeatureKeypoint> DISKDetector::FilterKeypoints(const std::vector<FeatureKeypoint>& keypoints,
                                                          const std::vector<float>& scores,
                                                          const DISKConfig& config) {
  std::vector<FeatureKeypoint> filtered_keypoints;
  
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
    }
  }
  
  // Sort by score and limit to max_keypoints
  std::vector<size_t> indices(filtered_keypoints.size());
  std::generate(indices.begin(), indices.end(), [n = 0]() mutable { return n++; });
  std::sort(indices.begin(), indices.end(),
            [&scores](size_t a, size_t b) {
              return scores[a] > scores[b];
            });
  
  std::vector<FeatureKeypoint> result;
  size_t num_keypoints = std::min(static_cast<size_t>(config.max_keypoints), indices.size());
  for (size_t i = 0; i < num_keypoints; ++i) {
    result.push_back(filtered_keypoints[indices[i]]);
  }
  
  return result;
}

std::vector<Eigen::VectorXf> DISKDetector::NormalizeDescriptors(const std::vector<Eigen::VectorXf>& descriptors) {
  std::vector<Eigen::VectorXf> normalized;
  normalized.reserve(descriptors.size());
  
  for (const auto& desc : descriptors) {
    float norm = desc.norm();
    if (norm > 0) {
      normalized.push_back(desc / norm);
    } else {
      normalized.push_back(desc);
    }
  }
  
  return normalized;
}

std::vector<FeatureKeypoint> DISKDetector::ApplySoftNMS(const std::vector<FeatureKeypoint>& keypoints,
                                                        const std::vector<float>& scores,
                                                        const DISKConfig& config) {
  // TODO: Implement soft NMS
  // This would involve:
  // 1. Computing distances between keypoints
  // 2. Applying soft suppression based on distances
  // 3. Returning the filtered keypoints
  
  // Placeholder implementation - return all keypoints
  return keypoints;
}

bool DISKDetector::InitializeModel() {
  // TODO: Implement actual model initialization based on backend
  // For now, we'll simulate successful initialization
  LOG(INFO) << "Initializing DISK model (backend: " << static_cast<int>(backend_) 
            << ", device: " << static_cast<int>(device_) << ")";
  
  // Simulate model loading
  model_ptr_ = reinterpret_cast<void*>(0x12345678);  // Dummy pointer
  
  return true;
}

bool DISKDetector::PreprocessImage(const Bitmap& image, std::vector<float>& input_tensor) {
  // TODO: Implement actual image preprocessing for DISK
  // This would involve:
  // 1. Resizing image to model input size
  // 2. Normalizing pixel values
  // 3. Converting to tensor format
  
  const int width = image.Width();
  const int height = image.Height();
  const int channels = 3;  // DISK expects RGB
  
  input_tensor.resize(width * height * channels);
  
  // Convert to RGB and normalize to [0, 1]
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      BitmapColor<uint8_t> color;
      image.GetPixel(x, y, &color);
      
      int idx = (y * width + x) * 3;
      input_tensor[idx] = color.r / 255.0f;
      input_tensor[idx + 1] = color.g / 255.0f;
      input_tensor[idx + 2] = color.b / 255.0f;
    }
  }
  
  return true;
}

bool DISKDetector::PostprocessOutput(const std::vector<float>& output_tensor,
                                   const DISKConfig& config,
                                   DISKResult& result) {
  // TODO: Implement actual output postprocessing for DISK
  // This would involve:
  // 1. Parsing the neural network output
  // 2. Extracting keypoints and descriptors
  // 3. Applying confidence thresholds
  
  // Placeholder postprocessing
  result.keypoints.clear();
  result.descriptors.clear();
  
  // Generate dummy keypoints
  const size_t num_keypoints = std::min(static_cast<size_t>(config.max_keypoints), static_cast<size_t>(1000));
  for (size_t i = 0; i < num_keypoints; ++i) {
    FeatureKeypoint kp;
    kp.x = static_cast<float>(rand() % 640);
    kp.y = static_cast<float>(rand() % 480);
    kp.a11 = 1.0f;
    kp.a12 = 0.0f;
    kp.a21 = 0.0f;
    kp.a22 = 1.0f;
    
    result.keypoints.push_back(kp);
  }
  
  // Generate dummy descriptors
  if (config.compute_descriptors) {
    result.descriptors.resize(result.keypoints.size());
    for (size_t i = 0; i < result.keypoints.size(); ++i) {
      result.descriptors[i] = Eigen::VectorXf::Random(config.descriptor_dim);
      result.descriptors[i].normalize();
    }
  }
  
  return true;
}

std::vector<Eigen::Vector2f> DISKDetector::ExtractDenseKeypoints(const Bitmap& image, const DISKConfig& config) {
  // TODO: Implement dense keypoint extraction for DISK
  // This would involve:
  // 1. Creating a regular grid of keypoints
  // 2. Applying the DISK detection network
  // 3. Filtering by confidence
  
  const int width = image.Width();
  const int height = image.Height();
  std::vector<Eigen::Vector2f> keypoints;
  
  // Create a regular grid of keypoints
  const int step = std::max(1, std::min(width, height) / 16);
  for (int y = step; y < height - step; y += step) {
    for (int x = step; x < width - step; x += step) {
      keypoints.emplace_back(static_cast<float>(x), static_cast<float>(y));
    }
  }
  
  return keypoints;
}

std::vector<Eigen::VectorXf> DISKDetector::ExtractDenseDescriptors(const Bitmap& image, const DISKConfig& config) {
  // TODO: Implement dense descriptor extraction for DISK
  // This would involve:
  // 1. Running the DISK network on the image
  // 2. Extracting descriptors for all locations
  // 3. Normalizing the descriptors
  
  const int width = image.Width();
  const int height = image.Height();
  std::vector<Eigen::VectorXf> descriptors;
  
  // Create dummy descriptors
  const int num_locations = (width / 16) * (height / 16);
  descriptors.resize(num_locations);
  
  for (int i = 0; i < num_locations; ++i) {
    descriptors[i] = Eigen::VectorXf::Random(config.descriptor_dim);
    descriptors[i].normalize();
  }
  
  return descriptors;
}

std::vector<float> DISKDetector::ComputeSoftScores(const std::vector<Eigen::VectorXf>& descriptors, const DISKConfig& config) {
  // TODO: Implement soft score computation
  // This would involve:
  // 1. Computing descriptor statistics
  // 2. Applying soft thresholding
  // 3. Returning confidence scores
  
  std::vector<float> scores;
  scores.reserve(descriptors.size());
  
  for (const auto& desc : descriptors) {
    // Placeholder: use descriptor norm as score
    float score = desc.norm();
    if (score < config.soft_threshold) {
      score = 0.0f;
    }
    scores.push_back(score);
  }
  
  return scores;
}

}  // namespace colmap
