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

#include "colmap/ml/ml_manager.h"

#include "colmap/util/logging.h"
#include "colmap/util/misc.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace colmap {

MLManager& MLManager::Instance() {
  static MLManager instance;
  return instance;
}

bool MLManager::RegisterModel(const std::string& name, 
                              std::shared_ptr<MLModel> model) {
  if (!model) {
    LOG(ERROR) << "Cannot register null model";
    return false;
  }

  if (models_.find(name) != models_.end()) {
    LOG(WARNING) << "Model '" << name << "' already registered, overwriting";
  }

  models_[name] = model;
  model_type_map_[model->GetType()] = name;
  
  LOG(INFO) << "Registered ML model: " << name << " (Type: " 
            << static_cast<int>(model->GetType()) << ")";
  return true;
}

std::shared_ptr<MLModel> MLManager::GetModel(const std::string& name) {
  auto it = models_.find(name);
  if (it == models_.end()) {
    LOG(WARNING) << "Model '" << name << "' not found";
    return nullptr;
  }
  return it->second;
}

std::shared_ptr<MLModel> MLManager::GetModel(MLModelType type) {
  auto it = model_type_map_.find(type);
  if (it == model_type_map_.end()) {
    LOG(WARNING) << "Model type " << static_cast<int>(type) << " not found";
    return nullptr;
  }
  return GetModel(it->second);
}

bool MLManager::LoadModel(const std::string& name, const MLModelConfig& config) {
  auto model = GetModel(name);
  if (!model) {
    LOG(ERROR) << "Model '" << name << "' not found";
    return false;
  }

  if (model->IsLoaded()) {
    LOG(WARNING) << "Model '" << name << "' already loaded";
    return true;
  }

  LOG(INFO) << "Loading ML model: " << name;
  bool success = model->Load(config);
  
  if (success) {
    LOG(INFO) << "Successfully loaded ML model: " << name;
  } else {
    LOG(ERROR) << "Failed to load ML model: " << name;
  }
  
  return success;
}

bool MLManager::UnloadModel(const std::string& name) {
  auto model = GetModel(name);
  if (!model) {
    LOG(WARNING) << "Model '" << name << "' not found";
    return false;
  }

  if (!model->IsLoaded()) {
    LOG(WARNING) << "Model '" << name << "' not loaded";
    return true;
  }

  LOG(INFO) << "Unloading ML model: " << name;
  model->Unload();
  return true;
}

void MLManager::UnloadAllModels() {
  LOG(INFO) << "Unloading all ML models";
  for (auto& [name, model] : models_) {
    if (model && model->IsLoaded()) {
      model->Unload();
    }
  }
}

std::vector<std::string> MLManager::GetAvailableModels() const {
  std::vector<std::string> names;
  names.reserve(models_.size());
  for (const auto& [name, model] : models_) {
    names.push_back(name);
  }
  return names;
}

std::vector<MLModelType> MLManager::GetAvailableModelTypes() const {
  std::vector<MLModelType> types;
  types.reserve(model_type_map_.size());
  for (const auto& [type, name] : model_type_map_) {
    types.push_back(type);
  }
  return types;
}

bool MLManager::IsModelLoaded(const std::string& name) const {
  auto it = models_.find(name);
  if (it == models_.end()) {
    return false;
  }
  return it->second && it->second->IsLoaded();
}

bool MLManager::IsModelTypeAvailable(MLModelType type) const {
  return model_type_map_.find(type) != model_type_map_.end();
}

std::vector<MLDevice> MLManager::GetAvailableDevices() const {
  std::vector<MLDevice> devices = {MLDevice::CPU};
  
  // Check for CUDA availability
  // TODO: Implement proper CUDA detection
  devices.push_back(MLDevice::CUDA);
  
  return devices;
}

bool MLManager::IsDeviceAvailable(MLDevice device) const {
  auto devices = GetAvailableDevices();
  return std::find(devices.begin(), devices.end(), device) != devices.end();
}

void MLManager::SetDefaultDevice(MLDevice device) {
  if (IsDeviceAvailable(device)) {
    default_device_ = device;
    LOG(INFO) << "Set default ML device to: " << static_cast<int>(device);
  } else {
    LOG(WARNING) << "Device " << static_cast<int>(device) << " not available";
  }
}

MLDevice MLManager::GetDefaultDevice() const {
  return default_device_;
}

void MLManager::SetModelCacheDirectory(const std::string& path) {
  cache_directory_ = path;
  LOG(INFO) << "Set ML model cache directory to: " << path;
}

std::string MLManager::GetModelCacheDirectory() const {
  return cache_directory_;
}

void MLManager::SetDownloadEnabled(bool enabled) {
  download_enabled_ = enabled;
  LOG(INFO) << "ML model download " << (enabled ? "enabled" : "disabled");
}

bool MLManager::IsDownloadEnabled() const {
  return download_enabled_;
}

void MLManager::ClearCache() {
  if (cache_directory_.empty()) {
    LOG(WARNING) << "Cache directory not set";
    return;
  }

  try {
    std::filesystem::remove_all(cache_directory_);
    std::filesystem::create_directories(cache_directory_);
    LOG(INFO) << "Cleared ML model cache";
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to clear cache: " << e.what();
  }
}

size_t MLManager::GetCacheSize() const {
  if (cache_directory_.empty()) {
    return 0;
  }

  try {
    size_t size = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(cache_directory_)) {
      if (entry.is_regular_file()) {
        size += entry.file_size();
      }
    }
    return size;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to calculate cache size: " << e.what();
    return 0;
  }
}

void MLManager::PrintModelInfo() const {
  LOG(INFO) << "=== ML Model Information ===";
  LOG(INFO) << "Registered models: " << models_.size();
  LOG(INFO) << "Loaded models: " << std::count_if(models_.begin(), models_.end(),
    [](const auto& pair) { return pair.second && pair.second->IsLoaded(); });
  LOG(INFO) << "Cache directory: " << (cache_directory_.empty() ? "Not set" : cache_directory_);
  LOG(INFO) << "Cache size: " << GetCacheSize() << " bytes";
  LOG(INFO) << "Download enabled: " << (download_enabled_ ? "Yes" : "No");
  LOG(INFO) << "Default device: " << static_cast<int>(default_device_);
  
  for (const auto& [name, model] : models_) {
    LOG(INFO) << "  - " << name << " (Type: " << static_cast<int>(model->GetType())
              << ", Loaded: " << (model->IsLoaded() ? "Yes" : "No")
              << ", Backend: " << static_cast<int>(model->GetBackend())
              << ", Device: " << static_cast<int>(model->GetDevice()) << ")";
  }
  LOG(INFO) << "=============================";
}

}  // namespace colmap
