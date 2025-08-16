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

#include <QApplication>
#include <QColor>
#include <QObject>
#include <QString>
#include <QWidget>

namespace colmap {

enum class ThemeType {
  LIGHT,
  DARK,
  SYSTEM  // Follow system preference
};

class ThemeManager : public QObject {
  Q_OBJECT

 public:
  static ThemeManager& Instance();

  // Theme management
  void SetTheme(ThemeType theme);
  ThemeType GetCurrentTheme() const;
  bool IsDarkTheme() const;

  // Color palette access
  QColor GetColor(const QString& color_name) const;
  QString GetStylesheet() const;

  // Widget theming
  void ApplyThemeToWidget(QWidget* widget);
  void ApplyThemeToApplication(QApplication* app);

  // Settings persistence
  void SaveThemePreference();
  void LoadThemePreference();

 signals:
  void ThemeChanged(ThemeType new_theme);

 private:
  ThemeManager();
  ~ThemeManager() = default;
  ThemeManager(const ThemeManager&) = delete;
  ThemeManager& operator=(const ThemeManager&) = delete;

  void InitializeColorPalettes();
  void LoadStylesheet();
  QString GetStylesheetPath() const;

  ThemeType current_theme_;
  QString current_stylesheet_;
  
  // Color palettes for different themes
  struct ColorPalette {
    QColor primary;
    QColor secondary;
    QColor accent;
    QColor background;
    QColor surface;
    QColor text_primary;
    QColor text_secondary;
    QColor border;
    QColor highlight;
    QColor error;
    QColor warning;
    QColor success;
  };

  ColorPalette light_palette_;
  ColorPalette dark_palette_;
  ColorPalette* current_palette_;
};

}  // namespace colmap
