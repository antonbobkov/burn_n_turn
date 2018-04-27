#include "GuiSdl.h"

namespace Gui
{
    SDL_Rect ToSdl(Rectangle r)
    {
        SDL_Rect ret;
        ret.x = r.p.x;
        ret.y = r.p.y;
        ret.w = r.sz.x;
        ret.h = r.sz.y;

        return ret;
    }

    SdlImage::SdlImage(SDL_Surface* pImg_)
        :Image((PointerAssert<ImageNullException>("SdlImage", "Constructor", "pImg_", pImg_), Size(pImg_->w,pImg_->h))), pImg(pImg_){}

    SdlImage::SdlImage(Size sz_):Image(sz_)
    {
        pImg = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, sz_.x, sz_.y, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

        if(!pImg)
            throw SimpleSdlImageException("constructor", "Cannot create empty SDL surface" + SdlImageException::GetSdlError());
    }

    SdlImage::~SdlImage()
    {
        Unlock();
        SDL_FreeSurface(pImg);
    }
        
    SdlGraphicalInterface::SdlGraphicalInterface(Size sz)
    {
        /* Initialize the SDL library */
        if( SDL_Init(SDL_INIT_VIDEO) < 0 )
            throw SimpleSGIException("constructor", "Cannot initialize SDL video library" + SimpleSGIException::GetSdlError());

        SDL_Surface* pScrImg = SDL_SetVideoMode(sz.x, sz.y, 32, SDL_DOUBLEBUF | SDL_HWSURFACE);

        if(!pScrImg)
            throw SimpleSGIException("constructor", "Cannot set SDL video mode for the screen" + SimpleSGIException::GetSdlError());

        pScreen = new SdlImage(pScrImg);

        DrawRectangle(Rectangle(pScreen->GetSize()), Color(0,0,255));
        SDL_Flip(pScreen->pImg);
    }

    SdlGraphicalInterface::~SdlGraphicalInterface()
    {
        pScreen->Unlock();
        DeleteImage(pScreen);
        SDL_Quit();
    }

    void SdlGraphicalInterface::DeleteImage(SdlImage* pImg)
    {
        delete pImg;
    }

    Image* SdlGraphicalInterface::GetImage(SdlImage* pImg) const
    {
        return pImg;
    }

    SdlImage* SdlGraphicalInterface::CopyImage(SdlImage* pImg)
    {
        try
        {
	        Size sz = pImg->GetSize();
            
            SdlImage * pRetImg = new SdlImage(sz);
        	
            for(Crd x = 0; x < sz.x; ++x)
		        for(Crd y = 0; y < sz.y; ++y)
			            pRetImg->SetPixel(Point(x,y), pImg->GetPixel(Point(x,y)));
            
            return pRetImg;
        }
        catch(ImageException& ex)
        {
            ex.AddFnName("Copy");
            throw;
        }
    }


    SdlImage* SdlGraphicalInterface::GetBlankImage(Size sz)
    {
        try
        {
            SdlImage* pRet = new SdlImage(sz);
            RectangleOnto(pRet, Rectangle(sz), Color(0,0,0));
            return pRet;
        }
        catch(ImageException& ex)
        {
            SimpleSGIException exThr("GetBlankImage", "Cannot create blank image");
            exThr.InheritException(ex);
            throw exThr;
        }
        catch(GraphicalInterfaceException& ex)
        {
            ex.AddFnName("GetBlankImage");
            throw;
        }
    }

    SdlImage* SdlGraphicalInterface::LoadImage(std::string sFileName)
    {
        SDL_PixelFormat pxf;
        pxf.palette = 0;
        pxf.BitsPerPixel = 32;
        pxf.BytesPerPixel = 4;
        pxf.Rloss = 0;
        pxf.Gloss = 0;
        pxf.Bloss = 0;
        pxf.Aloss = 0;
        pxf.Rshift = 16;
        pxf.Gshift = 8;
        pxf.Bshift = 0;
        pxf.Ashift = 24;
        pxf.Rmask = 0x00ff0000;
        pxf.Gmask = 0x0000ff00;
        pxf.Bmask = 0x000000ff;
        pxf.Amask = 0xff000000;

        SDL_Surface* pImg = SDL_LoadBMP(sFileName.c_str());

        if(!pImg)
            throw SimpleSGIException("LoadImage", "Cannot load bmp image from file " + sFileName + SimpleSGIException::GetSdlError());

        SDL_Surface* pRet = SDL_ConvertSurface(pImg, &pxf, SDL_HWSURFACE);

        if(!pRet)
            throw SimpleSGIException("LoadImage",
            "Cannot convert image to the usable format (Loaded from: " + sFileName + ")" + SimpleSGIException::GetSdlError());

        SDL_FreeSurface(pImg);
        return new SdlImage(pRet);
    }

    void SdlGraphicalInterface::SaveImage(std::string sFileName, SdlImage* pImg)
    {
        PointerAssert<NullPointerSGIException>("SdlGraphicalInterface", "SaveImage", "pImg", pImg);

        pImg->Unlock();
        
        SDL_PixelFormat pxf;
        pxf.palette = 0;
        pxf.BitsPerPixel = 24;
        pxf.BytesPerPixel = 3;
        pxf.Rloss = 0;
        pxf.Gloss = 0;
        pxf.Bloss = 0;
        pxf.Aloss = 8;
        pxf.Rshift = 16;
        pxf.Gshift = 8;
        pxf.Bshift = 0;
        pxf.Ashift = 0;
        pxf.Rmask = 0xff0000;
        pxf.Gmask = 0x00ff00;
        pxf.Bmask = 0x0000ff;
        pxf.Amask = 0x000000;

        SDL_Surface* pSv = SDL_ConvertSurface(pImg->pImg, &pxf, SDL_HWSURFACE);

        if(!pSv)
            throw SimpleSGIException("SaveImage",
            "Cannot convert image to the saveable format (Trying to save into: " + sFileName + ")" + SimpleSGIException::GetSdlError());

        if(SDL_SaveBMP(pSv, sFileName.c_str()) < 0)
            throw SimpleSGIException("SaveImage", "Failed to save an image into " + sFileName + SimpleSGIException::GetSdlError());

        SDL_FreeSurface(pSv);
    }

    void SdlGraphicalInterface::DrawImage(Point p, SdlImage* pImg, Rectangle r, bool bRefresh/* = true*/)
    {
        PointerAssert<NullPointerSGIException>("SdlGraphicalInterface", "DrawImage", "pImg", pImg);

        try
        {
            pScreen->Unlock();
            pImg->Unlock();

            AdjustImageOverlap(pScreen->GetSize(), pImg->GetSize(), p, r);
            ImageOnto(pScreen, p, pImg, r);

	        Size sz = pImg->GetSize();
	        if(bRefresh)
                SDL_UpdateRect(pScreen->pImg, p.x, p.y, r.sz.x, r.sz.y);
        }
        catch(ImageException& ex)
        {
            SimpleSGIException exThr("Draw", "Cannot draw an image");
            exThr.InheritException(ex);
            throw exThr;
        }
        catch(GraphicalInterfaceException& ex)
        {
            ex.AddFnName("Draw");
            throw;
        }
    }

    void SdlGraphicalInterface::DrawRectangle(Rectangle p, Color c, bool bRefresh /*= true*/)
    {
        try
        {
            RectangleOnto(pScreen, p, c);
            if(bRefresh)
                Refresh(p);
        }
        catch(ImageException& ex)
        {
            SimpleSGIException exThr("DrawRectangle",
                "Cannot draw a reactangle " + RectangleToString(p) + " color " + ColorToString(c));
            exThr.InheritException(ex);
            throw exThr;
        }
        catch(GraphicalInterfaceException& ex)
        {
            ex.AddFnName("Draw");
            throw;
        }
    }

    void SdlGraphicalInterface::RectangleOnto(SdlImage* pImg, Rectangle p, Color c)
    {
        PointerAssert<NullPointerSGIException>("SdlGraphicalInterface", "RectangleOnto", "pImg", pImg);

        pImg->Unlock();
        p = Intersect(p, Rectangle(pImg->GetSize()));
        SDL_Rect r = ToSdl(p);
        if(0 > SDL_FillRect(pImg->pImg, &r, SDL_MapRGB(pImg->pImg->format, c.R, c.G, c.B)))
            throw SimpleSGIException("RectangleOnto", "Cannot draw a rectangle in an image" + SimpleSGIException::GetSdlError());
    }

    void SdlGraphicalInterface::Refresh(Rectangle r)
    {
        pScreen->Unlock();
        r = Intersect(r, pScreen->GetSize());
        SDL_UpdateRect(pScreen->pImg, r.p.x, r.p.y, r.sz.x, r.sz.y);
    }

    void SdlGraphicalInterface::RefreshAll()
    {
        pScreen->Unlock();
        if(0 > SDL_Flip(pScreen->pImg))
            Refresh(Rectangle(pScreen->GetSize()));
    }


    void SdlGraphicalInterface::ImageOnto(SdlImage* pImgDest, Point p, SdlImage* pImgSrc, Rectangle r)
    {
        PointerAssert<NullPointerSGIException>("SdlGraphicalInterface", "ImageOnto", "pImgDest", pImgDest);
        PointerAssert<NullPointerSGIException>("SdlGraphicalInterface", "ImageOnto", "pImgSrc" , pImgSrc);

        try
        {
            pImgDest->Unlock();
            pImgSrc->Unlock();
            
            bool bSelf = (pImgDest == pImgSrc);

            if(bSelf)
                pImgSrc = CopyImage(pImgDest);

            AdjustImageOverlap(pImgDest->GetSize(), pImgSrc->GetSize(), p, r);

            SDL_Rect dstRect = ToSdl(Rectangle(p, Size(0,0)));
            SDL_Rect srcRect = ToSdl(r);
            SDL_BlitSurface(pImgSrc->pImg, &srcRect, pImgDest->pImg, &dstRect);

            if(bSelf)
                DeleteImage(pImgSrc);
        }
        catch(GraphicalInterfaceException& ie)
        {
            ie.AddFnName("ImageOnto");
            throw;
        }
        catch(ImageException& ie)
        {
            SimpleSGIException gse("ImageOnto", "Cannot draw onto an image");
            gse.InheritException(ie);
            throw gse;
        }
    }
}