#ifndef GUISDL_ALREADY_INCLUDED_GUI_0316
#define GUISDL_ALREADY_INCLUDED_GUI_0316

#include <vector>
#include <fstream>

#include "SDL.h"
#include "SDL_Image.h"

#include "GuiGen.h"

namespace Gui
{

    class SdlGraphicalInterface;

    class SdlImageException: public ImageException
    {
    public:
        SdlImageException(crefString strExcName_, crefString strClsName_, crefString strFnName_)
            :ImageException(strExcName_, strClsName_, strFnName_){}

        static std::string GetSdlError(){return std::string(" (SDL error: ") + SDL_GetError() + ")";}
    };

    class SimpleSdlImageException: public SdlImageException
    {
        std::string strProblem;
    public:
        SimpleSdlImageException(crefString strFnName_, crefString strProblem_)
            :SdlImageException("SimpleSdlImageException", "SdlImage", strFnName_), strProblem(strProblem_){}

        /*virtual*/ std::string GetErrorMessage() const {return strProblem;}
    };


    class SdlImage: public Image
    {
        friend class SdlGraphicalInterface;
        
        SDL_Surface* pImg;

    public:

        SdlImage(SDL_Surface* pImg_);
        SdlImage(Size sz_);
        
        ~SdlImage();
            
        void Lock() const
        {
            if(!pImg->locked && SDL_MUSTLOCK(pImg))
			    SDL_LockSurface(pImg);
        }

        void Unlock() const
        {
            if(pImg->locked)
			    SDL_UnlockSurface(pImg);
        }

        /*virtual*/ void SetPixel(Point p, const Color& c)
	    {
            Lock();

            ((Color *) pImg->pixels)[(p.y * pImg->w + p.x)] = c;
	    }

        /*virtual*/ Color GetPixel(Point p) const 
	    {
            Lock();

            return ((Color *) pImg->pixels)[(p.y * pImg->w + p.x)];
	    }
    };

    class SdlGraphicalInterfaceException: public GraphicalInterfaceException
    {
    public:
        SdlGraphicalInterfaceException(crefString strExcName_, crefString strClsName_, crefString strFnName_)
            :GraphicalInterfaceException(strExcName_, strClsName_, strFnName_){}

        static std::string GetSdlError(){return std::string(" (SDL error: ") + SDL_GetError() + ")";}
    };

    class SimpleSGIException: public SdlGraphicalInterfaceException
    {
        std::string strProblem;
    public:
        SimpleSGIException(crefString strFnName_, crefString strProblem_)
            :SdlGraphicalInterfaceException("SimpleSGIException", "SdlGraphicalInterface", strFnName_), strProblem(strProblem_){}

        /*virtual*/ std::string GetErrorMessage() const {return strProblem;}
    };

    class NullPointerSGIException: public SdlGraphicalInterfaceException
    {
    public:
        std::string strPntName;

        NullPointerSGIException(crefString strClsName_, crefString strFnName_, crefString strPntName_)
            :SdlGraphicalInterfaceException("NullPointerSGIException", strClsName_, strFnName_), strPntName(strPntName_){}

        /*virtual*/ std::string GetErrorMessage() const {return "Null pointer passed for " + strPntName;}
    };

    class SdlGraphicalInterface: public GraphicalInterface<SdlImage*>
    {
	    SdlImage* pScreen;
    public:

        SdlGraphicalInterface(Size sz);
        ~SdlGraphicalInterface();
        
        /*virtual*/ void DeleteImage(SdlImage* pImg);
        /*virtual*/ Image* GetImage(SdlImage* pImg) const;
        /*virtual*/ SdlImage* CopyImage(SdlImage* pImg);

        /*virtual*/ SdlImage* GetBlankImage(Size sz);
        /*virtual*/ SdlImage* LoadImage(std::string sFileName);
        /*virtual*/ void SaveImage(std::string sFileName, SdlImage* pImg);
        
        /*virtual*/ void DrawImage(Point p, SdlImage* pImg, Rectangle r, bool bRefresh = true);
        
        /*virtual*/ void DrawRectangle(Rectangle p, Color c, bool bRedraw = true);
        /*virtual*/ void RectangleOnto(SdlImage* pImg, Rectangle p, Color c);

        /*virtual*/ void Refresh(Rectangle r);
        /*virtual*/ void RefreshAll();

        /*virtual*/ void ImageOnto(SdlImage* pImgDest, Point p, SdlImage* pImgSrc, Rectangle r);
    };
}

#endif //GUISDL_ALREADY_INCLUDED_GUI_0316
