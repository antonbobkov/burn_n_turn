#include "GuiOf.h"

namespace Gui
{
    OpnFrmImage::OpnFrmImage(ofImage* pImg_)
        :Image(Size(pImg_->getWidth(), pImg_->getHeight())), pImg(pImg_)
    {
        pPixels = pImg->getPixels();
    }
    
    OpnFrmImage::OpnFrmImage(Size sz)
        :Image(sz)
    {
        pImg = new ofImage();
        pImg->allocate(sz.x, sz.y, OF_IMAGE_COLOR_ALPHA);
        pPixels = pImg->getPixels();
    }
    
    OpnFrmImage::~OpnFrmImage()
    {
        delete pImg;
    }

    void OpnFrmImage::SetPixel(Point p, const Color& c)
    {
        int nPos = (p.x + p.y * GetSize().x) * 4;
        pPixels[nPos + 0] = c.R;
        pPixels[nPos + 1] = c.G;
        pPixels[nPos + 2] = c.B;
        pPixels[nPos + 3] = c.nTransparent;
    }
    
    Color OpnFrmImage::GetPixel(Point p) const
    {
        Color c;

        int nPos = (p.x + p.y * GetSize().x) * 4;

        c.R = pPixels[nPos + 0];
        c.G = pPixels[nPos + 1];
        c.B = pPixels[nPos + 2];
        c.nTransparent = pPixels[nPos + 3];

        return c;
    }


    OfGraphicalInterface::OfGraphicalInterface(Size sz)
    {
        pScreen = new OpnFrmImage(sz);
    }
    
    OfGraphicalInterface::~OfGraphicalInterface()
    {
        delete pScreen;
    }

    void OfGraphicalInterface::DeleteImage(OpnFrmImage* pImg)
    {
        delete pImg;
    }
    
    Image* OfGraphicalInterface::GetImage(OpnFrmImage* pImg) const
    {
        return pImg;
    }

    OpnFrmImage* OfGraphicalInterface::GetBlankImage(Size sz)
    {
        return new OpnFrmImage(sz);
    }

    OpnFrmImage* OfGraphicalInterface::LoadImage(std::string sFileName)
    {
        ofImage* pImg = new ofImage();
        if(!pImg->loadImage(sFileName))
            throw SimpleException("OfGraphicalInterface", "LoadImage", "Cannot load image from " + sFileName);
        pImg->setImageType(OF_IMAGE_COLOR_ALPHA);
        return new OpnFrmImage(pImg);
    }
    
    void OfGraphicalInterface::SaveImage(std::string sFileName, OpnFrmImage* pImg)
    {
        pImg->pImg->update();
        pImg->pImg->saveImage(sFileName);
    }

    void OfGraphicalInterface::DrawImage(Point p, OpnFrmImage* pImg, Rectangle r, bool bRefresh)
    {
        ImageOnto(pScreen, p, pImg, r);
    }

    void OfGraphicalInterface::DrawRectangle(Rectangle p, Color c, bool bRedraw)
    {
        RectangleOnto(pScreen, p, c);
    }

    void OfGraphicalInterface::Draw()
    {
        pScreen->pImg->update();
        pScreen->pImg->draw(0, 0);
    }

}