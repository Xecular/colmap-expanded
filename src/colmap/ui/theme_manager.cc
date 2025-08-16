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

#include "colmap/ui/theme_manager.h"

#include "colmap/util/logging.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStyleFactory>
#include <QTextStream>

namespace colmap {

ThemeManager& ThemeManager::Instance() {
  static ThemeManager instance;
  return instance;
}

ThemeManager::ThemeManager()
    : current_theme_(ThemeType::LIGHT), current_palette_(&light_palette_) {
  InitializeColorPalettes();
  LoadThemePreference();
  LoadStylesheet();
}

void ThemeManager::InitializeColorPalettes() {
  // Light theme palette (modern, clean design)
  light_palette_.primary = QColor(41, 128, 185);      // Blue
  light_palette_.secondary = QColor(52, 152, 219);    // Lighter blue
  light_palette_.accent = QColor(230, 126, 34);       // Orange
  light_palette_.background = QColor(248, 249, 250);  // Light gray
  light_palette_.surface = QColor(255, 255, 255);     // White
  light_palette_.text_primary = QColor(33, 37, 41);   // Dark gray
  light_palette_.text_secondary = QColor(108, 117, 125); // Medium gray
  light_palette_.border = QColor(222, 226, 230);      // Light border
  light_palette_.highlight = QColor(52, 152, 219);    // Blue highlight
  light_palette_.error = QColor(220, 53, 69);         // Red
  light_palette_.warning = QColor(255, 193, 7);       // Yellow
  light_palette_.success = QColor(40, 167, 69);       // Green

  // Dark theme palette (modern dark design)
  dark_palette_.primary = QColor(52, 152, 219);       // Blue
  dark_palette_.secondary = QColor(41, 128, 185);     // Darker blue
  dark_palette_.accent = QColor(230, 126, 34);        // Orange
  dark_palette_.background = QColor(33, 37, 41);      // Dark gray
  dark_palette_.surface = QColor(52, 58, 64);         // Darker surface
  dark_palette_.text_primary = QColor(248, 249, 250); // Light gray
  dark_palette_.text_secondary = QColor(173, 181, 189); // Medium light gray
  dark_palette_.border = QColor(73, 80, 87);          // Dark border
  dark_palette_.highlight = QColor(52, 152, 219);     // Blue highlight
  dark_palette_.error = QColor(220, 53, 69);          // Red
  dark_palette_.warning = QColor(255, 193, 7);        // Yellow
  dark_palette_.success = QColor(40, 167, 69);        // Green
}

void ThemeManager::SetTheme(ThemeType theme) {
  if (current_theme_ == theme) {
    return;
  }

  current_theme_ = theme;
  
  // Update current palette pointer
  switch (theme) {
    case ThemeType::DARK:
      current_palette_ = &dark_palette_;
      break;
    case ThemeType::LIGHT:
    case ThemeType::SYSTEM:
    default:
      current_palette_ = &light_palette_;
      break;
  }

  LoadStylesheet();
  SaveThemePreference();
  
  emit ThemeChanged(theme);
}

ThemeType ThemeManager::GetCurrentTheme() const {
  return current_theme_;
}

bool ThemeManager::IsDarkTheme() const {
  return current_theme_ == ThemeType::DARK;
}

QColor ThemeManager::GetColor(const QString& color_name) const {
  if (!current_palette_) {
    return QColor();
  }

  if (color_name == "primary") return current_palette_->primary;
  if (color_name == "secondary") return current_palette_->secondary;
  if (color_name == "accent") return current_palette_->accent;
  if (color_name == "background") return current_palette_->background;
  if (color_name == "surface") return current_palette_->surface;
  if (color_name == "text_primary") return current_palette_->text_primary;
  if (color_name == "text_secondary") return current_palette_->text_secondary;
  if (color_name == "border") return current_palette_->border;
  if (color_name == "highlight") return current_palette_->highlight;
  if (color_name == "error") return current_palette_->error;
  if (color_name == "warning") return current_palette_->warning;
  if (color_name == "success") return current_palette_->success;

  return QColor();
}

QString ThemeManager::GetStylesheet() const {
  return current_stylesheet_;
}

void ThemeManager::ApplyThemeToWidget(QWidget* widget) {
  if (!widget) return;
  
  widget->setStyleSheet(current_stylesheet_);
}

void ThemeManager::ApplyThemeToApplication(QApplication* app) {
  if (!app) return;
  
  app->setStyleSheet(current_stylesheet_);
}

void ThemeManager::LoadStylesheet() {
  QString stylesheet_path = GetStylesheetPath();
  QFile file(stylesheet_path);
  
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    LOG(WARNING) << "Could not open stylesheet file: " << stylesheet_path.toStdString();
    current_stylesheet_ = "";
    return;
  }

  QTextStream in(&file);
  current_stylesheet_ = in.readAll();
  file.close();
}

QString ThemeManager::GetStylesheetPath() const {
  QString theme_name = (current_theme_ == ThemeType::DARK) ? "dark" : "light";
  return QString(":/stylesheets/%1.qss").arg(theme_name);
}

void ThemeManager::SaveThemePreference() {
  QSettings settings;
  settings.setValue("theme/type", static_cast<int>(current_theme_));
}

void ThemeManager::LoadThemePreference() {
  QSettings settings;
  int theme_value = settings.value("theme/type", static_cast<int>(ThemeType::LIGHT)).toInt();
  current_theme_ = static_cast<ThemeType>(theme_value);
  
  // Update palette pointer
  if (current_theme_ == ThemeType::DARK) {
    current_palette_ = &dark_palette_;
  } else {
    current_palette_ = &light_palette_;
  }
}

}  // namespace colmap
