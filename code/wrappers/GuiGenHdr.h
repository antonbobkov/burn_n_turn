/*
 * GuiGenHdr.h - Gui namespace: graphics primitives and drawing abstraction.
 * Declares: Color, Point, fPoint, Size, Rectangle; Matrix/Image/GraphicalInterface
 * exceptions; Image and GraphicalInterface<ImageHndl>; SimpleGraphicalInterface,
 * CameraControl, Scale; FontWriter, MouseTracker; helpers (InsideRectangle, etc.).
 */

#include <string>
#include <vector>
#include <list>
#include <stdlib.h>
#include <cmath>

#include <fstream>

#include "General.h"
#include "SmartPointer.h"

namespace Gui
{
    typedef int Crd;
    typedef unsigned char Byte;

    /* RGBA-style color: B, G, R bytes and transparency (0 = fully transparent). */
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
        if (c1.nTransparent != 0 && c2.nTransparent == 0)
            return false;
        if (c1.nTransparent == 0 && c2.nTransparent != 0)
            return false;
        return (c1.B == c2.B) && (c1.G == c2.G) && (c1.R == c2.R);
    }

    inline bool operator < (const Color& c1, const Color& c2) 
    {
        if(c1 == c2)
			return false;

		if(c1.R < c2.R)
			return true;
		if(c1.R > c2.R)
			return false;
		if(c1.G < c2.G)
			return true;
		if(c1.G > c2.G)
			return false;
		if(c1.B < c2.B)
			return true;
		//if(c1.B > c2.B)
			return false;
    }
	
	std::string ColorToString(Color c); // for output

    /* 2D integer point (Crd x, y) with +=, -=, *= and free operators +, -, *, ==, !=. */
    struct Point
    {
        Crd x, y;

        Point():x(0), y(0){}
        Point(Crd x_, Crd y_):x(x_), y(y_){}

        Point& operator += (const Point& p){x += p.x; y += p.y; return *this;}
        Point& operator -= (const Point& p){x -= p.x; y -= p.y; return *this;}
        Point& operator *= (Crd c){x *= c; y *= c; return *this;}
    };

    inline Point operator - (const Point& p) {return Point(-p.x, -p.y);}
    inline Point operator + (const Point& p1, const Point& p2) {return Point(p1) += p2;}
    inline Point operator - (const Point& p1, const Point& p2) {return Point(p1) -= p2;}
    inline bool operator == (const Point& p1, const Point& p2) {return p1.x == p2.x && p1.y == p2.y;}
    inline bool operator != (const Point& p1, const Point& p2) {return p1.x != p2.x || p1.y != p2.y;}

    inline Point operator * (const Point& p, Crd c) {return Point(p) *= c;}

    inline std::ostream& operator << (std::ostream& ofs, Point f){return ofs << f.x << " " << f.y;}
    inline std::istream& operator >> (std::istream& ifs, Point& f){return ifs >> f.x >> f.y;}

    /* 2D float point; Normalize(), Length(), conversion to/from Point. */
    struct fPoint
    {
        float x,y;

        fPoint():x(0), y(0){}
        fPoint(double x_, double y_):x(float(x_)), y(float(y_)){}

        fPoint(Point p): x(float(p.x)), y(float(p.y)){}

        Point ToPnt() const {return Point(int(x), int(y));}

        void Normalize(float f = 1);

        float Length() const {return sqrt(x*x + y*y);}
    };

    inline fPoint& operator += (fPoint& f1, const fPoint& f2){f1.x += f2.x; f1.y += f2.y; return f1;}
    inline fPoint& operator -= (fPoint& f1, const fPoint& f2){f1.x -= f2.x; f1.y -= f2.y; return f1;}
    inline fPoint operator + (const fPoint& f1, const fPoint& f2){fPoint f(f1); return f += f2;}
    inline fPoint operator - (const fPoint& f1, const fPoint& f2){fPoint f(f1); return f -= f2;}
    inline bool operator == (const fPoint& f1, const fPoint& f2){return (f1.x == f2.x) && (f1.y == f2.y);}
    inline bool operator != (const fPoint& f1, const fPoint& f2){return (f1.x != f2.x) || (f1.y != f2.y);}
    inline fPoint& operator /= (fPoint& f1, float f){f1.x /= f; f1.y /= f; return f1;}
    inline fPoint& operator *= (fPoint& f1, float f){f1.x *= f; f1.y *= f; return f1;}
    inline fPoint operator / (const fPoint& f1, float f){fPoint ff(f1); return ff /= f;}
    inline fPoint operator * (const fPoint& f1, float f){fPoint ff(f1); return ff *= f;}

	inline float Dot(const fPoint& f1, const fPoint& f2){return f1.x*f2.x + f1.y*f2.y;}

    inline std::ostream& operator << (std::ostream& ofs, fPoint f){ofs << f.x << " " << f.y; return ofs;}
    inline std::istream& operator >> (std::istream& ifs, fPoint& f){ifs >> f.x >> f.y; return ifs;}

    /* 2D extent (Crd x, y); Area() = x * y. */
    struct Size
    {
        Crd x, y;

        Size():x(0), y(0){}
        Size(Crd x_, Crd y_):x(x_), y(y_){}

        Crd Area() const {return x * y;}
    };

    inline bool operator == (const Size& p1, const Size& p2) {return p1.x == p2.x && p1.y == p2.y;}
    inline bool operator != (const Size& p1, const Size& p2) {return p1.x != p2.x || p1.y != p2.y;}

    inline std::ostream& operator << (std::ostream& ofs, Size s){return ofs << s.x << " " << s.y;}
    inline std::istream& operator >> (std::istream& ifs, Size& s){return ifs >> s.x >> s.y;}

    /* Axis-aligned rectangle (Point p, Size sz); normalizes to non-negative size. */
    struct Rectangle
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

	inline bool operator == (const Rectangle& r1, const Rectangle& r2) {return r1.p == r2.p && r1.sz == r2.sz;}
    inline bool operator != (const Rectangle& r1, const Rectangle& r2) {return !(r1 == r2);}

	inline std::ostream& operator << (std::ostream& ofs, Rectangle r)
        {return ofs << r.p.x << " " << r.p.y << " " << r.sz.x << " " << r.sz.y;}
    inline std::istream& operator >> (std::istream& ifs, Rectangle& r)
        {return ifs >> r.p.x >> r.p.y >> r.sz.x >> r.sz.y;}

    bool InsideRectangle(Rectangle r, Point p);   // is the point inside the rectangle?

    Point Center(Rectangle r);
    Point RandomPnt(Rectangle r);

    Rectangle operator + (const Rectangle& r, const Point& p);    // displace
    Rectangle operator + (const Rectangle& r1, const Rectangle& r2);

    Rectangle Intersect(const Rectangle& r1, const Rectangle& r2);    // return intersection of two rectangles
    std::string RectangleToString(Rectangle r); // for output

    /* Size and Point used to describe matrix/rectangle out-of-range errors. */
    struct MatrixErrorInfo
    {
        Size sz;
        Point p;

        MatrixErrorInfo(Size sz_, Point p_)
            :sz(sz_), p(p_){}

        std::string GetErrorMessage() const;
    };

    /* MyException for matrix/rectangle out-of-range; message from MatrixErrorInfo. */
    class MatrixException: public MyException
    {
    public:
        MatrixErrorInfo inf;

        MatrixException(crefString strClsName_, crefString strFnName_, Size sz, Point p)
            :MyException("MatrixException", strClsName_, strFnName_), inf(sz, p){}

        MatrixException(crefString strClsName_, crefString strFnName_, MatrixErrorInfo inf_)
            :MyException("MatrixException", strClsName_, strFnName_), inf(inf_){}

        /*virtual*/ std::string GetErrorMessage() const {return inf.GetErrorMessage();}
    };

    /* Per-pixel transparency grid (Size + vector<Byte>); Set/Get(Point), Safe variants throw. */
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

    /* Base exception for image-related errors (name, class, function). */
    class ImageException: public MyException
    {
    public:
        ImageException(crefString strExcName_, crefString strClsName_, crefString strFnName_)
            :MyException(strExcName_, strClsName_, strFnName_){}
    };

    /* ImageException for out-of-range access; message from MatrixErrorInfo. */
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

    /* ImageException for null pointer; message includes pointer name. */
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

    /* Base exception for graphical-interface errors (name, class, function). */
    class GraphicalInterfaceException: public MyException
    {
    public:
        GraphicalInterfaceException(crefString strExcName_, crefString strClsName_, crefString strFnName_)
            :MyException(strExcName_, strClsName_, strFnName_){}
    };

    /* GraphicalInterfaceException with a single problem string as message. */
    class GraphicalInterfaceSimpleException: public GraphicalInterfaceException
    {
        std::string strProblem;
    public:
        GraphicalInterfaceSimpleException(crefString strClsName_, crefString strFnName_, crefString strProblem_)
            :GraphicalInterfaceException("GraphicalInterfaceSimpleException", strClsName_, strFnName_), strProblem(strProblem_){}

        /*virtual*/ std::string GetErrorMessage() const {return strProblem;}
    };

    /* GraphicalInterfaceException for null pointer; message includes pointer name. */
    class NullPointerGIException: public GraphicalInterfaceException
    {
    public:
        std::string strPntName;

        NullPointerGIException(crefString strClsName_, crefString strFnName_, crefString strPntName_)
            :GraphicalInterfaceException("NullPointerGIException", strClsName_, strFnName_), strPntName(strPntName_){}

        /*virtual*/ std::string GetErrorMessage() const {return "Null pointer passed for " + strPntName;}
    };

    /* Abstract drawing API: create/load/save images, draw images/rectangles,
     * refresh. ImageHndl is backend-specific (e.g. raw pointer or Index). */
    template<class ImageHndl>
    class GraphicalInterface: virtual public SP_Info
    {
    public:
        virtual ~GraphicalInterface(){}

        virtual void DeleteImage(ImageHndl pImg)=0;
        virtual Image* GetImage(ImageHndl pImg) const =0;
        virtual ImageHndl CopyImage(ImageHndl pImg);

        virtual ImageHndl FlipImage(ImageHndl pImg, bool bHorizontal = true);
        virtual ImageHndl ScaleImage(ImageHndl pImg, unsigned nScale);

        virtual ImageHndl GetBlankImage(Size sz)=0;                            // create blank image of specified size
        
        virtual ImageHndl LoadImage(std::string sFileName);                    // loads image from file
        virtual void       SaveImage(std::string sFileName, ImageHndl pImg);   // save image to file

        virtual void DrawImage(Point p, ImageHndl pImg, bool bRefresh = true); // draws an image
        virtual void DrawImage(Point p, ImageHndl pImg, Rectangle r, bool bRefresh = true)=0;  // draws part of the image

        virtual void DrawRectangle(Rectangle p, Color c, bool bRedraw = true)=0;    // draws rectangle onto a screen
        virtual void RectangleOnto(ImageHndl pImg, Rectangle p, Color c);          // draws rectangle onto some image

        //virtual void Refresh(Rectangle r)=0;                                    // refreshes part of the screen
        virtual void RefreshAll()=0;                                            // refreshes all screen

        // draws one image onto another (it is okay for one image not to be fully inside another)
        // p or r.p can have negative values
        virtual void ImageOnto(ImageHndl pImgDest, Point p, ImageHndl pImgSrc, Rectangle r);  
    };

    /* GraphicalInterface<Index> wrapper: delegates to a real GI, uses IndexKeeper
     * and IndexRemover so images are freed when Index is destroyed. */
    template<class ImageHndl>
    struct SimpleGraphicalInterface: public GraphicalInterface<Index>, public IndexRemover
    {
        // okay, no need for SSP as GraphicalInterface is not going to point to us
        SP<GraphicalInterface<ImageHndl> > pGr;

        IndexKeeper<ImageHndl> kp;

        ImageHndl Get(const Index& n) const;
        Index New(ImageHndl pImg);
        
        /*virtual*/ void DeleteIndex(const Index &pImg)
        {
            pGr->DeleteImage(Get(pImg));
            kp.FreeElement(pImg.GetIndex());
        }
        
    public:
        
        SimpleGraphicalInterface(SP<GraphicalInterface<ImageHndl> > pGr_):pGr(pGr_){}
        
        /*virtual*/ void DeleteImage(Index pImg);
        /*virtual*/ Image* GetImage(Index pImg) const;
        /*virtual*/ Index CopyImage(Index pImg);

        /*virtual*/ Index GetBlankImage(Size sz);                            // create blank image of specified size
        
        /*virtual*/ Index LoadImage(std::string sFileName);                    // loads image from file
        /*virtual*/ void     SaveImage(std::string sFileName, Index pImg);     // save image to file

        /*virtual*/ void DrawImage(Point p, Index pImg, bool bRefresh = true); // draws an image
        /*virtual*/ void DrawImage(Point p, Index pImg, Rectangle r, bool bRefresh = true);  // draws part of the image

        /*virtual*/ void DrawRectangle(Rectangle p, Color c, bool bRedraw = true);    // draws rectangle onto a screen
        /*virtual*/ void RectangleOnto(Index pImg, Rectangle p, Color c);          // draws rectangle onto some image

        /*virtual*/ void Refresh(Rectangle r);                                    // refreshes part of the screen
        /*virtual*/ void RefreshAll();                                            // refreshes all screen

        // draws one image onto another (it is okay for one image not to be fully inside another)
        // p or r.p can have negative values
        /*virtual*/ void ImageOnto(Index pImgDest, Point p, Index pImgSrc, Rectangle r);  
    };

    /* View transform: offset, zoom factor, and box size for coordinate conversion. */
    struct Scale
    {
        Point pOffset;
        float fZoom;
        Size szBox;

        Scale(float fZoom_ = 1.F, Point pOffset_ = Point(), Size szBox_ = Size()):pOffset(pOffset_), fZoom(fZoom_), szBox(szBox_){}
        Scale(float fZoom_, Rectangle r):pOffset(r.p), fZoom(fZoom_), szBox(r.sz){}
    };

    /* View/camera: stack of Scale (Push/Pop), translate/zoom/SetBox, and
     * coordinate conversion (toR/fromR, toF/fromF). Draws through a GI with scaling. */
    template<class ImageHndl>
    class CameraControl
    {
        std::list<Scale> lsScales;
        Scale sCurr;
    public:
        SP< GraphicalInterface<ImageHndl> > pGr;

        CameraControl(SP< GraphicalInterface<ImageHndl> > pGr_ = 0, Scale s = Scale())
            :pGr(pGr_), sCurr(s){}

        Point GetOffset() {return sCurr.pOffset;}
        float GetZoomF()  {return sCurr.fZoom;}
        int   GetZoom()   {return int(sCurr.fZoom);}
        Size  GetBox()    {return sCurr.szBox;}

        void Push(){lsScales.push_back(sCurr);}
        void Pop();

        void Translate(Point p);
        void BX_Translate(Point p);
        void LU_Translate(Point p);
        void RB_Translate(Point p);

        void Translate(fPoint p){Translate(fromF(p));}
        void BX_Translate(fPoint p){BX_Translate(fromF(p));}
        void LU_Translate(fPoint p){LU_Translate(fromF(p));}
        void RB_Translate(fPoint p){RB_Translate(fromF(p));}

        void Zoom(int z);
        void Zoom(float z);
        void SetBox(Size sz);
        
        ImageHndl LoadImage(std::string sFileName); 
        
        void DrawImage(Point p, ImageHndl pImg, bool bCentered = false, bool bRefresh = false); 
        void DrawImage(fPoint p, ImageHndl pImg, bool bCentered = false, bool bRefresh = false);

        void DrawRectangle(Rectangle p, Color c, bool bRedraw = false);

        Point fromF(fPoint p);
        fPoint toF (Point p);

        Point toR  (Point p);
        Point fromR(Point p);

        Size toR   (Size sz); 
        Size fromR (Size sz);
        
        Rectangle toR  (Rectangle r);
        Rectangle fromR(Rectangle r);
    };

    /* Renders text using a font (FilePath + name), bitmap symbols, and a
     * GraphicalInterface<Index>; GetSize, DrawWord/DrawColorWord, Recolor, SetGap. */
    struct FontWriter: virtual public SP_Info
    {
        std::vector<int> vImgIndx;
        std::vector<Index> vImg;
        SP<GraphicalInterface<Index> > pGr;

        Size szSymbol;
        Color clSymbol;
        int nGap;

        FontWriter(FilePath& fp, std::string sFont, SP<GraphicalInterface<Index> > pGr_, unsigned nZoom = 1);

        Size GetSize(std::string s);

        std::string GetNumber(int n, unsigned nDigits = 0);

        // NOTE: bug - colored words have non-transparent background
        void DrawColorWord(std::string s, Point p, Color c = Color(0,0,0,0),
            bool bCenter = false, bool bRefresh = false);

        void DrawWord(std::string s, Point p, bool bCenter = false, bool bRefresh = false)
            {DrawColorWord(s, p, Color(0,0,0,0), bCenter, bRefresh);}

        void Recolor(Color c);
        void SetGap(int nG);
    };

    template <class T>
    void ConvertImage(SP<GraphicalInterface<T> > pGr, std::string strImg, std::string strExtFrom, std::string strExtTo)
    {
       T img = pGr->LoadImage(strImg + "." + strExtFrom);
       pGr->SaveImage(strImg + "." + strExtTo, img);
        pGr->DeleteImage(img);
    }

    /* Tracks mouse position and relative movement: OnMouse(Point), GetRelMovement(). */
    struct MouseTracker
    {
        Point pLastRead;
        Point pCurPos;

        bool bInitialized;

        MouseTracker():bInitialized(false){}

        void OnMouse(Point p)
        {
            pCurPos = p;
            if(!bInitialized)
            {
                bInitialized = true;
                pLastRead = p;
            }
        }

        Point GetRelMovement()
        {
            if(!bInitialized)
                return Point(0,0);

            Point pRet = pCurPos - pLastRead;
            pLastRead = pCurPos;
            return pRet;
        }
    };
}