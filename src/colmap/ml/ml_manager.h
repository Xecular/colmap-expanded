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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "colmap/util/logging.h"

namespace colmap {

enum class MLModelType {
  SUPERPONT_DETECTOR,
  SUPERGLUE_MATCHER,
  LOFTR_MATCHER,
  DISK_DETECTOR,
  R2D2_DETECTOR,
  MVSNET_MVS,
  NERF_RENDERER,
  INSTANT_NGP
};

enum class MLBackend {
  PYTORCH,
  TENSORFLOW,
  ONNX,
  OPENVINO
};

enum class MLDevice {
  CPU,
  CUDA,
  OPENCL,
  VULKAN
};

struct MLModelConfig {
  std::string model_path;
  MLBackend backend = MLBackend::PYTORCH;
  MLDevice device = MLDevice::CPU;
  bool use_fp16 = false;
  int batch_size = 1;
  float confidence_threshold = 0.5f;
  std::unordered_map<std::string, std::string> parameters;
};

class MLModel {
 public:
  virtual ~MLModel() = default;
  
  virtual bool Load(const MLModelConfig& config) = 0;
  virtual bool IsLoaded() const = 0;
  virtual void Unload() = 0;
  virtual MLModelType GetType() const = 0;
  virtual std::string GetName() const = 0;
  virtual MLBackend GetBackend() const = 0;
  virtual MLDevice GetDevice() const = 0;
};

class MLManager {
 public:
  static MLManager& Instance();

  // Model management
  bool RegisterModel(const std::string& name, 
                     std::shared_ptr<MLModel> model);
  std::shared_ptr<MLModel> GetModel(const std::string& name);
  std::shared_ptr<MLModel> GetModel(MLModelType type);
  bool LoadModel(const std::string& name, const MLModelConfig& config);
  bool UnloadModel(const std::string& name);
  void UnloadAllModels();

  // Model discovery and caching
  std::vector<std::string> GetAvailableModels() const;
  std::vector<MLModelType> GetAvailableModelTypes() const;
  bool IsModelLoaded(const std::string& name) const;
  bool IsModelTypeAvailable(MLModelType type) const;

  // Device management
  std::vector<MLDevice> GetAvailableDevices() const;
  bool IsDeviceAvailable(MLDevice device) const;
  void SetDefaultDevice(MLDevice device);
  MLDevice GetDefaultDevice() const;

  // Configuration
  void SetModelCacheDirectory(const std::string& path);
  std::string GetModelCacheDirectory() const;
  void SetDownloadEnabled(bool enabled);
  bool IsDownloadEnabled() const;

  // Utility
  void ClearCache();
  size_t GetCacheSize() const;
  void PrintModelInfo() const;

 private:
  MLManager() = default;
  ~MLManager() = default;
  MLManager(const MLManager&) = delete;
  MLManager& operator=(const MLManager&) = delete;

  std::unordered_map<std::string, std::shared_ptr<MLModel>> models_;
  std::unordered_map<MLModelType, std::string> model_type_map_;
  std::string cache_directory_;
  bool download_enabled_ = true;
  MLDevice default_device_ = MLDevice::CPU;
};

}  // namespace colmap
