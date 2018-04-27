#ifndef GUI_MFC_INCLUDE_GUARD_09_03_10_09_43_PM
#define GUI_MFC_INCLUDE_GUARD_09_03_10_09_43_PM

#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#include <afxwin.h>

#include "GuiGen.h"
#include "MessageWriter.h"

namespace Gui
{
    class MfcGraphicalInterface;

    class CDibException: public MyException
    {
        std::string strProblem;
    public:
        CDibException(crefString strFnName_, crefString strProblem_)
            :MyException("CDibException", "CDib", strFnName_), strProblem(strProblem_){}

        /*virtual*/ std::string GetErrorMessage() const {return strProblem;}
    };

    // this is a shell class for the bmp-type image for MFC (and Windows in general)
    class CDib
    {
	    enum Alloc {noAlloc, crtAlloc, heapAlloc};
    public:
	    LPVOID m_lpvColorTable;
	    HBITMAP m_hBitmap;
	    LPBYTE m_lpImage;  // starting address of DIB bits
	    LPBITMAPINFOHEADER m_lpBMIH; //  buffer containing the BITMAPINFOHEADER
    private:
	    HGLOBAL m_hGlobal; // For external windows we need to free;
	                       //  could be allocated by this class or allocated externally
	    Alloc m_nBmihAlloc;
	    Alloc m_nImageAlloc;
	    DWORD m_dwSizeImage; // of bits -- not BITMAPINFOHEADER or BITMAPFILEHEADER
	    int m_nColorTableEntries;
    	
	    HANDLE m_hFile;
	    HANDLE m_hMap;
	    LPVOID m_lpvFile;
	    HPALETTE m_hPalette;
    public:
	    CDib();
	    CDib(CDib &d){};

        CDib* Copy();

        void CreateEmptyImage24(CSize size);

	    CDib(CSize size, int nBitCount);	// builds BITMAPINFOHEADER
	    ~CDib();
	    int GetSizeImage() {return m_dwSizeImage;}
	    int GetSizeHeader()
		    {return sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries;}
	    CSize GetDimensions();
	    BOOL AttachMapFile(const char* strPathname, BOOL bShare = FALSE);
	    BOOL CopyToMapFile(const char* strPathname);
	    BOOL AttachMemory(LPVOID lpvMem, BOOL bMustDelete = FALSE, HGLOBAL hGlobal = NULL);
	    BOOL Draw(CDC* pDC, CPoint origin, CSize size);  // until we implemnt CreateDibSection
        BOOL Draw(CDC* pDC, CPoint origin, CSize size, CPoint from, CSize part);
	    HBITMAP CreateSection(CDC* pDC = NULL);
	    UINT UsePalette(CDC* pDC, BOOL bBackground = FALSE);
	    BOOL MakePalette();
	    BOOL SetSystemPalette(CDC* pDC);
	    BOOL Compress(CDC* pDC, BOOL bCompress = TRUE); // FALSE means decompress
	    HBITMAP CreateBitmap(CDC* pDC);
	    BOOL Read(CFile* pFile);
	    BOOL Read(std::istream* pIstr);
	    BOOL ReadSection(CFile* pFile, CDC* pDC = NULL);
	    BOOL Write(CFile* pFile);
        BOOL Write(std::ostream* pOstr);
	    void Serialize(CArchive& ar);
	    void Empty();
    private:
	    void DetachMapFile();
	    void ComputePaletteSize(int nBitCount);
	    void ComputeMetrics();
    };

    // MFC impelementation of GUI Image class
    class MfcImage: public Image
    {
        friend class MfcGraphicalInterface;     // implemented by MfcGraphicalInterface
        
        CDib* pImg;
        TransparencyGrid tg;        // mfc doesn't support transparency natively

    public:

        MfcImage(CDib* pImg_);
        ~MfcImage(){delete pImg;}
        
        /*virtual*/ MfcImage* Copy() const;
        
        /*virtual*/ void SetPixel(Point p, const Color& c)
        {
            Size sz = GetSize();

            int ps = (3*sz.x + 3 - (3*sz.x - 1)%4) * (sz.y - p.y - 1) + p.x * 3;
            
            pImg->m_lpImage[ps + 2] = c.R;
            pImg->m_lpImage[ps + 1] = c.G;
            pImg->m_lpImage[ps + 0] = c.B;

            tg.Set(p, c.nTransparent);
        }

        /*virtual*/ Color GetPixel(Point p) const
        {
            Size sz = GetSize();

            int ps = (3*sz.x + 3 - (3*sz.x - 1)%4) * (sz.y - p.y - 1) + p.x * 3;

            Color ret;
            
            ret.R = pImg->m_lpImage[ps + 2];
            ret.G = pImg->m_lpImage[ps + 1];
            ret.B = pImg->m_lpImage[ps + 0];

            ret.nTransparent = tg.Get(p);

            return ret;
        }
    };

    class MfcGraphicalInterfaceException: public GraphicalInterfaceException
    {
    public:
        MfcGraphicalInterfaceException(crefString strExcName_, crefString strClsName_, crefString strFnName_)
            :GraphicalInterfaceException(strExcName_, strClsName_, strFnName_){}

        static std::string GetMfcError();
    };

    class SimpleMGIException: public MfcGraphicalInterfaceException
    {
        std::string strProblem;
    public:
        SimpleMGIException(crefString strFnName_, crefString strProblem_)
            :MfcGraphicalInterfaceException("SimpleMGIException", "MfcGraphicalInterface", strFnName_), strProblem(strProblem_){}

        /*virtual*/ std::string GetErrorMessage() const {return strProblem;}
    };

    class NullPointerMGIException: public MfcGraphicalInterfaceException
    {
    public:
        std::string strPntName;

        NullPointerMGIException(crefString strClsName_, crefString strFnName_, crefString strPntName_)
            :MfcGraphicalInterfaceException("NullPointerMGIException", strClsName_, strFnName_), strPntName(strPntName_){}

        /*virtual*/ std::string GetErrorMessage() const {return "Null pointer passed for " + strPntName;}
    };

    // helper class for MfcGraphicalInterface (as of right now equivalent to Rectangle)
    struct ImageToDraw
    {
        Rectangle r;

        ImageToDraw();
        ImageToDraw(Rectangle r_):r(r_){}
    };

    // MFC implementation of GUI GraphicalInterface
    // Note: use after window finalized its construction ie after OnInitialUpdate function was finished
    class MfcGraphicalInterface: public GraphicalInterface<MfcImage*>
    {
        std::vector<ImageToDraw> vecImg;    // rectangles to redraw
        Crd nArea;          // redrawing area - redraw all if more than total screen area

        Size szScr;

        CView* pWnd;        // need to have a pointer to window

        //CDC* pMem;
        //CBitmap* pBitmap;

        MfcImage* pScreen;  // all screen

    public:

        MfcGraphicalInterface(Size sz, CView* pWnd_);
        ~MfcGraphicalInterface();

        /*virtual*/ void DeleteImage(MfcImage* pImg);
        /*virtual*/ Image* GetImage(MfcImage* pImg) const;

        /*virtual*/ MfcImage* GetBlankImage(Size sz);

        /*virtual*/ MfcImage* LoadImage(std::string sFileName);
        /*virtual*/ void      SaveImage(std::string sFileName, MfcImage* pImg);
        
        /*virtual*/ void      DrawImage(Point p, MfcImage* pImg, Rectangle r, bool bRefresh = true);
        
        /*virtual*/ void DrawRectangle(Rectangle p, Color c, bool bRedraw = true);
        /*virtual*/ void RectangleOnto(MfcImage* pImg, Rectangle p, Color c);

        /*virtual*/ void Refresh(Rectangle r);
        /*virtual*/ void RefreshAll();

        /*virtual*/ void ImageOnto(MfcImage* pImgDest, Point p, MfcImage* pImgSrc, Rectangle r);

        void OnInitialUpdate();     // initialization (not used as of right now)
        void OnPaint(CDC* pDC);     // could be also called from OnDraw (or OnPaint)
    
    private:
        void Draw(CDC* pDC);        // internal drawing function (called from OnPaint); does actual drawing from vecImg
    };

    class MfcMessenger: public MessageWriter
    {
        std::string strFile;
    public:
        MfcMessenger(std::string strFile_):strFile(strFile_){std::ofstream ofs(strFile.c_str());}

        /*virtual*/ void Write(WriteType wt, std::string strMsg);
    };
}

#endif // GUI_MFC_INCLUDE_GUARD_09_03_10_09_43_PM