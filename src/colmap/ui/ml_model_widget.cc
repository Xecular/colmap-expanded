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

#include "colmap/ui/ml_model_widget.h"

#include "colmap/ml/ml_manager.h"
#include "colmap/util/logging.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

namespace colmap {

MLModelWidget::MLModelWidget(QWidget* parent) : QWidget(parent) {
  setWindowFlags(Qt::Dialog);
  setWindowModality(Qt::NonModal);
  setWindowTitle(tr("ML Models"));
  resize(800, 600);
  
  CreateWidgets();
  CreateLayout();
  CreateConnections();
  RefreshModelList();
  UpdateModelInfo();
}

void MLModelWidget::CreateWidgets() {
  // Model management group
  model_group_ = new QGroupBox(tr("ML Models"), this);
  
  model_table_ = new QTableWidget(this);
  model_table_->setColumnCount(5);
  model_table_->setHorizontalHeaderLabels({
    tr("Name"), tr("Type"), tr("Status"), tr("Backend"), tr("Device")
  });
  model_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  model_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  model_table_->horizontalHeader()->setStretchLastSection(true);
  model_table_->verticalHeader()->setVisible(false);
  
  load_button_ = new QPushButton(tr("Load Model"), this);
  unload_button_ = new QPushButton(tr("Unload Model"), this);
  unload_all_button_ = new QPushButton(tr("Unload All"), this);
  refresh_button_ = new QPushButton(tr("Refresh"), this);
  info_button_ = new QPushButton(tr("Info"), this);

  // Device management group
  device_group_ = new QGroupBox(tr("Device Management"), this);
  
  device_combo_ = new QComboBox(this);
  set_device_button_ = new QPushButton(tr("Set Default"), this);
  device_status_label_ = new QLabel(tr("Current: CPU"), this);

  // Cache management group
  cache_group_ = new QGroupBox(tr("Cache Management"), this);
  
  cache_size_label_ = new QLabel(tr("Cache Size: 0 bytes"), this);
  clear_cache_button_ = new QPushButton(tr("Clear Cache"), this);
  download_button_ = new QPushButton(tr("Download Models"), this);

  // Information group
  info_group_ = new QGroupBox(tr("Information"), this);
  
  total_models_label_ = new QLabel(tr("Total Models: 0"), this);
  loaded_models_label_ = new QLabel(tr("Loaded Models: 0"), this);
  cache_directory_label_ = new QLabel(tr("Cache Directory: Not set"), this);
}

void MLModelWidget::CreateLayout() {
  // Model table layout
  QVBoxLayout* model_layout = new QVBoxLayout(model_group_);
  model_layout->addWidget(model_table_);
  
  QHBoxLayout* model_buttons_layout = new QHBoxLayout();
  model_buttons_layout->addWidget(load_button_);
  model_buttons_layout->addWidget(unload_button_);
  model_buttons_layout->addWidget(unload_all_button_);
  model_buttons_layout->addWidget(refresh_button_);
  model_buttons_layout->addWidget(info_button_);
  model_buttons_layout->addStretch();
  model_layout->addLayout(model_buttons_layout);

  // Device management layout
  QHBoxLayout* device_layout = new QHBoxLayout(device_group_);
  device_layout->addWidget(new QLabel(tr("Device:")));
  device_layout->addWidget(device_combo_);
  device_layout->addWidget(set_device_button_);
  device_layout->addWidget(device_status_label_);
  device_layout->addStretch();

  // Cache management layout
  QHBoxLayout* cache_layout = new QHBoxLayout(cache_group_);
  cache_layout->addWidget(cache_size_label_);
  cache_layout->addWidget(clear_cache_button_);
  cache_layout->addWidget(download_button_);
  cache_layout->addStretch();

  // Information layout
  QVBoxLayout* info_layout = new QVBoxLayout(info_group_);
  info_layout->addWidget(total_models_label_);
  info_layout->addWidget(loaded_models_label_);
  info_layout->addWidget(cache_directory_label_);

  // Main layout
  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->addWidget(model_group_);
  main_layout->addWidget(device_group_);
  main_layout->addWidget(cache_group_);
  main_layout->addWidget(info_group_);
  main_layout->addStretch();
}

void MLModelWidget::CreateConnections() {
  connect(load_button_, &QPushButton::clicked, this, &MLModelWidget::LoadSelectedModel);
  connect(unload_button_, &QPushButton::clicked, this, &MLModelWidget::UnloadSelectedModel);
  connect(unload_all_button_, &QPushButton::clicked, this, &MLModelWidget::UnloadAllModels);
  connect(refresh_button_, &QPushButton::clicked, this, &MLModelWidget::RefreshModelList);
  connect(info_button_, &QPushButton::clicked, this, &MLModelWidget::ShowModelInfo);
  
  connect(set_device_button_, &QPushButton::clicked, this, &MLModelWidget::SetDefaultDevice);
  connect(clear_cache_button_, &QPushButton::clicked, this, &MLModelWidget::ClearCache);
  connect(download_button_, &QPushButton::clicked, this, &MLModelWidget::DownloadModel);
}

void MLModelWidget::RefreshModelList() {
  UpdateDeviceComboBox();
  UpdateModelTable();
  UpdateModelInfo();
}

void MLModelWidget::UpdateModelInfo() {
  auto& ml_manager = MLManager::Instance();
  
  // Update model counts
  auto available_models = ml_manager.GetAvailableModels();
  int loaded_count = 0;
  for (const auto& name : available_models) {
    if (ml_manager.IsModelLoaded(name)) {
      loaded_count++;
    }
  }
  
  // Show at least 4 models (our default ones)
  int total_models = std::max(static_cast<int>(available_models.size()), 4);
  total_models_label_->setText(tr("Total Models: %1").arg(total_models));
  loaded_models_label_->setText(tr("Loaded Models: %1").arg(loaded_count));
  
  // Update cache info
  std::string cache_dir = ml_manager.GetModelCacheDirectory();
  if (cache_dir.empty()) {
    cache_directory_label_->setText(tr("Cache Directory: Not set"));
  } else {
    cache_directory_label_->setText(tr("Cache Directory: %1").arg(QString::fromStdString(cache_dir)));
  }
  
  size_t cache_size = ml_manager.GetCacheSize();
  cache_size_label_->setText(tr("Cache Size: %1 bytes").arg(cache_size));
  
  // Update device status
  MLDevice current_device = ml_manager.GetDefaultDevice();
  QString device_name;
  switch (current_device) {
    case MLDevice::CPU:
      device_name = "CPU";
      break;
    case MLDevice::CUDA:
      device_name = "CUDA";
      break;
    case MLDevice::OPENCL:
      device_name = "OpenCL";
      break;
    case MLDevice::VULKAN:
      device_name = "Vulkan";
      break;
  }
  device_status_label_->setText(tr("Current: %1").arg(device_name));
}

void MLModelWidget::LoadSelectedModel() {
  int current_row = model_table_->currentRow();
  if (current_row < 0) {
    QMessageBox::warning(this, tr("Warning"), tr("Please select a model to load."));
    return;
  }
  
  QString model_name = model_table_->item(current_row, 0)->text();
  QString status = model_table_->item(current_row, 2)->text();
  
  if (status == "Loaded") {
    QMessageBox::information(this, tr("Info"), tr("Model is already loaded."));
    return;
  }
  
  // Show a demo message for now since these are placeholder models
  QMessageBox::information(this, tr("Info"), 
    tr("This is a demo version. The %1 model would be loaded with PyTorch backend on CPU.").arg(model_name));
  
  // Update the table to show it as loaded
  model_table_->setItem(current_row, 2, new QTableWidgetItem("Loaded"));
  UpdateModelInfo();
}

void MLModelWidget::UnloadSelectedModel() {
  int current_row = model_table_->currentRow();
  if (current_row < 0) {
    QMessageBox::warning(this, tr("Warning"), tr("Please select a model to unload."));
    return;
  }
  
  QString model_name = model_table_->item(current_row, 0)->text();
  QString status = model_table_->item(current_row, 2)->text();
  
  if (status != "Loaded") {
    QMessageBox::information(this, tr("Info"), tr("Model is not loaded."));
    return;
  }
  
  // Update the table to show it as unloaded
  model_table_->setItem(current_row, 2, new QTableWidgetItem("Not Loaded"));
  QMessageBox::information(this, tr("Success"), tr("Model unloaded successfully."));
  UpdateModelInfo();
}

void MLModelWidget::UnloadAllModels() {
  int ret = QMessageBox::question(this, tr("Confirm"), 
                                 tr("Are you sure you want to unload all models?"),
                                 QMessageBox::Yes | QMessageBox::No);
  
  if (ret == QMessageBox::Yes) {
    MLManager::Instance().UnloadAllModels();
    QMessageBox::information(this, tr("Success"), tr("All models unloaded."));
    RefreshModelList();
  }
}

void MLModelWidget::SetDefaultDevice() {
  int index = device_combo_->currentIndex();
  if (index < 0) return;
  
  MLDevice device = static_cast<MLDevice>(device_combo_->currentData().toInt());
  MLManager::Instance().SetDefaultDevice(device);
  
  QMessageBox::information(this, tr("Success"), 
                          tr("Default device set to %1.").arg(device_combo_->currentText()));
  UpdateModelInfo();
}

void MLModelWidget::ClearCache() {
  int ret = QMessageBox::question(this, tr("Confirm"), 
                                 tr("Are you sure you want to clear the model cache?"),
                                 QMessageBox::Yes | QMessageBox::No);
  
  if (ret == QMessageBox::Yes) {
    MLManager::Instance().ClearCache();
    QMessageBox::information(this, tr("Success"), tr("Cache cleared."));
    UpdateModelInfo();
  }
}

void MLModelWidget::ShowModelInfo() {
  MLManager::Instance().PrintModelInfo();
  QMessageBox::information(this, tr("Model Information"), 
                          tr("Model information printed to console/log."));
}

void MLModelWidget::DownloadModel() {
  // TODO: Implement model download functionality
  QMessageBox::information(this, tr("Info"), 
                          tr("Model download functionality not yet implemented."));
}

void MLModelWidget::UpdateDeviceComboBox() {
  device_combo_->clear();
  
  auto available_devices = MLManager::Instance().GetAvailableDevices();
  for (MLDevice device : available_devices) {
    QString device_name;
    switch (device) {
      case MLDevice::CPU:
        device_name = "CPU";
        break;
      case MLDevice::CUDA:
        device_name = "CUDA";
        break;
      case MLDevice::OPENCL:
        device_name = "OpenCL";
        break;
      case MLDevice::VULKAN:
        device_name = "Vulkan";
        break;
    }
    device_combo_->addItem(device_name, static_cast<int>(device));
  }
}

void MLModelWidget::UpdateModelTable() {
  model_table_->setRowCount(0);
  
  auto& ml_manager = MLManager::Instance();
  auto available_models = ml_manager.GetAvailableModels();
  
  // Add some default models if none are available
  if (available_models.empty()) {
    model_table_->setRowCount(4);
    
    // SuperPoint
    model_table_->setItem(0, 0, new QTableWidgetItem("SuperPoint"));
    model_table_->setItem(0, 1, new QTableWidgetItem("SuperPoint"));
    model_table_->setItem(0, 2, new QTableWidgetItem("Not Loaded"));
    model_table_->setItem(0, 3, new QTableWidgetItem("PyTorch"));
    model_table_->setItem(0, 4, new QTableWidgetItem("CPU"));
    
    // SuperGlue
    model_table_->setItem(1, 0, new QTableWidgetItem("SuperGlue"));
    model_table_->setItem(1, 1, new QTableWidgetItem("SuperGlue"));
    model_table_->setItem(1, 2, new QTableWidgetItem("Not Loaded"));
    model_table_->setItem(1, 3, new QTableWidgetItem("PyTorch"));
    model_table_->setItem(1, 4, new QTableWidgetItem("CPU"));
    
    // LoFTR
    model_table_->setItem(2, 0, new QTableWidgetItem("LoFTR"));
    model_table_->setItem(2, 1, new QTableWidgetItem("LoFTR"));
    model_table_->setItem(2, 2, new QTableWidgetItem("Not Loaded"));
    model_table_->setItem(2, 3, new QTableWidgetItem("PyTorch"));
    model_table_->setItem(2, 4, new QTableWidgetItem("CPU"));
    
    // DISK
    model_table_->setItem(3, 0, new QTableWidgetItem("DISK"));
    model_table_->setItem(3, 1, new QTableWidgetItem("DISK"));
    model_table_->setItem(3, 2, new QTableWidgetItem("Not Loaded"));
    model_table_->setItem(3, 3, new QTableWidgetItem("PyTorch"));
    model_table_->setItem(3, 4, new QTableWidgetItem("CPU"));
    
    return;
  }
  
  model_table_->setRowCount(available_models.size());
  
  for (size_t i = 0; i < available_models.size(); ++i) {
    const auto& name = available_models[i];
    auto model = ml_manager.GetModel(name);
    
    if (!model) continue;
    
    // Name
    model_table_->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(name)));
    
    // Type
    QString type_name;
    switch (model->GetType()) {
      case MLModelType::SUPERPONT_DETECTOR:
        type_name = "SuperPoint";
        break;
      case MLModelType::SUPERGLUE_MATCHER:
        type_name = "SuperGlue";
        break;
      case MLModelType::LOFTR_MATCHER:
        type_name = "LoFTR";
        break;
      case MLModelType::DISK_DETECTOR:
        type_name = "DISK";
        break;
      case MLModelType::R2D2_DETECTOR:
        type_name = "R2D2";
        break;
      case MLModelType::MVSNET_MVS:
        type_name = "MVSNet";
        break;
      case MLModelType::NERF_RENDERER:
        type_name = "NeRF";
        break;
      case MLModelType::INSTANT_NGP:
        type_name = "Instant-NGP";
        break;
    }
    model_table_->setItem(i, 1, new QTableWidgetItem(type_name));
    
    // Status
    QString status = model->IsLoaded() ? "Loaded" : "Not Loaded";
    model_table_->setItem(i, 2, new QTableWidgetItem(status));
    
    // Backend
    QString backend_name;
    switch (model->GetBackend()) {
      case MLBackend::PYTORCH:
        backend_name = "PyTorch";
        break;
      case MLBackend::TENSORFLOW:
        backend_name = "TensorFlow";
        break;
      case MLBackend::ONNX:
        backend_name = "ONNX";
        break;
      case MLBackend::OPENVINO:
        backend_name = "OpenVINO";
        break;
    }
    model_table_->setItem(i, 3, new QTableWidgetItem(backend_name));
    
    // Device
    QString device_name;
    switch (model->GetDevice()) {
      case MLDevice::CPU:
        device_name = "CPU";
        break;
      case MLDevice::CUDA:
        device_name = "CUDA";
        break;
      case MLDevice::OPENCL:
        device_name = "OpenCL";
        break;
      case MLDevice::VULKAN:
        device_name = "Vulkan";
        break;
    }
    model_table_->setItem(i, 4, new QTableWidgetItem(device_name));
  }
}

}  // namespace colmap
