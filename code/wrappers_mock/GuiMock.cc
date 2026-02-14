#include "GuiMock.h"

#include "color.h"

namespace Gui {

MockImage::MockImage(std::string name, Size sz) : Image(sz), name_(name) {}

void MockImage::SetPixel(Point p, const Color &c) {
  (void)p;
  (void)c;
}

Color MockImage::GetPixel(Point p) const {
  (void)p;
  return Color();
}

void MockImage::SetPixelSafe(Point p, const Color &c) {
  if (InsideRectangle(GetRectangle(), p))
    SetPixel(p, c);
}

Color MockImage::GetPixelSafe(Point p) const {
  if (!InsideRectangle(GetRectangle(), p))
    return Color();
  return GetPixel(p);
}

void MockGraphicalInterface::DeleteImage(std::string pImg) {
  images_.erase(pImg);
}

Image *MockGraphicalInterface::GetImage(std::string pImg) const {
  auto it = images_.find(pImg);
  if (it == images_.end())
    throw ImageNullException("MockGraphicalInterface", "GetImage", pImg);
  return it->second.get();
}

std::string MockGraphicalInterface::GetBlankImage(Size sz) {
  std::string key = "blank_" + std::to_string(sz.x) + "_" +
                    std::to_string(sz.y) + "_" + std::to_string(next_id_++);
  images_[key] = std::make_unique<MockImage>(key, sz);
  return key;
}

std::string MockGraphicalInterface::LoadImage(std::string sFileName) {
  std::string key = sFileName + "_" + std::to_string(next_id_++);
  images_[key] = std::make_unique<MockImage>(key, Size(1, 1));
  return key;
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
