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

#include "colmap/ml/ml_manager.h"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

namespace colmap {

class MLModelWidget : public QWidget {
  Q_OBJECT

 public:
  explicit MLModelWidget(QWidget* parent = nullptr);

  void RefreshModelList();
  void UpdateModelInfo();

 private slots:
  void LoadSelectedModel();
  void UnloadSelectedModel();
  void UnloadAllModels();
  void SetDefaultDevice();
  void ClearCache();
  void ShowModelInfo();
  void DownloadModel();

 private:
  void CreateWidgets();
  void CreateLayout();
  void CreateConnections();
  void UpdateDeviceComboBox();
  void UpdateModelTable();

  // Widgets
  QGroupBox* model_group_;
  QTableWidget* model_table_;
  QPushButton* load_button_;
  QPushButton* unload_button_;
  QPushButton* unload_all_button_;
  QPushButton* refresh_button_;
  QPushButton* info_button_;

  QGroupBox* device_group_;
  QComboBox* device_combo_;
  QPushButton* set_device_button_;
  QLabel* device_status_label_;

  QGroupBox* cache_group_;
  QLabel* cache_size_label_;
  QPushButton* clear_cache_button_;
  QPushButton* download_button_;

  QGroupBox* info_group_;
  QLabel* total_models_label_;
  QLabel* loaded_models_label_;
  QLabel* cache_directory_label_;
};

}  // namespace colmap
