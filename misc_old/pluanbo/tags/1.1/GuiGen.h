#ifndef GUIGEN_ALREADY_INCLUDED_GUI_0308
#define GUIGEN_ALREADY_INCLUDED_GUI_0308


#include <string>
#include <vector>
#include <list>

#include <fstream>

namespace Gui
{
    template<class T>
    inline T Gmin(T a, T b)
    {return a < b ? a : b;}

    template<class T>
    inline T Gmin(T a, T b, T c)
    {return Gmin(a, Gmin(b,c));}

    template<class T>
    inline T Gmax(T a, T b)
    {return a > b ? a : b;}

    template<class T>
    inline T Gmax(T a, T b, T c)
    {return Gmax(a, Gmax(b,c));}

    typedef int Crd;
    typedef unsigned char Byte;

    typedef const std::string& crefString;

    // MyException - error message + some information about error
    class MyException
    {
        std::string strExcName;         // name of exception
        std::string strClsName;         // name of class generated exception
        std::string strFnNameTrack;     // list of function names
        std::string strInherited;       // description of inherited exceptions

        std::string GetHeader() const;

    protected:
        void ResetName(crefString istrExcName);
        virtual std::string GetErrorMessage() const=0;

    public:
        MyException(crefString strExcName_, crefString strClsName_, crefString strFnName_);
        virtual ~MyException(){}

        void AddFnName(crefString strFnName);
        void InheritException(const MyException& crefExc);

        std::string GetDescription(bool bDetailed = false) const;
    };

    class SimpleException: public MyException // simple string exception
    {
        std::string strProblem;
    public:
        SimpleException(crefString strClsName_, crefString strFnName_, crefString strProblem_)
            :MyException("SimpleException", strClsName_, strFnName_), strProblem(strProblem_){}

        SimpleException(crefString strProblem_)
            :MyException("<N/A>", "<N/A>", "<N/A>"), strProblem(strProblem_){}
        
        /*virtual*/ std::string GetErrorMessage() const
        {
            return strProblem;
        }
    };

    struct Color
    {
        Byte B,G,R;
        Byte nTransparent;  // 0 - fully transparent

        Color():R(0), G(0), B(0), nTransparent(Byte(255)){}
        Color(Byte R_, Byte G_, Byte B_, Byte nTransparent_ = Byte(255)):R(R_), G(G_), B(B_), nTransparent(nTransparent_){}
        Color(int R_, int G_, int B_, int nTransparent_ = 255)  // for convenience
            :R(Byte(R_)), G(Byte(G_)), B(Byte(B_)), nTransparent(Byte(nTransparent_)){}
    };

    // two fully transparent colors are equal, otherwise compare color components only
    inline bool operator == (const Color& c1, const Color& c2) 
    {
        if (c1.nTransparent == 0 && c2.nTransparent == 0)
            return true;
        return (c1.B == c2.B) && (c1.G == c2.G) && (c1.R == c2.R);
    }

    std::string ColorToString(Color c); // for output

    struct Point
    {
        Crd x, y;

        Point():x(0), y(0){}
        Point(Crd x_, Crd y_):x(x_), y(y_){}

        Point& operator += (const Point& p){x += p.x; y += p.y; return *this;}
        Point& operator -= (const Point& p){x -= p.x; y -= p.y; return *this;}
    };

    inline Point operator + (const Point& p1, const Point& p2) {return Point(p1) += p2;}
    inline Point operator - (const Point& p1, const Point& p2) {return Point(p1) -= p2;}
    inline bool operator == (const Point& p1, const Point& p2) {return p1.x == p2.x && p1.y == p2.y;}
    inline bool operator != (const Point& p1, const Point& p2) {return p1.x != p2.x || p1.y != p2.y;}

    struct Size
    {
        Crd x, y;

        Size():x(0), y(0){}
        Size(Crd x_, Crd y_):x(x_), y(y_){}

        Crd Area() const {return x * y;}
    };

    inline bool operator == (const Size& p1, const Size& p2) {return p1.x == p2.x && p1.y == p2.y;}
    inline bool operator != (const Size& p1, const Size& p2) {return p1.x != p2.x || p1.y != p2.y;}

    struct Rectangle    // rectangle, cannot have negative size, zero rectangle has size (0,0)
    {
        Point p;
        Size sz;

        Rectangle():p(0,0), sz(0,0){}
        Rectangle(Size sz_):p(Point(0,0)), sz(sz_){Normalize();}
        Rectangle(Point p_, Size sz_):p(p_), sz(sz_){Normalize();}
        Rectangle(Point p1, Point p2):p(p1), sz(Size(p2.x - p1.x, p2.y - p1.y)){Normalize();}
        Rectangle(Crd l, Crd t, Crd r, Crd b):p(l, t), sz(r - l, b - t){Normalize();}

        void Normalize()
        {
            if(sz.x < 0 || sz.y < 0)
                sz = Size(0,0);
        }
        
        Crd Left() const {return p.x;}
        Crd Right() const{return p.x + sz.x;}
        Crd Top() const{return p.y;}
        Crd Bottom() const{return p.y + sz.y;}

        Point GetBottomRight() const{return Point(p.x + sz.x, p.y + sz.y);}

        Crd Area() const{return sz.Area();}
    };

    inline bool InsideRectangle(Rectangle r, Point p)   // is the point inside the rectangle?
    {
        return r.p.x <= p.x && p.x < r.p.x + r.sz.x && r.p.y <= p.y && p.y < r.p.y + r.sz.y;
    }

    inline Rectangle operator + (const Rectangle& r, const Point& p)    // displace
    {
        return Rectangle(r.p + p, r.sz);
    }


    inline Rectangle operator + (const Rectangle& r1, const Rectangle& r2)
    {
        return Rectangle(r1.p + r2.p, Size(r1.sz.x + r2.sz.x, r1.sz.y + r2.sz.y));
    }
    
    inline Rectangle Intersect(const Rectangle& r1, const Rectangle& r2)    // return intersection of two rectangles
    {
        Crd l = Gmax(r1.Left(), r2.Left());
        Crd r = Gmin(r1.Right(), r2.Right());
        Crd t = Gmax(r1.Top(), r2.Top());
        Crd b = Gmin(r1.Bottom(), r2.Bottom());

        return Rectangle(l, t, r, b);
    }

    std::string RectangleToString(Rectangle r); // for output

    struct MatrixErrorInfo  // stores info for out of range exception
    {
        Size sz;
        Point p;

        MatrixErrorInfo(Size sz_, Point p_)
            :sz(sz_), p(p_){}

        std::string GetErrorMessage() const;
    };

    class MatrixException: public MyException   // out of range exception
    {
    public:
        MatrixErrorInfo inf;

        MatrixException(crefString strClsName_, crefString strFnName_, Size sz, Point p)
            :MyException("MatrixException", strClsName_, strFnName_), inf(sz, p){}

        MatrixException(crefString strClsName_, crefString strFnName_, MatrixErrorInfo inf_)
            :MyException("MatrixException", strClsName_, strFnName_), inf(inf_){}

        /*virtual*/ std::string GetErrorMessage() const {return inf.GetErrorMessage();}
    };


    // helper class, describes transparency grid
    // isn't used in general implementation
    struct TransparencyGrid
    {
        Size sz;
        std::vector<Byte> vGrid;

        TransparencyGrid(Size sz_):sz(sz_), vGrid(sz.x*sz.y, 255){}

        void Set(Point p, Byte bTrans)
        {
            vGrid[p.x + p.y * sz.x] = bTrans;
        }
        
        Byte Get(Point p) const
        {
            return vGrid[p.x + p.y * sz.x];
        }

        void SetSafe(Point p, Byte bTrans)
        {
            if(!InsideRectangle(Rectangle(sz), p))
                throw MatrixException("TransparencyGrid", "SetSafe", sz, p);

            vGrid[p.x + p.y * sz.x] = bTrans;
        }
        
        Byte GetSafe(Point p) const
        {
            if(!InsideRectangle(Rectangle(sz), p))
                throw MatrixException("TransparencyGrid", "GetSafe", sz, p);

            return vGrid[p.x + p.y * sz.x];
        }
    };

    class ImageException: public MyException
    {
    public:
        ImageException(crefString strExcName_, crefString strClsName_, crefString strFnName_)
            :MyException(strExcName_, strClsName_, strFnName_){}
    };

    class ImageMatrixException: public ImageException
    {
    public:
        MatrixErrorInfo inf;

        ImageMatrixException(crefString strClsName_, crefString strFnName_, Size sz, Point p)
            :ImageException("ImageMatrixException", strClsName_, strFnName_), inf(sz, p){}

        /*virtual*/ std::string GetErrorMessage() const {return inf.GetErrorMessage();}
    };

    template<class NullExc, class T>
    T PointerAssert(crefString strClsName_, crefString strFnName_, crefString strPntName_, T pnt)
    {
        if(!pnt)
            throw NullExc(strClsName_, strFnName_, strPntName_);
        return pnt;
    }

    class ImageNullException: public ImageException
    {
    public:
        std::string strPntName;

        ImageNullException(crefString strClsName_, crefString strFnName_, crefString strPntName_)
            :ImageException("ImageNullException", strClsName_, strFnName_), strPntName(strPntName_){}

        /*virtual*/ std::string GetErrorMessage() const {return "Null pointer passed for " + strPntName;}
    };



    // Image - main building block of drawing class, handled by grafical interface class
    class Image
    {
        Size sz;
    public:
        Image(Size sz_):sz(sz_){}
        virtual ~Image(){}

        Size GetSize() const {return sz;}
        Rectangle GetRectangle() const {return Rectangle(sz);}

        virtual void SetPixel(Point p, const Color& c)=0;       // no excetions (no checks)
        virtual Color GetPixel(Point p) const=0;

        virtual void SetPixelSafe(Point p, const Color& c);     // checks if we are out bounds
        virtual Color GetPixelSafe(Point p) const;

        virtual void ChangeColor(const Color& cFrom, const Color& cTo);     // changes all instances of color cFrom to color cTo

        virtual void SetTransparentColor(const Color& c);               // makes all points with color c transparent
        virtual void ColorTransparent(const Color& c);                  // colors all transparent points to color c
    };


    class GraphicalInterfaceException: public MyException
    {
    public:
        GraphicalInterfaceException(crefString strExcName_, crefString strClsName_, crefString strFnName_)
            :MyException(strExcName_, strClsName_, strFnName_){}
    };

    class GraphicalInterfaceSimpleException: public GraphicalInterfaceException
    {
        std::string strProblem;
    public:
        GraphicalInterfaceSimpleException(crefString strClsName_, crefString strFnName_, crefString strProblem_)
            :GraphicalInterfaceException("GraphicalInterfaceSimpleException", strClsName_, strFnName_), strProblem(strProblem_){}

        /*virtual*/ std::string GetErrorMessage() const {return strProblem;}
    };

    class NullPointerGIException: public GraphicalInterfaceException
    {
    public:
        std::string strPntName;

        NullPointerGIException(crefString strClsName_, crefString strFnName_, crefString strPntName_)
            :GraphicalInterfaceException("NullPointerGIException", strClsName_, strFnName_), strPntName(strPntName_){}

        /*virtual*/ std::string GetErrorMessage() const {return "Null pointer passed for " + strPntName;}
    };

    // Graphical interface - handles all drawing
    template<class ImageHndl>
    class GraphicalInterface
    {
    public:
        virtual ~GraphicalInterface(){}

        virtual void DeleteImage(ImageHndl pImg)=0;
        virtual Image* GetImage(ImageHndl pImg)=0;
        virtual ImageHndl CopyImage(ImageHndl pImg)=0;

        virtual ImageHndl GetBlankImage(Size sz)=0;                            // create blank image of specified size
        
        virtual ImageHndl LoadImage(std::string sFileName);                    // loads image from file
        virtual void       SaveImage(std::string sFileName, ImageHndl pImg);   // save image to file

        virtual void DrawImage(Point p, ImageHndl pImg, bool bRefresh = true); // draws an image
        virtual void DrawImage(Point p, ImageHndl pImg, Rectangle r, bool bRefresh = true)=0;  // draws part of the image

        virtual void DrawRectangle(Rectangle p, Color c, bool bRedraw = true)=0;    // draws rectangle onto a screen
        virtual void RectangleOnto(ImageHndl pImg, Rectangle p, Color c);          // draws rectangle onto some image

        virtual void Refresh(Rectangle r)=0;                                    // refreshes part of the screen
        virtual void RefreshAll()=0;                                            // refreshes all screen

        // draws one image onto another (it is okay for one image not to be fully inside another)
        // p or r.p can have negative values
        virtual void ImageOnto(ImageHndl pImgDest, Point p, ImageHndl pImgSrc, Rectangle r);  
    };

    template<class ImageHndl>
    struct SimpleGraphicalInterface: public GraphicalInterface<unsigned>
    {
        GraphicalInterface<ImageHndl>* pGr;

        std::vector<ImageHndl> vStorage;
        std::list<unsigned> lsFree;
        
    public:
        
        SimpleGraphicalInterface(GraphicalInterface<ImageHndl>* pGr_):pGr(pGr_){}
        
        /*virtual*/ void DeleteImage(unsigned pImg);
        /*virtual*/ Image* GetImage(unsigned pImg);
        /*virtual*/ unsigned CopyImage(unsigned pImg);

        /*virtual*/ unsigned GetBlankImage(Size sz);                            // create blank image of specified size
        
        /*virtual*/ unsigned LoadImage(std::string sFileName);                    // loads image from file
        /*virtual*/ void     SaveImage(std::string sFileName, unsigned pImg);     // save image to file

        /*virtual*/ void DrawImage(Point p, unsigned pImg, bool bRefresh = true); // draws an image
        /*virtual*/ void DrawImage(Point p, unsigned pImg, Rectangle r, bool bRefresh = true);  // draws part of the image

        /*virtual*/ void DrawRectangle(Rectangle p, Color c, bool bRedraw = true);    // draws rectangle onto a screen
        /*virtual*/ void RectangleOnto(unsigned pImg, Rectangle p, Color c);          // draws rectangle onto some image

        /*virtual*/ void Refresh(Rectangle r);                                    // refreshes part of the screen
        /*virtual*/ void RefreshAll();                                            // refreshes all screen

        // draws one image onto another (it is okay for one image not to be fully inside another)
        // p or r.p can have negative values
        /*virtual*/ void ImageOnto(unsigned pImgDest, Point p, unsigned pImgSrc, Rectangle r);  
    };


//--------------------------------------------------------------------------------------------------
// Implementation of various helper functions and template definitions
//--------------------------------------------------------------------------------------------------
    
    // primarily used by ImageOnto - adjusts p and r so that we don't attempt to write outside of pictures while drawing
    void AdjustImageOverlap(Size sz1, Size sz2, Point& p, Rectangle& r);

    // default loading and saving functions (works only with bmp format)
    template<class ImageHndl>
    ImageHndl  GuiLoadImage(std::istream* pStr, GraphicalInterface<ImageHndl>* pGr);
    void        GuiSaveImage(std::ostream* pStr, const Image* pImg);

    template <class ImageHndl>
    void GraphicalInterface<ImageHndl>::DrawImage(Point p, ImageHndl pImg, bool bRefresh/* = true*/)
    {
        PointerAssert<ImageNullException>("GraphicalInterface", "DrawImage(small)", "pImg", pImg);
        
        DrawImage(p, pImg, Rectangle(GetImage(pImg)->GetSize()), bRefresh);   // draw all image
    }


    inline unsigned GetBmpPos(Point p, Size sz) // bmp coordinate recount function
    {
        return (3*sz.x + 3 - (3*sz.x - 1)%4) * (sz.y - p.y - 1) + p.x * 3;
    }

    inline Byte DrawColorOnto(float fColor1, float fColor2, float fTr2) // formula for drawing transparent colors
    {
        return Byte((fColor1 * (1 - fTr2/255) + fTr2/255*fColor2));
    }

    inline void ExplDrawColorOnto(Byte& fColor1, float fColor2, float fTr2) // faster version of DrawColorOnto
    {
        fColor1 = Byte((float(fColor1) * (1 - fTr2/255) + fTr2/255*fColor2));
    }

    // default implementation - uses GuiLoadImage
    template <class ImageHndl>
    ImageHndl GraphicalInterface<ImageHndl>::LoadImage(std::string sFileName)
    {
        std::ifstream ifs(sFileName.c_str(), std::ios_base::in | std::ios_base::binary);
        
        if(ifs.fail())
            throw GraphicalInterfaceSimpleException("GraphicalInterface", "LoadImage",
            "Cannot load image (cannot open file " + sFileName + ")");
        
        try {return GuiLoadImage(&ifs, this);}
        catch (SimpleException& se)
        {
            GraphicalInterfaceSimpleException ge("GraphicalInterface", "LoadImage",
            "Cannot load image from file " + sFileName);
            ge.InheritException(se);
            throw ge;
        }
    }

    // default implementation - uses GuiSaveImage
    template <class ImageHndl>
    void GraphicalInterface<ImageHndl>::SaveImage(std::string sFileName, ImageHndl pImg)
    {
        PointerAssert<NullPointerGIException>("GraphicalInterface", "SaveImage", "pImg", pImg);

        std::ofstream ofs(sFileName.c_str(), std::ios_base::out | std::ios_base::binary);
        
        if(ofs.fail())
            throw GraphicalInterfaceSimpleException("GraphicalInterface", "SaveImage",
            "Cannot save image (cannot open file " + sFileName + ")");
        
        try{GuiSaveImage(&ofs, GetImage(pImg));}
        catch (SimpleException& se)
        {
            GraphicalInterfaceSimpleException ge("GraphicalInterface", "SaveImage",
            "Cannot save image into file " + sFileName);
            ge.InheritException(se);
            throw ge;
        }
    }


    template <class ImageHndl>
    void GraphicalInterface<ImageHndl>::RectangleOnto(ImageHndl pImg, Rectangle p, Color c)
    {
        PointerAssert<NullPointerGIException>("GraphicalInterface", "RectangleOnto", "pImg", pImg);

        p = Intersect(p, GetImage(pImg)->GetSize());
        
        Point i;
        Size sz(p.p.x + p.sz.x, p.p.y + p.sz.y);
        for(i.y = p.p.y; i.y < sz.y; ++i.y)
        for(i.x = p.p.x; i.x < sz.x; ++i.x)
            GetImage(pImg)->SetPixel(i, c);
    }

    template <class ImageHndl>
    void GraphicalInterface<ImageHndl>::ImageOnto(ImageHndl pImgDest, Point p, ImageHndl pImgSrc, Rectangle r)
    {
        PointerAssert<NullPointerGIException>("GraphicalInterface", "ImageOnto", "pImgDest", pImgDest);
        PointerAssert<NullPointerGIException>("GraphicalInterface", "ImageOnto", "pImgSrc" , pImgSrc);

        try
        {
            bool bSelf = (pImgDest == pImgSrc);         // take care of the case when we try to draw image onto itself

            if(bSelf)
                pImgSrc = CopyImage(pImgDest);             // need a copy
            
            // make sure we are inside the bounds
            AdjustImageOverlap(GetImage(pImgDest)->GetSize(), GetImage(pImgSrc)->GetSize(), p, r);

            Point i,j; Size sz = Size(r.p.x + r.sz.x, r.p.y + r.sz.y);
            for(i.y = r.p.y, j.y = p.y; i.y < sz.y; ++i.y, ++j.y)
            for(i.x = r.p.x, j.x = p.x; i.x < sz.x; ++i.x, ++j.x)
            {
                Color b = GetImage(pImgDest)->GetPixel(j);
                Color t = GetImage(pImgSrc)->GetPixel(i);
                
                Color b2;
                b2.nTransparent = b.nTransparent;
                b2.R = DrawColorOnto(b.R, t.R, t.nTransparent);
                b2.G = DrawColorOnto(b.G, t.G, t.nTransparent);
                b2.B = DrawColorOnto(b.B, t.B, t.nTransparent);

                if(!(b2 == b))
                    GetImage(pImgDest)->SetPixel(j, b2);
            }

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
            GraphicalInterfaceSimpleException gse("GraphicalInterface", "ImageOnto", "Cannot draw onto an image");
            gse.InheritException(ie);
            throw gse;
        }
    }
    
    template<class ImageHndl>
    ImageHndl  GuiLoadImage(std::istream* pStr, GraphicalInterface<ImageHndl>* pGr)
    {
        PointerAssert<SimpleException>("<global>", "GuiLoadImage", "pStr", pStr);
        PointerAssert<SimpleException>("<global>", "GuiLoadImage", "pGr" , pGr);
        
        // by some weird reason bfType counts as 4 bytes as opposing to 2, giving an invalid format
        // so we have to write everything by hand
        struct BmpFileHdr       
        {
            unsigned short  bfType;
            unsigned long   bfSize;
            unsigned short  bfReserved1;
            unsigned short  bfReserved2;
            unsigned long   bfOffBits;
        };

        struct BmpInfoHdr
        {
            unsigned long   biSize;
            long            biWidth;
            long            biHeight;
            unsigned short  biPlanes;
            unsigned short  biBitCount;
            unsigned long   biCompression;
            unsigned long   biSizeImage;
            long            biXPelsPerMeter;
            long            biYPelsPerMeter;
            unsigned long   biClrUsed;
            unsigned long   biClrImportant;
        };

        BmpFileHdr bmfh;
        
        //pStr->write((char*) &bmfh, sizeof(BmpFileHdr));
        pStr->read((char*) &(bmfh.bfType), 2);
        pStr->read((char*) &(bmfh.bfSize), 4);
        pStr->read((char*) &(bmfh.bfReserved1), 2);
        pStr->read((char*) &(bmfh.bfReserved2), 2);
        pStr->read((char*) &(bmfh.bfOffBits), 4);
            
        if(pStr->fail())
            throw SimpleException("<global>", "GuiLoadImage", "Failed to read from the provided stream (bmp file header)");
        if(bmfh.bfType != 0x4d42)
            throw SimpleException("<global>", "GuiLoadImage", "Bad format of bmp file");

        BmpInfoHdr bmih;

        //pStr->write((char*) &bmih, sizeof(BmpInfoHdr));
        pStr->read((char*) &(bmih.biSize), 4);
        pStr->read((char*) &(bmih.biWidth), 4);
        pStr->read((char*) &(bmih.biHeight), 4);
        pStr->read((char*) &(bmih.biPlanes), 2);
        pStr->read((char*) &(bmih.biBitCount), 2);
        pStr->read((char*) &(bmih.biCompression), 4);
        pStr->read((char*) &(bmih.biSizeImage), 4);
        pStr->read((char*) &(bmih.biXPelsPerMeter), 4);
        pStr->read((char*) &(bmih.biYPelsPerMeter), 4);
        pStr->read((char*) &(bmih.biClrUsed), 4);
        pStr->read((char*) &(bmih.biClrImportant), 4);

        if(pStr->fail())
            throw SimpleException("<global>", "GuiLoadImage", "Failed to read from the provided stream (bmp info header)");

        ImageHndl pRet = pGr->GetBlankImage(Size(bmih.biWidth, bmih.biHeight));
	    
        int nFourByteOffset = (4 - bmih.biWidth*3 % 4)%4;   // extra zeros for 4byte forced width

        Point p;
        for(p.y = bmih.biHeight - 1; p.y >= 0; --p.y)
        {
            for(p.x = 0; p.x < bmih.biWidth; ++p.x)
            {
                Color c;
                pStr->read((char *) &c, 3);
                pGr->GetImage(pRet)->SetPixel(p, c);
            }
            
            char tBuf[4];
            pStr->read(tBuf, nFourByteOffset);  // extra zeros for 4byte forced width
        }

        if(pStr->fail())
        {
            pGr->DeleteImage(pRet);
            throw SimpleException("<global>", "GuiLoadImage", "Failed to read from the provided stream (bmp data)");
        }
        
        return pRet;
    }

    template<class ImageHndl>
    void SimpleGraphicalInterface<ImageHndl>::DeleteImage(unsigned pImg)
    {
        pGr->DeleteImage(vStorage.at(pImg));
        lsFree.push_back(pImg);
    }

    template<class ImageHndl>
    unsigned SimpleGraphicalInterface<ImageHndl>::CopyImage(unsigned pImg)
    {
        ImageHndl pImgC = pGr->CopyImage(vStorage.at(pImg));
        
        if(lsFree.empty())
        {
            vStorage.push_back(pImgC);
            return unsigned(vStorage.size() - 1);
        }
        unsigned pIntImg = lsFree.front();
        lsFree.pop_front();
        vStorage[pIntImg] = pImgC;
        return pIntImg;
    }

    template<class ImageHndl>
    Image* SimpleGraphicalInterface<ImageHndl>::GetImage(unsigned pImg)
    {
        return vStorage.at(pImg);
    }

    template<class ImageHndl>
    unsigned SimpleGraphicalInterface<ImageHndl>::GetBlankImage(Size sz)
    {
        ImageHndl pImg = pGr->GetBlankImage(sz);

        if(lsFree.empty())
        {
            vStorage.push_back(pImg);
            return unsigned(vStorage.size() - 1);
        }
        unsigned pIntImg = lsFree.front();
        lsFree.pop_front();
        vStorage[pIntImg] = pImg;
        return pIntImg;
    }
    
    template<class ImageHndl>
    unsigned SimpleGraphicalInterface<ImageHndl>::LoadImage(std::string sFileName)
    {
        ImageHndl pImg = pGr->LoadImage(sFileName);

        if(lsFree.empty())
        {
            vStorage.push_back(pImg);
            return unsigned(vStorage.size() - 1);
        }
        unsigned pIntImg = lsFree.front();
        lsFree.pop_front();
        vStorage[pIntImg] = pImg;
        return pIntImg;
    }
    
    template<class ImageHndl>
    void SimpleGraphicalInterface<ImageHndl>::SaveImage
        (std::string sFileName, unsigned pImg)
    {pGr->SaveImage(sFileName, vStorage.at(pImg));}

    template<class ImageHndl>
    void SimpleGraphicalInterface<ImageHndl>::DrawImage
        (Point p, unsigned pImg, bool bRefresh = true)
    {pGr->DrawImage(p, vStorage.at(pImg), bRefresh);}

    template<class ImageHndl>
    void SimpleGraphicalInterface<ImageHndl>::DrawImage
        (Point p, unsigned pImg, Rectangle r, bool bRefresh = true)
    {pGr->DrawImage(p, vStorage.at(pImg), r, bRefresh);}


    template<class ImageHndl>
    void SimpleGraphicalInterface<ImageHndl>::DrawRectangle
        (Rectangle p, Color c, bool bRedraw = true)
    {pGr->DrawRectangle(p, c, bRedraw);}

    template<class ImageHndl>
    void SimpleGraphicalInterface<ImageHndl>::RectangleOnto
        (unsigned pImg, Rectangle p, Color c)
    {pGr->RectangleOnto(vStorage.at(pImg), p, c);}

    template<class ImageHndl>
    void SimpleGraphicalInterface<ImageHndl>::Refresh(Rectangle r)
    {pGr->Refresh(r);}

    template<class ImageHndl>
    void SimpleGraphicalInterface<ImageHndl>::RefreshAll()
    {pGr->RefreshAll();}
        
    template<class ImageHndl>
    void SimpleGraphicalInterface<ImageHndl>::ImageOnto
        (unsigned pImgDest, Point p, unsigned pImgSrc, Rectangle r)
    {pGr->ImageOnto(vStorage.at(pImgDest), p, vStorage.at(pImgSrc), r);}
}



#endif //GUIGEN_ALREADY_INCLUDED_GUI_0308
