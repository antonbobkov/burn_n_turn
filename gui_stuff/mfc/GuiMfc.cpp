#include "GuiMfc.h"

namespace Gui
{
    inline Size MfcConvert(CSize sz)
    {
        return Size(sz.cx, sz.cy);
    }

    inline CSize MfcConvert(Size sz)
    {
        return CSize(sz.x, sz.y);
    }

    inline Point MfcConvert(CPoint p)
    {
        return Point(p.x, p.y);
    }

    inline CPoint MfcConvert(Point p)
    {
        return CPoint(p.x, p.y);
    }

    inline CRect MfcConvert(Rectangle r)
    {
        return CRect(r.Left(), r.Top(), r.Right(), r.Bottom());
    }

    inline Rectangle MfcConvert(CRect r)
    {
        return Rectangle(r.left, r.top, r.right, r.bottom);
    }

    MfcImage::MfcImage(CDib* pImg_)
    :Image(MfcConvert(PointerAssert<ImageNullException>("MfcImage", "constructor", "pImg_", pImg_)->GetDimensions())),
    pImg(pImg_), tg(MfcConvert(pImg_->GetDimensions())){}


    MfcImage* MfcImage::Copy() const
    {
        MfcImage* pRet = new MfcImage(pImg->Copy());
        pRet->tg = tg;
        return pRet;
    }

    std::string MfcGraphicalInterfaceException::GetMfcError()
    {
        std::ostringstream ostr;
        ostr << " (MFC error: " << ::GetLastError() << ")";
        return ostr.str();
    }



    MfcGraphicalInterface::MfcGraphicalInterface(Size sz_, CView* pWnd_):szScr(sz_), pWnd(pWnd_), nArea(0)
    {
        PointerAssert<NullPointerMGIException>("MfcGraphicalInterface", "constructor", "pWnd_", pWnd_);

        pScreen = GetBlankImage(szScr);

        //pMem = new CDC();
        //pBitmap = new CBitmap();
    }

    MfcGraphicalInterface::~MfcGraphicalInterface()
    {
        delete pScreen;

        //delete pMem;
        //delete pBitmap;
    }

    void MfcGraphicalInterface::DeleteImage(MfcImage* pImg)
    {
        delete pImg;
    }

    Image* MfcGraphicalInterface::GetImage(MfcImage* pImg) const
    {
        return pImg;
    }

    MfcImage* MfcGraphicalInterface::GetBlankImage(Size sz)
    {
        CDib* pImg = new CDib();
        pImg->CreateEmptyImage24(CSize(sz.x, sz.y));
        return new MfcImage(pImg);
    }

    MfcImage* MfcGraphicalInterface::LoadImage(std::string sFileName)
    {
        std::ifstream f(sFileName.c_str(), std::ios_base::in | std::ios_base::binary);
        if(f.fail())
            throw SimpleMGIException("LoadImage", "Cannot load image - cannot open file " + sFileName);

        CDib* pImg = new CDib();
        
        try{pImg->Read(&f);}
        catch(CDibException& de)
        {
            SimpleMGIException me("LoadImage",
            "Cannot load bmp image from " + sFileName + MfcGraphicalInterfaceException::GetMfcError());
            me.InheritException(de);
            throw me;
        }

        if(pImg->m_lpBMIH->biBitCount != 24)
        {
            throw SimpleMGIException("LoadImage", "Image in " + sFileName + " has bit depth " + S(pImg->m_lpBMIH->biBitCount) 
                + " whereas current version of GenGui only supports 24-bit bmp images");
        }
        
        return new MfcImage(pImg);
    }

    void MfcGraphicalInterface::SaveImage(std::string sFileName, MfcImage* pImg)
    {
        PointerAssert<NullPointerMGIException>("MfcGraphicalInterface", "SaveImage", "pImg", pImg);

        std::ofstream f(sFileName.c_str(), std::ios_base::out | std::ios_base::binary);
        if(f.fail())
            throw SimpleMGIException("SaveImage", "Cannot save image - cannot open file " + sFileName);

        try{pImg->pImg->Write(&f);}
        catch(CDibException& de)
        {
            SimpleMGIException me("SaveImage",
            "Cannot save image to " + sFileName + MfcGraphicalInterfaceException::GetMfcError());
            me.InheritException(de);
            throw me;
        }
    }

    void MfcGraphicalInterface::DrawImage(Point p, MfcImage* pImg, Rectangle r, bool bRefresh/* = true*/)
    {
        PointerAssert<NullPointerMGIException>("MfcGraphicalInterface", "DrawImage", "pImg", pImg);

        AdjustImageOverlap(szScr, pImg->GetSize(), p, r);

        //DrawImageOntoCDC(pMem, p, pScreen, Rectangle(p, r.sz));
        //pImg->pImg->Draw(pMem, MfcConvert(p - r.p), MfcConvert(pImg->GetSize()));
        //DrawImageOntoCDC(pMem, p, pImg, r);
        ImageOnto(pScreen, p, pImg, r);
        //if(pScreen->pImg->Draw(pMem, MfcConvert(p), MfcConvert(r.sz), MfcConvert(p), MfcConvert(r.sz)) == FALSE)
        //    throw SimpleMGIException("DrawImage", "Cannot draw image");

        if(bRefresh)
        {
            if(nArea < szScr.x*szScr.y)
            {
                vecImg.push_back(ImageToDraw(Rectangle(p, r.sz)));
                nArea += r.Area();
            }

            CRect r(0,0,szScr.x,szScr.y);
            pWnd->InvalidateRect(&r, FALSE);
        }
    }

    void MfcGraphicalInterface::DrawRectangle(Rectangle p, Color c, bool bRedraw/* = true*/)
    {
        p = Intersect(p, Rectangle(szScr));
        RectangleOnto(pScreen, p, c);
        //if(pScreen->pImg->Draw(pMem, MfcConvert(p.p), MfcConvert(p.sz), MfcConvert(p.p), MfcConvert(p.sz)) == FALSE)
        //    throw SimpleMGIException("DrawRectangle", "Cannot draw image");
        
        if(bRedraw)
        {
            if(nArea < szScr.x*szScr.y)
            {
                vecImg.push_back(ImageToDraw(p));
                nArea += p.Area();
            }

            CRect r(0,0,szScr.x,szScr.y);
            pWnd->InvalidateRect(&r, FALSE);
        }
    }

    void MfcGraphicalInterface::RectangleOnto(MfcImage* pImg, Rectangle p, Color c)
    {
        PointerAssert<NullPointerMGIException>("MfcGraphicalInterface", "RectangleOnto", "pImg", pImg);

        Size sz1 = pImg->GetSize();
        p = Intersect(p, Rectangle(sz1));
        
        Point i; Size sz(p.p.x + p.sz.x, p.p.y + p.sz.y);
        unsigned n;
        LPBYTE* arr1 = &pImg->pImg->m_lpImage;
        for(i.y = p.p.y; i.y < sz.y; ++i.y)
        for(i.x = p.p.x, n = GetBmpPos(i, sz1); i.x < sz.x; ++i.x, n+=3)
        {
            pImg->tg.Set(i, 255);
            (*arr1)[n + 0] = c.B;
            (*arr1)[n + 1] = c.G;
            (*arr1)[n + 2] = c.R;
        }
    }


    void MfcGraphicalInterface::Refresh(Rectangle r)
    {
        vecImg.push_back(ImageToDraw(r));

        //CRect r_(0,0,szScr.x,szScr.y);
        CRect r_(0,0,szScr.x + 1,szScr.y + 1);
        pWnd->InvalidateRect(&r_, FALSE);
    }

    void MfcGraphicalInterface::RefreshAll()
    {
        vecImg.push_back(ImageToDraw(szScr));

        //CRect r(0,0,szScr.x,szScr.y);
        CRect r(0,0,szScr.x + 1,szScr.y + 1);
        pWnd->InvalidateRect(&r, FALSE);
    }

    
    void MfcGraphicalInterface::ImageOnto(MfcImage* pImgDest, Point p, MfcImage* pImgSrc, Rectangle r)
    {
        PointerAssert<NullPointerMGIException>("MfcGraphicalInterface", "ImageOnto", "pImgDest", pImgDest);
        PointerAssert<NullPointerMGIException>("MfcGraphicalInterface", "ImageOnto", "pImgSrc", pImgSrc);

        bool bSelf = (pImgDest == pImgSrc);

        if(bSelf)
            pImgSrc = pImgDest->Copy();

        AdjustImageOverlap(pImgDest->GetSize(), pImgSrc->GetSize(), p, r);

        Point i,j; Size sz(r.p.x + r.sz.x, r.p.y + r.sz.y);
        unsigned n,m;
        Size sz1 = pImgDest->GetSize();
        Size sz2 = pImgSrc->GetSize();
        LPBYTE* arr1 = &pImgDest->pImg->m_lpImage;
        LPBYTE* arr2 = &pImgSrc->pImg->m_lpImage;
        for(i.y = r.p.y, j.y = p.y; i.y < sz.y; ++i.y, ++j.y)
        for(i.x = r.p.x, j.x = p.x, n = GetBmpPos(j, sz1), m = GetBmpPos(i, sz2); i.x < sz.x; ++i.x, ++j.x, n+=3, m+=3)
        {
            float nTopTr = float(pImgSrc->tg.Get(i));
            ExplDrawColorOnto((*arr1)[n + 0], (*arr2)[m + 0], nTopTr);
            ExplDrawColorOnto((*arr1)[n + 1], (*arr2)[m + 1], nTopTr);
            ExplDrawColorOnto((*arr1)[n + 2], (*arr2)[m + 2], nTopTr);
        }

        if(bSelf)
            delete pImgSrc;
    }
    

    /*
    MfcImage* MfcGraphicalInterface::CopyPart(MfcImage* pImg, Rectangle r)
    {
        MfcImage* pRet = GetBlankImage(r.sz);
        ImageOnto(pRet, Point(0,0), pImg, r);
        return pRet;
    }

    void MfcGraphicalInterface::DrawImageOntoCDC(CDC* pDC, Point p, MfcImage* pImg, Rectangle r)
    {
        //AdjustImageOverlap(szScr, pImg->GetSize(), p, r);
        
        //pImg->pImg->Draw(pDC, MfcConvert(p - r.p), MfcConvert(pImg->GetSize()));
        
        /*
        MfcImage* pDr = CopyPart(pImg, r);
        pDr->pImg->Draw(pDC, MfcConvert(p), MfcConvert(r.sz));
        delete pDr;
        /
        
        pImg->pImg->Draw(pDC, MfcConvert(p), MfcConvert(r.sz), MfcConvert(r.p), MfcConvert(r.sz));
    }
    */


    void MfcGraphicalInterface::OnInitialUpdate()
    {
        /*
        if (pMem->GetSafeHdc() == NULL)
        {
            CSize sizeTotal(szScr.x,szScr.y);

            CClientDC dc(pWnd);
	        pWnd->OnPrepareDC(&dc);
	        CRect rectMax(0,0, sizeTotal.cx, sizeTotal.cy);
	        dc.LPtoDP(rectMax);
	        pBitmap->CreateCompatibleBitmap(&dc, rectMax.right, rectMax.bottom);
	        pMem->CreateCompatibleDC(&dc);
	        pMem->SelectObject(pBitmap);
            
            pMem->SelectClipRgn(NULL);
            CBrush backgroundBrush((COLORREF) ::GetSysColor(COLOR_WINDOW));
            pMem->SelectObject(&backgroundBrush);
        }
        */
    }

    void MfcGraphicalInterface::OnPaint(CDC* pDC)
    {
        PointerAssert<NullPointerMGIException>("MfcGraphicalInterface", "OnPaint", "pDC", pDC);
 
        //SaveImage("img\\ScrShot.bmp", pScreen);
        
        CRect rectUpdate;
        pDC->GetClipBox(&rectUpdate);

        //if (rectUpdate == CRect(0,0,szScr.x,szScr.y))
        if (rectUpdate == CRect(0,0,szScr.x + 1,szScr.y + 1))
	        Draw(pDC);
        else
            //DrawImageOntoCDC(pDC, MfcConvert(rectUpdate.TopLeft()), pScreen, MfcConvert(rectUpdate));
            //pDC->BitBlt(rectUpdate.left, rectUpdate.top, rectUpdate.Width(), rectUpdate.Height(), pMem, rectUpdate.left, rectUpdate.top, SRCCOPY);
            pScreen->pImg->Draw(pDC, rectUpdate.TopLeft(), rectUpdate.Size(), rectUpdate.TopLeft(), rectUpdate.Size());
    }

    void MfcGraphicalInterface::Draw(CDC* pDC)
    {
        PointerAssert<NullPointerMGIException>("MfcGraphicalInterface", "Draw", "pDC", pDC);

        if(nArea >= szScr.x*szScr.y)
            //DrawImageOntoCDC(pDC, Point(0,0), pScreen, szScr);
            //pDC->BitBlt(0, 0, szScr.x,szScr.y, pMem, 0, 0, SRCCOPY);
            pScreen->pImg->Draw(pDC, CPoint(0,0), MfcConvert(szScr), CPoint(0,0), MfcConvert(szScr));
        else
            for(size_t i = 0, sz = vecImg.size(); i < sz; ++i)
            {
                Rectangle r = vecImg[i].r;
                
                //pScreen->pImg->Draw(pDC, CPoint(p.x, p.y), CSize(picsz.x, picsz.y), CPoint(p.x, p.y), CSize(picsz.x, picsz.y));
                //pDC->BitBlt(r.p.x, r.p.y, r.sz.x, r.sz.y, pMem, r.p.x, r.p.y, SRCCOPY);
                pScreen->pImg->Draw(pDC, MfcConvert(r.p), MfcConvert(r.sz), MfcConvert(r.p), MfcConvert(r.sz));
            }
                //DrawImageOntoCDC(pDC, vecImg[i].r.p, pScreen, vecImg[i].r);
        
        nArea = 0;
        vecImg.clear();
    }

// ------------------------------------------------------------------------------------------
// CDib stuff (most funtions aren't used)
// ------------------------------------------------------------------------------------------
    
    CDib::CDib()
    {
	    m_hFile = NULL;
	    m_hBitmap = NULL;
	    m_hPalette = NULL;
	    m_nBmihAlloc = m_nImageAlloc = noAlloc;
	    Empty();
    }

    CDib::CDib(CSize size, int nBitCount)
    {
	    m_hFile = NULL;
	    m_hBitmap = NULL;
	    m_hPalette = NULL;
	    m_nBmihAlloc = m_nImageAlloc = noAlloc;
	    Empty();
	    ComputePaletteSize(nBitCount);
	    m_lpBMIH = (LPBITMAPINFOHEADER) new 
		    char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries];
	    m_nBmihAlloc = crtAlloc;
	    m_lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
	    m_lpBMIH->biWidth = size.cx;
	    m_lpBMIH->biHeight = size.cy;
	    m_lpBMIH->biPlanes = 1;
	    m_lpBMIH->biBitCount = nBitCount;
	    m_lpBMIH->biCompression = BI_RGB;
	    m_lpBMIH->biSizeImage = 0;
	    m_lpBMIH->biXPelsPerMeter = 0;
	    m_lpBMIH->biYPelsPerMeter = 0;
	    m_lpBMIH->biClrUsed = m_nColorTableEntries;
	    m_lpBMIH->biClrImportant = m_nColorTableEntries;
	    ComputeMetrics();
	    memset(m_lpvColorTable, 0, sizeof(RGBQUAD) * m_nColorTableEntries);
	    m_lpImage = NULL;  // no data yet
    }

    CDib::~CDib()
    {
	    Empty();
    }

    CSize CDib::GetDimensions()
    {	
	    if(m_lpBMIH == NULL) return CSize(0, 0);
	    return CSize((int) m_lpBMIH->biWidth, (int) m_lpBMIH->biHeight);
    }

    BOOL CDib::AttachMapFile(const char* strPathname, BOOL bShare) // for reading
    {
	    // if we open the same file twice, Windows treats it as 2 separate files
	    // doesn't work with rare BMP files where # palette entries > biClrUsed
	    HANDLE hFile = ::CreateFile(strPathname, GENERIC_WRITE | GENERIC_READ,
		    bShare ? FILE_SHARE_READ : 0,
		    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	    ASSERT(hFile != INVALID_HANDLE_VALUE);
	    DWORD dwFileSize = ::GetFileSize(hFile, NULL);
	    HANDLE hMap = ::CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	    DWORD dwErr = ::GetLastError();
	    if(hMap == NULL) {
		    AfxMessageBox("Empty bitmap file");
		    return FALSE;
	    }
	    LPVOID lpvFile = ::MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0); // map whole file
	    ASSERT(lpvFile != NULL);
	    if(((LPBITMAPFILEHEADER) lpvFile)->bfType != 0x4d42) {
		    AfxMessageBox("Invalid bitmap file");
		    DetachMapFile();
		    return FALSE;
	    }
	    AttachMemory((LPBYTE) lpvFile + sizeof(BITMAPFILEHEADER));
	    m_lpvFile = lpvFile;
	    m_hFile = hFile;
	    m_hMap = hMap;
	    return TRUE;
    }

    BOOL CDib::CopyToMapFile(const char* strPathname)
    {
	    // copies DIB to a new file, releases prior pointers
	    // if you previously used CreateSection, the HBITMAP will be NULL (and unusable)
	    BITMAPFILEHEADER bmfh;
	    bmfh.bfType = 0x4d42;  // 'BM'
	    bmfh.bfSize = m_dwSizeImage + sizeof(BITMAPINFOHEADER) +
			    sizeof(RGBQUAD) * m_nColorTableEntries + sizeof(BITMAPFILEHEADER);
	    // meaning of bfSize open to interpretation
	    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
			    sizeof(RGBQUAD) * m_nColorTableEntries;	
	    HANDLE hFile = ::CreateFile(strPathname, GENERIC_WRITE | GENERIC_READ, 0, NULL,
		    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	    ASSERT(hFile != INVALID_HANDLE_VALUE);
	    int nSize =  sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
				    sizeof(RGBQUAD) * m_nColorTableEntries +  m_dwSizeImage;
	    HANDLE hMap = ::CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, nSize, NULL);
	    DWORD dwErr = ::GetLastError();
	    ASSERT(hMap != NULL);
	    LPVOID lpvFile = ::MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0); // map whole file
	    ASSERT(lpvFile != NULL);
	    LPBYTE lpbCurrent = (LPBYTE) lpvFile;
	    memcpy(lpbCurrent, &bmfh, sizeof(BITMAPFILEHEADER)); // file header
	    lpbCurrent += sizeof(BITMAPFILEHEADER);
	    LPBITMAPINFOHEADER lpBMIH = (LPBITMAPINFOHEADER) lpbCurrent;
	    memcpy(lpbCurrent, m_lpBMIH,
		    sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries); // info
	    lpbCurrent += sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries;
	    memcpy(lpbCurrent, m_lpImage, m_dwSizeImage); // bit image
	    DWORD dwSizeImage = m_dwSizeImage;
	    Empty();
	    m_dwSizeImage = dwSizeImage;
	    m_nBmihAlloc = m_nImageAlloc = noAlloc;
	    m_lpBMIH = lpBMIH;
	    m_lpImage = lpbCurrent;
	    m_hFile = hFile;
	    m_hMap = hMap;
	    m_lpvFile = lpvFile;
	    ComputePaletteSize(m_lpBMIH->biBitCount);
	    ComputeMetrics();
	    MakePalette();
	    return TRUE;
    }

    BOOL CDib::AttachMemory(LPVOID lpvMem, BOOL bMustDelete, HGLOBAL hGlobal)
    {
	    // assumes contiguous BITMAPINFOHEADER, color table, image
	    // color table could be zero length
	    Empty();
	    m_hGlobal = hGlobal;
	    if(bMustDelete == FALSE) {
		    m_nBmihAlloc = noAlloc;
	    }
	    else {
		    m_nBmihAlloc = ((hGlobal == NULL) ? crtAlloc : heapAlloc);
	    }
	    m_lpBMIH = (LPBITMAPINFOHEADER) lpvMem;
	    ComputeMetrics();
	    ComputePaletteSize(m_lpBMIH->biBitCount);
	    m_lpImage = (LPBYTE) m_lpvColorTable + sizeof(RGBQUAD) * m_nColorTableEntries;
	    MakePalette();

        return TRUE;
    }

    UINT CDib::UsePalette(CDC* pDC, BOOL bBackground /* = FALSE */)
    {
	    if(m_hPalette == NULL) return 0;
	    HDC hdc = pDC->GetSafeHdc();
	    ::SelectPalette(hdc, m_hPalette, bBackground);
	    return ::RealizePalette(hdc);
    }

    BOOL CDib::Draw(CDC* pDC, CPoint origin, CSize size)
    {
	    if(m_lpBMIH == NULL) return FALSE;
	    if(m_hPalette != NULL) {
		    ::SelectPalette(pDC->GetSafeHdc(), m_hPalette, TRUE);
	    }
	    pDC->SetStretchBltMode(COLORONCOLOR);
	    ::StretchDIBits(pDC->GetSafeHdc(), origin.x, origin.y, size.cx, size.cy,
		    0, 0, m_lpBMIH->biWidth, m_lpBMIH->biHeight,
		    m_lpImage, (LPBITMAPINFO) m_lpBMIH, DIB_RGB_COLORS, SRCCOPY);
	    return TRUE;
    }

    BOOL CDib::Draw(CDC* pDC, CPoint origin, CSize size, CPoint from, CSize part)
    {
	    if(m_lpBMIH == NULL) return FALSE;
	    if(m_hPalette != NULL) {
		    ::SelectPalette(pDC->GetSafeHdc(), m_hPalette, TRUE);
	    }
	    pDC->SetStretchBltMode(COLORONCOLOR);
	    ::StretchDIBits(pDC->GetSafeHdc(), origin.x, origin.y, size.cx, size.cy,
		    from.x, m_lpBMIH->biHeight - from.y - part.cy, part.cx, part.cy,
		    m_lpImage, (LPBITMAPINFO) m_lpBMIH, DIB_RGB_COLORS, SRCCOPY);
	    return TRUE;
    }

    HBITMAP CDib::CreateSection(CDC* pDC /* = NULL */)
    {
	    if(m_lpBMIH == NULL) return NULL;
	    if(m_lpImage != NULL) return NULL; // can only do this if image doesn't exist
	    m_hBitmap = ::CreateDIBSection(pDC->GetSafeHdc(), (LPBITMAPINFO) m_lpBMIH,
		    DIB_RGB_COLORS,	(LPVOID*) &m_lpImage, NULL, 0);
	    ASSERT(m_lpImage != NULL);
	    return m_hBitmap;
    }

    BOOL CDib::MakePalette()
    {
	    // makes a logical palette (m_hPalette) from the DIB's color table
	    // this palette will be selected and realized prior to drawing the DIB
	    if(m_nColorTableEntries == 0) return FALSE;
	    if(m_hPalette != NULL) ::DeleteObject(m_hPalette);
	    TRACE("CDib::MakePalette -- m_nColorTableEntries = %d\n", m_nColorTableEntries);
	    LPLOGPALETTE pLogPal = (LPLOGPALETTE) new char[2 * sizeof(WORD) +
		    m_nColorTableEntries * sizeof(PALETTEENTRY)];
	    pLogPal->palVersion = 0x300;
	    pLogPal->palNumEntries = m_nColorTableEntries;
	    LPRGBQUAD pDibQuad = (LPRGBQUAD) m_lpvColorTable;
	    for(int i = 0; i < m_nColorTableEntries; i++) {
		    pLogPal->palPalEntry[i].peRed = pDibQuad->rgbRed;
		    pLogPal->palPalEntry[i].peGreen = pDibQuad->rgbGreen;
		    pLogPal->palPalEntry[i].peBlue = pDibQuad->rgbBlue;
		    pLogPal->palPalEntry[i].peFlags = 0;
		    pDibQuad++;
	    }
	    m_hPalette = ::CreatePalette(pLogPal);
	    delete pLogPal;
	    return TRUE;
    }	

    BOOL CDib::SetSystemPalette(CDC* pDC)
    {
	    // if the DIB doesn't have a color table, we can use the system's halftone palette
	    if(m_nColorTableEntries != 0) return FALSE;
	    m_hPalette = ::CreateHalftonePalette(pDC->GetSafeHdc());
	    return TRUE;
    }

    HBITMAP CDib::CreateBitmap(CDC* pDC)
    {
        if (m_dwSizeImage == 0) return NULL;
	    HBITMAP hBitmap = ::CreateDIBitmap(pDC->GetSafeHdc(), m_lpBMIH,
                CBM_INIT, m_lpImage, (LPBITMAPINFO) m_lpBMIH, DIB_RGB_COLORS);
        ASSERT(hBitmap != NULL);
        return hBitmap;
    }

    BOOL CDib::Compress(CDC* pDC, BOOL bCompress /* = TRUE */)
    {
	    // 1. makes GDI bitmap from existing DIB
	    // 2. makes a new DIB from GDI bitmap with compression
	    // 3. cleans up the original DIB
	    // 4. puts the new DIB in the object
	    if((m_lpBMIH->biBitCount != 4) && (m_lpBMIH->biBitCount != 8)) return FALSE;
		    // compression supported only for 4 bpp and 8 bpp DIBs
	    if(m_hBitmap) return FALSE; // can't compress a DIB Section!
	    TRACE("Compress: original palette size = %d\n", m_nColorTableEntries); 
	    HDC hdc = pDC->GetSafeHdc();
	    HPALETTE hOldPalette = ::SelectPalette(hdc, m_hPalette, FALSE);
	    HBITMAP hBitmap;  // temporary
	    if((hBitmap = CreateBitmap(pDC)) == NULL) return FALSE;
	    int nSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries;
	    LPBITMAPINFOHEADER lpBMIH = (LPBITMAPINFOHEADER) new char[nSize];
	    memcpy(lpBMIH, m_lpBMIH, nSize);  // new header
	    if(bCompress) {
		    switch (lpBMIH->biBitCount) {
		    case 4:
			    lpBMIH->biCompression = BI_RLE4;
			    break;
		    case 8:
			    lpBMIH->biCompression = BI_RLE8;
			    break;
		    default:
			    ASSERT(FALSE);
		    }
		    // calls GetDIBits with null data pointer to get size of compressed DIB
		    if(!::GetDIBits(pDC->GetSafeHdc(), hBitmap, 0, (UINT) lpBMIH->biHeight,
						    NULL, (LPBITMAPINFO) lpBMIH, DIB_RGB_COLORS)) {
			    AfxMessageBox("Unable to compress this DIB");
			    // probably a problem with the color table
	 		    ::DeleteObject(hBitmap);
			    delete [] lpBMIH;
			    ::SelectPalette(hdc, hOldPalette, FALSE);
			    return FALSE; 
		    }
		    if (lpBMIH->biSizeImage == 0) {
			    AfxMessageBox("Driver can't do compression");
	 		    ::DeleteObject(hBitmap);
			    delete [] lpBMIH;
			    ::SelectPalette(hdc, hOldPalette, FALSE);
			    return FALSE; 
		    }
		    else {
			    m_dwSizeImage = lpBMIH->biSizeImage;
		    }
	    }
	    else {
		    lpBMIH->biCompression = BI_RGB; // decompress
		    // figure the image size from the bitmap width and height
		    DWORD dwBytes = ((DWORD) lpBMIH->biWidth * lpBMIH->biBitCount) / 32;
		    if(((DWORD) lpBMIH->biWidth * lpBMIH->biBitCount) % 32) {
			    dwBytes++;
		    }
		    dwBytes *= 4;
		    m_dwSizeImage = dwBytes * lpBMIH->biHeight; // no compression
		    lpBMIH->biSizeImage = m_dwSizeImage;
	    } 
	    // second GetDIBits call to make DIB
	    LPBYTE lpImage = (LPBYTE) new char[m_dwSizeImage];
	    VERIFY(::GetDIBits(pDC->GetSafeHdc(), hBitmap, 0, (UINT) lpBMIH->biHeight,
    		    lpImage, (LPBITMAPINFO) lpBMIH, DIB_RGB_COLORS));
        TRACE("dib successfully created - height = %d\n", lpBMIH->biHeight);
	    ::DeleteObject(hBitmap);
	    Empty();
	    m_nBmihAlloc = m_nImageAlloc = crtAlloc;
	    m_lpBMIH = lpBMIH;
	    m_lpImage = lpImage;
	    ComputeMetrics();
	    ComputePaletteSize(m_lpBMIH->biBitCount);
	    MakePalette();
	    ::SelectPalette(hdc, hOldPalette, FALSE);
	    TRACE("Compress: new palette size = %d\n", m_nColorTableEntries); 
	    return TRUE;
    }

    void CDib::CreateEmptyImage24(CSize size)
    {
        Empty();

	    ComputePaletteSize(24);

        m_lpBMIH = new BITMAPINFOHEADER();
    	
	    m_nBmihAlloc = m_nImageAlloc = crtAlloc;

        m_lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
	    m_lpBMIH->biWidth = size.cx;
	    m_lpBMIH->biHeight = size.cy;
	    m_lpBMIH->biPlanes = 1;
	    m_lpBMIH->biBitCount = 24;
	    m_lpBMIH->biCompression = BI_RGB;
	    m_lpBMIH->biSizeImage = (3 * size.cx + (4 - 3*size.cx%4)) * size.cy;
	    m_lpBMIH->biXPelsPerMeter = 0;
	    m_lpBMIH->biYPelsPerMeter = 0;
	    m_lpBMIH->biClrUsed = m_nColorTableEntries;
	    m_lpBMIH->biClrImportant = m_nColorTableEntries;

	    ComputeMetrics();
	    MakePalette();

        m_lpImage = (LPBYTE) new char[m_dwSizeImage];
        for(DWORD i = 0, sz = m_dwSizeImage; i < sz; ++i)
            m_lpImage[i] = 0;
    }


    CDib* CDib::Copy()
    {
        CDib* pRet = new CDib();
        pRet->Empty();
    	
        DWORD sz = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries;
        char* pInfo = new char[sz];
        for(DWORD i = 0; i < sz; ++i)
            pInfo[i] = ((char*)m_lpBMIH)[i];
        
        pRet->m_lpBMIH = (LPBITMAPINFOHEADER) pInfo;
        
        pRet->m_nBmihAlloc = m_nBmihAlloc;
        pRet->m_nImageAlloc = m_nImageAlloc;

	    pRet->ComputeMetrics();
	    pRet->ComputePaletteSize(pRet->m_lpBMIH->biBitCount);
	    pRet->MakePalette();

        pRet->m_lpImage = (LPBYTE) new char[pRet->m_dwSizeImage];
        for(DWORD i = 0, sz = pRet->m_dwSizeImage; i < sz; ++i)
            pRet->m_lpImage[i] = m_lpImage[i];

        return pRet;
    }


    BOOL CDib::Read(CFile* pFile)
    {
	    // 1. read file header to get size of info hdr + color table
	    // 2. read info hdr (to get image size) and color table
	    // 3. read image
	    // can't use bfSize in file header
        Empty();
        int nCount, nSize;
        BITMAPFILEHEADER bmfh;
        
        nCount = pFile->Read((LPVOID) &bmfh, sizeof(BITMAPFILEHEADER));
        if(nCount != sizeof(BITMAPFILEHEADER)) {
	        throw CDibException("Read", "Cannot read BITMAPFILEHEADER part of bmp file");
        }
        if(bmfh.bfType != 0x4d42) {
	        throw CDibException("Read", "Bad format of the bmp file");
        }
        nSize = bmfh.bfOffBits - sizeof(BITMAPFILEHEADER);
        m_lpBMIH = (LPBITMAPINFOHEADER) new char[nSize];
        m_nBmihAlloc = m_nImageAlloc = crtAlloc;
        nCount = pFile->Read(m_lpBMIH, nSize); // info hdr & color table
        ComputeMetrics();
        ComputePaletteSize(m_lpBMIH->biBitCount);
        MakePalette();
        m_lpImage = (LPBYTE) new char[m_dwSizeImage];
        nCount = pFile->Read(m_lpImage, m_dwSizeImage); // image only

        return TRUE;
    }

    BOOL CDib::Read(std::istream* pIstr)
    {
	    // 1. read file header to get size of info hdr + color table
	    // 2. read info hdr (to get image size) and color table
	    // 3. read image
	    // can't use bfSize in file header
        Empty();
        int nSize;
        BITMAPFILEHEADER bmfh;
        
        pIstr->read((char*) &bmfh, sizeof(BITMAPFILEHEADER));
        if(pIstr->fail()) {
            throw CDibException("Read", "Cannot read BITMAPFILEHEADER part of bmp file");
        }
        if(bmfh.bfType != 0x4d42) {
	        throw CDibException("Read", "Bad format of the bmp file");
        }
        nSize = bmfh.bfOffBits - sizeof(BITMAPFILEHEADER);
        m_lpBMIH = (LPBITMAPINFOHEADER) new char[nSize];
        m_nBmihAlloc = m_nImageAlloc = crtAlloc;
        pIstr->read((char*)m_lpBMIH, nSize); // info hdr & color table
        ComputeMetrics();
        ComputePaletteSize(m_lpBMIH->biBitCount);
        MakePalette();
        m_lpImage = (LPBYTE) new char[m_dwSizeImage];
        pIstr->read((char*)m_lpImage, m_dwSizeImage); // image only

	    return TRUE;
    }

    BOOL CDib::ReadSection(CFile* pFile, CDC* pDC /* = NULL */)
    {
	    // new function reads BMP from disk and creates a DIB section
	    //    allows modification of bitmaps from disk
	    // 1. read file header to get size of info hdr + color table
	    // 2. read info hdr (to get image size) and color table
	    // 3. create DIB section based on header parms
	    // 4. read image into memory that CreateDibSection allocates
	    Empty();
	    int nCount, nSize;
	    BITMAPFILEHEADER bmfh;
	    nCount = pFile->Read((LPVOID) &bmfh, sizeof(BITMAPFILEHEADER));
	    if(nCount != sizeof(BITMAPFILEHEADER)) {
            throw CDibException("ReadSection", "Cannot read BITMAPFILEHEADER part of bmp file");
	    }
	    if(bmfh.bfType != 0x4d42) {
	        throw CDibException("ReadSection", "Bad format of the bmp file");
	    }
	    nSize = bmfh.bfOffBits - sizeof(BITMAPFILEHEADER);
	    m_lpBMIH = (LPBITMAPINFOHEADER) new char[nSize];
	    m_nBmihAlloc = crtAlloc;
	    m_nImageAlloc = noAlloc;
	    nCount = pFile->Read(m_lpBMIH, nSize); // info hdr & color table
	    if(m_lpBMIH->biCompression != BI_RGB) {
	        throw CDibException("ReadSection", "Bad compression of the bmp file");
	    }
	    ComputeMetrics();
	    ComputePaletteSize(m_lpBMIH->biBitCount);
	    MakePalette();
	    UsePalette(pDC);
	    m_hBitmap = ::CreateDIBSection(pDC->GetSafeHdc(), (LPBITMAPINFO) m_lpBMIH,
		    DIB_RGB_COLORS,	(LPVOID*) &m_lpImage, NULL, 0);
	    ASSERT(m_lpImage != NULL);
	    nCount = pFile->Read(m_lpImage, m_dwSizeImage); // image only

        return TRUE;
    }

    BOOL CDib::Write(CFile* pFile)
    {
	    BITMAPFILEHEADER bmfh;
	    bmfh.bfType = 0x4d42;  // 'BM'
	    int nSizeHdr = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries;
	    bmfh.bfSize = 0;
    //	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + nSizeHdr + m_dwSizeImage;
	    // meaning of bfSize open to interpretation (bytes, words, dwords?) -- we won't use it
	    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
			    sizeof(RGBQUAD) * m_nColorTableEntries;	
	    
        pFile->Write((LPVOID) &bmfh, sizeof(BITMAPFILEHEADER));
	    pFile->Write((LPVOID) m_lpBMIH,  nSizeHdr);
	    pFile->Write((LPVOID) m_lpImage, m_dwSizeImage);

        return TRUE;
    }

    BOOL CDib::Write(std::ostream* pOstr)
    {
	    BITMAPFILEHEADER bmfh;
	    bmfh.bfType = 0x4d42;  // 'BM'
	    int nSizeHdr = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries;
	    bmfh.bfSize = 0;
    //	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + nSizeHdr + m_dwSizeImage;
	    // meaning of bfSize open to interpretation (bytes, words, dwords?) -- we won't use it
	    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
			    sizeof(RGBQUAD) * m_nColorTableEntries;	
        
        pOstr->write((char*) &bmfh, sizeof(BITMAPFILEHEADER));
	    pOstr->write((char*) m_lpBMIH,  nSizeHdr);
	    pOstr->write((char*) m_lpImage, m_dwSizeImage);

        return TRUE;
    }

    void CDib::Serialize(CArchive& ar)
    {
	    ULONGLONG dwPos;
	    dwPos = ar.GetFile()->GetPosition();
	    TRACE("CDib::Serialize -- pos = %d\n", dwPos);
	    ar.Flush();
	    dwPos = ar.GetFile()->GetPosition();
	    TRACE("CDib::Serialize -- pos = %d\n", dwPos);
	    if(ar.IsStoring()) {
		    Write(ar.GetFile());
	    }
	    else {
		    Read(ar.GetFile());
	    }
    }

    // helper functions
    void CDib::ComputePaletteSize(int nBitCount)
    {
	    if((m_lpBMIH == NULL) || (m_lpBMIH->biClrUsed == 0)) {
		    switch(nBitCount) {
			    case 1:
				    m_nColorTableEntries = 2;
				    break;
			    case 4:
				    m_nColorTableEntries = 16;
				    break;
			    case 8:
				    m_nColorTableEntries = 256;
				    break;
			    case 16:
			    case 24:
			    case 32:
				    m_nColorTableEntries = 0;
				    break;
			    default:
				    ASSERT(FALSE);
		    }
	    }
	    else {
		    m_nColorTableEntries = m_lpBMIH->biClrUsed;
	    }
	    ASSERT((m_nColorTableEntries >= 0) && (m_nColorTableEntries <= 256)); 
    }

    void CDib::ComputeMetrics()
    {
	    if(m_lpBMIH->biSize != sizeof(BITMAPINFOHEADER)) {
	        throw CDibException("ComputeMetrics", "Not a valid Windows bitmap - probably an OS/2 bitmap");
	    }
	    m_dwSizeImage = m_lpBMIH->biSizeImage;
	    if(m_dwSizeImage == 0) {
		    DWORD dwBytes = ((DWORD) m_lpBMIH->biWidth * m_lpBMIH->biBitCount) / 32;
		    if(((DWORD) m_lpBMIH->biWidth * m_lpBMIH->biBitCount) % 32) {
			    dwBytes++;
		    }
		    dwBytes *= 4;
		    m_dwSizeImage = dwBytes * m_lpBMIH->biHeight; // no compression
	    }
	    m_lpvColorTable = (LPBYTE) m_lpBMIH + sizeof(BITMAPINFOHEADER);
    }

    void CDib::Empty()
    {
	    // this is supposed to clean up whatever is in the DIB
	    DetachMapFile();
	    if(m_nBmihAlloc == crtAlloc) {
		    delete [] m_lpBMIH;
	    }
	    else if(m_nBmihAlloc == heapAlloc) {
		    ::GlobalUnlock(m_hGlobal);
		    ::GlobalFree(m_hGlobal);
	    }
	    if(m_nImageAlloc == crtAlloc) delete [] m_lpImage;
	    if(m_hPalette != NULL) ::DeleteObject(m_hPalette);
	    if(m_hBitmap != NULL) ::DeleteObject(m_hBitmap);
	    m_nBmihAlloc = m_nImageAlloc = noAlloc;
	    m_hGlobal = NULL;
	    m_lpBMIH = NULL;
	    m_lpImage = NULL;
	    m_lpvColorTable = NULL;
	    m_nColorTableEntries = 0;
	    m_dwSizeImage = 0;
	    m_lpvFile = NULL;
	    m_hMap = NULL;
	    m_hFile = NULL;
	    m_hBitmap = NULL;
	    m_hPalette = NULL;
    }

    void CDib::DetachMapFile()
    {
	    if(m_hFile == NULL) return;
	    ::UnmapViewOfFile(m_lpvFile);
	    ::CloseHandle(m_hMap);
	    ::CloseHandle(m_hFile);
	    m_hFile = NULL;
    }

    void MfcMessenger::Write(WriteType wt, std::string strMsg)
    {
        if(wt == WT_DEBUG)
        {
            std::ofstream ofs(strFile.c_str(), std::ios_base::out | std::ios_base::app);
            ofs << strMsg;
        }
        else
        {
            AfxGetMainWnd()->MessageBox(strMsg.c_str(), "Error Message", MB_ICONSTOP); 
        }
    }

}