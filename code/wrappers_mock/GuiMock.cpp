#include "GuiMock.h"

namespace Gui {

MockImage::MockImage(Size sz) : Image(sz) {}

void MockImage::SetPixel(Point p, const Color &c) {
  (void)p;
  (void)c;
}

Color MockImage::GetPixel(Point p) const {
  (void)p;
  return Color();
}

void MockGraphicalInterface::DeleteImage(std::string pImg) {
  images_.erase(pImg);
}

Image *MockGraphicalInterface::GetImage(std::string pImg) const {
  auto it = images_.find(pImg);
  if (it == images_.end())
    return nullptr;
  return it->second.get();
}

std::string MockGraphicalInterface::GetBlankImage(Size sz) {
  std::string key = "blank_" + std::to_string(next_blank_id_++);
  images_[key] = std::make_unique<MockImage>(sz);
  return key;
}

std::string MockGraphicalInterface::LoadImage(std::string sFileName) {
  if (images_.find(sFileName) == images_.end())
    images_[sFileName] = std::make_unique<MockImage>(Size(1, 1));
  return sFileName;
}

void MockGraphicalInterface::SaveImage(std::string sFileName,
                                       std::string pImg) {
  (void)sFileName;
  (void)pImg;
}

void MockGraphicalInterface::DrawImage(Point p, std::string pImg, Rectangle r,
                                       bool bRefresh) {
  (void)p;
  (void)pImg;
  (void)r;
  (void)bRefresh;
}

void MockGraphicalInterface::DrawRectangle(Rectangle p, Color c, bool bRedraw) {
  (void)p;
  (void)c;
  (void)bRedraw;
}

void MockGraphicalInterface::RefreshAll() {}

} // namespace Gui
