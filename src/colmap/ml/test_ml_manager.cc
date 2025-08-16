// Copyright (c), ETH Zurich and UNC Chapel Hill.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the the following disclaimer.
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
#include "colmap/ml/superpoint_detector.h"
#include "colmap/ml/superglue_matcher.h"

#include <iostream>

int main() {
  std::cout << "Testing COLMAP ML Manager..." << std::endl;
  
  // Get the ML manager instance
  auto& ml_manager = colmap::MLManager::Instance();
  
  // Set up cache directory
  ml_manager.SetModelCacheDirectory("./ml_cache");
  ml_manager.SetDownloadEnabled(true);
  
  // Register some models
  auto superpoint = std::make_shared<colmap::SuperPointDetector>();
  auto superglue = std::make_shared<colmap::SuperGlueMatcher>();
  
  bool success1 = ml_manager.RegisterModel("superpoint", superpoint);
  bool success2 = ml_manager.RegisterModel("superglue", superglue);
  
  std::cout << "Registered SuperPoint: " << (success1 ? "SUCCESS" : "FAILED") << std::endl;
  std::cout << "Registered SuperGlue: " << (success2 ? "SUCCESS" : "FAILED") << std::endl;
  
  // Print model information
  ml_manager.PrintModelInfo();
  
  // Test device management
  auto devices = ml_manager.GetAvailableDevices();
  std::cout << "Available devices: ";
  for (auto device : devices) {
    std::cout << static_cast<int>(device) << " ";
  }
  std::cout << std::endl;
  
  // Set default device
  ml_manager.SetDefaultDevice(colmap::MLDevice::CPU);
  std::cout << "Default device set to: " << static_cast<int>(ml_manager.GetDefaultDevice()) << std::endl;
  
  // Test model loading (this will fail since we don't have actual model files)
  colmap::MLModelConfig config;
  config.model_path = "dummy_model.pth";
  config.backend = colmap::MLBackend::PYTORCH;
  config.device = colmap::MLDevice::CPU;
  
  bool load_success = ml_manager.LoadModel("superpoint", config);
  std::cout << "Load model test: " << (load_success ? "SUCCESS" : "EXPECTED FAILURE") << std::endl;
  
  // Test cache operations
  size_t cache_size = ml_manager.GetCacheSize();
  std::cout << "Cache size: " << cache_size << " bytes" << std::endl;
  
  std::cout << "ML Manager test completed!" << std::endl;
  return 0;
}
