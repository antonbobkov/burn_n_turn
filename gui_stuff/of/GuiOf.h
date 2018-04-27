#ifndef GUI_OPEN_FRAMEWORKS_INCLUDE_GRD_09_07_10_06_57_PM
#define GUI_OPEN_FRAMEWORKS_INCLUDE_GRD_09_07_10_06_57_PM

#undef LoadImage

#include "GuiGen.h"
#include "OfImage.h"

#undef LoadImage

namespace Gui
{
    class OfGraphicalInterface;
    
    class OpnFrmImage: public Image
    {
        friend class OfGraphicalInterface;

        ofImage* pImg;
        unsigned char* pPixels;

    public:

        OpnFrmImage(ofImage* pImg_);
        OpnFrmImage(Size sz);
        ~OpnFrmImage();

        /*virtual*/ void SetPixel(Point p, const Color& c);
        /*virtual*/ Color GetPixel(Point p) const ;
    };

    class OfGraphicalInterface: public GraphicalInterface<OpnFrmImage*>
    {
        OpnFrmImage* pScreen;
    public:
        OfGraphicalInterface(Size sz);
        ~OfGraphicalInterface();
        
        /*virtual*/ void DeleteImage(OpnFrmImage* pImg);
        /*virtual*/ Image* GetImage(OpnFrmImage* pImg) const;

        /*virtual*/ OpnFrmImage* GetBlankImage(Size sz);

        /*virtual*/ OpnFrmImage* LoadImage(std::string sFileName); 
        /*virtual*/ void SaveImage(std::string sFileName, OpnFrmImage* pImg); 

        /*virtual*/ void DrawImage(Point p, OpnFrmImage* pImg, Rectangle r, bool bRefresh = true);

        /*virtual*/ void DrawRectangle(Rectangle p, Color c, bool bRedraw = true);

        /*virtual*/ void Refresh(Rectangle r){}
        /*virtual*/ void RefreshAll(){}

        void Draw();
    };
}

#endif // GUI_OPEN_FRAMEWORKS_INCLUDE_GRD_09_07_10_06_57_PM