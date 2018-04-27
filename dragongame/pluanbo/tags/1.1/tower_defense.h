#include "GuiGen.h"

#include <vector>
#include <set>
#include <string>

#include <sstream>
#include <fstream>

#include<stdlib.h>
#include<math.h>

using namespace Gui;

typedef Gui::GraphicalInterface<unsigned> Graphic;

struct GlobalController;

struct GameController
{
    GlobalController* pGl;

    Rectangle rBound;

    GameController(GlobalController* pGl_, Rectangle rBound_ = Rectangle()):pGl(pGl_), rBound(rBound_){}

    virtual void Update(){};
    virtual void OnKey(char c, bool bUp){};
    virtual void OnMouse(Gui::Point pShift){};
};

struct Drawer
{
    Graphic* pGr;
    virtual void Draw(unsigned nImg, Point p, bool bCentered = true)=0;
};

struct ScalingDrawer: public Drawer
{
    unsigned nFactor;
    Color cTr;

    ScalingDrawer(Graphic* pGr_, unsigned nFactor_, Color cTr_ = Color(0, 255, 255))
        :nFactor(nFactor_), cTr(cTr_){pGr = pGr_;}

    void Scale(unsigned& pImg)
    {
        Image* pOrig = pGr->GetImage(pImg);
        unsigned pRet = pGr->GetBlankImage(Size(pOrig->GetSize().x * nFactor, pOrig->GetSize().y * nFactor));
        Image* pFin = pGr->GetImage(pRet);
        
        Point p, s;
        for(p.y = 0; p.y < pOrig->GetSize().y; ++p.y)
        for(p.x = 0; p.x < pOrig->GetSize().x; ++p.x)
        for(s.y = 0; s.y < nFactor; ++s.y)
        for(s.x = 0; s.x < nFactor; ++s.x)
            pFin->SetPixel(Point(p.x * nFactor + s.x, p.y * nFactor + s.y), pOrig->GetPixel(p));

        pGr->DeleteImage(pImg);
        pImg = pRet;
    }

    /*virtual*/ void Draw(unsigned nImg, Point p, bool bCentered = true)
    {
        p.x *= nFactor;
        p.y *= nFactor;
        if(bCentered)
        {
            Size sz = pGr->GetImage(nImg)->GetSize();
            p.x -= sz.x / 2;
            p.y -= sz.y / 2;
        }

        pGr->DrawImage(p, nImg, false);
    }

    /*virtual*/ unsigned LoadImage(std::string strFile)
    {
        unsigned n = pGr->LoadImage(strFile);
        pGr->GetImage(n)->ChangeColor(cTr, Color(0,0,0,0));
        pGr->GetImage(n)->ChangeColor(Color(0,0,0), Color(0,0,0,0));
        return n;
    }
};

struct NumberDrawer
{
    ScalingDrawer* pDr;
    std::vector<unsigned> vImg;

    NumberDrawer(ScalingDrawer* pDr_):pDr(pDr_)
    {
        unsigned nImg = pDr->LoadImage("numbers.bmp");
        Image*   pImg = pDr->pGr->GetImage(nImg);
        for(unsigned i = 0; i <= 9; ++i)
        {
            unsigned nCurr = pDr->pGr->GetBlankImage(Size(3, 5));
            Image* pCurr = pDr->pGr->GetImage(nCurr);
            
            Point p;
            for(p.y = 0; p.y < 5; ++p.y)
            for(p.x = 0; p.x < 3; ++p.x)
                pCurr->SetPixel(p, pImg->GetPixel(Point(i * 4 + p.x, p.y)));
            pDr->Scale(nCurr);
            vImg.push_back(nCurr);
        }

        pDr->pGr->DeleteImage(nImg);
    }

    void DrawNumber(unsigned n, Point p)
    {
        std::vector<unsigned> vDigits;
        if(n == 0)
            vDigits.push_back(0);
        while(n != 0)
        {
            vDigits.push_back(n%10);
            n /= 10;
        }
        for(unsigned i = 0; i < vDigits.size(); ++i)
        {
            pDr->Draw(vImg[vDigits[vDigits.size() - i - 1]], Point(p.x + 4 * i, p.y), false);
        }
    }
};

struct GlobalController
{
    std::vector<GameController*> vCnt;
    unsigned nActive;

    Graphic* pGraph;
    ScalingDrawer* pDr;
    NumberDrawer* pNum;


    GlobalController(ScalingDrawer* pDr_, NumberDrawer* pNum_)
        :nActive(0), pDr(pDr_), pGraph(pDr_->pGr), pNum(pNum_){}
    
    ~GlobalController()
    {
        for(unsigned i = 0, sz = vCnt.size(); i < sz; ++i)
            delete vCnt[i];
    }
};

struct SimpleController: public GameController
{
    unsigned nImage;

    SimpleController(GlobalController* pGraph, std::string strFileName):GameController(pGraph)
    {
        nImage = pGl->pGraph->LoadImage(strFileName);
    }

    ~SimpleController()
    {
        pGl->pGraph->DeleteImage(nImage);
    }

    /*virtual*/ void Update()
    {
        pGl->pGraph->DrawImage(Gui::Point(0,0), nImage);
    }

    /*virtual*/ void OnKey(char c, bool bUp)
    {
        if(bUp)
            return;
        
        if(pGl->nActive == pGl->vCnt.size() - 1)
            pGl->nActive = 0;
        else
            ++pGl->nActive;
    }
};

struct FlashingController: public GameController
{
    unsigned nImage, nText;
    unsigned nTimer;
    bool bShow;

    FlashingController(GlobalController* pGraph, std::string strFileName, std::string strTextName)
        :GameController(pGraph), nTimer(0), bShow(true)
    {
        nImage = pGl->pGraph->LoadImage(strFileName);
        nText  = pGl->pGraph->LoadImage(strTextName);

        pGl->pGraph->GetImage(nText)->ChangeColor(Color(255,255,255), Color(0,0,0,0));
    }

    ~FlashingController()
    {
        pGl->pGraph->DeleteImage(nImage);
        pGl->pGraph->DeleteImage(nText);
    }

    /*virtual*/ void Update()
    {
        ++nTimer;
        if(nTimer % 10 == 0)
            bShow = !bShow;
        
        pGl->pGraph->DrawImage(Point(0,0), nImage, false);
        if(bShow)
            pGl->pGraph->DrawImage(Point(0,0), nText, false);
        pGl->pGraph->RefreshAll();
    }

    /*virtual*/ void OnKey(char c, bool bUp)
    {
        if(bUp)
            return;
        
        if(pGl->nActive == pGl->vCnt.size() - 1)
            pGl->nActive = 0;
        else
            ++pGl->nActive;
    }
};

struct Entity
{
    bool bExist;
    Entity():bExist(true){}
    virtual ~Entity(){}

};

struct EventEntity : virtual public Entity
{
    virtual void Move(){}
    virtual void Update(){}
};

struct ScreenEntity : virtual public Entity
{
    virtual Point GetPosition()=0;
};



struct VisualEntity : virtual public ScreenEntity
{
    virtual void Draw(Drawer * pDr){}
    virtual double GetPriority(){return 0;}
};

struct Timer
{
    unsigned nTimer, nPeriod;

    Timer(unsigned nPeriod_ = 1):nTimer(0), nPeriod(nPeriod_){}

    bool Tick(){return (++nTimer % nPeriod == 0);}
};

struct ImageSequence
{
    std::vector<unsigned> vImage;
    unsigned nActive;

    void Toggle()
    {
        if(nActive == vImage.size() - 1)
            nActive = 0;
        else
            nActive++;
    }

    unsigned GetImage(){return vImage[nActive];}

    void Add(unsigned nImg){vImage.push_back(nImg);}

    ImageSequence():nActive(0){}
    ImageSequence(unsigned img1):nActive(0){Add(img1);}
    ImageSequence(unsigned img1, unsigned img2):nActive(0){Add(img1); Add(img2);}
    ImageSequence(unsigned img1, unsigned img2, unsigned img3):nActive(0){Add(img1); Add(img2); Add(img3);}

    void Load(ScalingDrawer* pDr, std::string fFile, std::string fFolder = "")
    {
        std::ifstream ifs((fFolder + fFile).c_str());
        
        while(true)
        {
            std::string s;
            ifs >> s;
            if(ifs.fail())
                break;
            unsigned nImg = pDr->LoadImage(fFolder + s);
            pDr->Scale(nImg);
            Add(nImg);
        }
    }

    ImageSequence(ScalingDrawer* pDr, std::string fFile, std::string fFolder = ""):nActive(0)
    {
        Load(pDr, fFile, fFolder);
    }

};

struct SimpleVisualEntity : virtual public EventEntity, public VisualEntity
{
    double dPriority;

    Timer t;
    
    bool bTimer, bStep, bCenter;

    Point pPrev;

    ImageSequence seq;


    SimpleVisualEntity(double dPriority_, const ImageSequence& seq_, bool bCenter_, unsigned nPeriod) 
        : dPriority(dPriority_), seq(seq_), t(nPeriod), bCenter(bCenter_),
        bTimer(true), bStep(false) 
    {}
    
    SimpleVisualEntity(double dPriority_, const ImageSequence& seq_, bool bCenter_, bool bStep_ = false) 
        : dPriority(dPriority_), seq(seq_), bCenter(bCenter_),
        bTimer(false), bStep(bStep_) 
    {}

    /*virtual*/ void Draw(Drawer * pDr)
    {
        pDr->Draw(seq.GetImage(), GetPosition(), bCenter);
    }

    /*virtual*/ double GetPriority() { return dPriority; }

    /*virtual*/ void Update()
    {
        if(bTimer)
        {
            if(t.Tick())
                seq.Toggle();
        }
        else if(bStep)
        {
            Point p = GetPosition();
            if(p != pPrev)
                seq.Toggle();
            pPrev = p;
        }
    }
};

struct Animation: public SimpleVisualEntity
{
    Point pos;
    
    Animation(double dPriority_, const ImageSequence& seq, unsigned nTimeMeasure_, Point p, bool bCenter = false)
        :SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p){}

    /*virtual*/ Point GetPosition(){return pos;}
};

struct Countdown: public VisualEntity, public EventEntity
{
    NumberDrawer* pNum;
    unsigned nTime, nCount;

    Countdown(NumberDrawer* pNum_, unsigned nTime_):pNum(pNum_), nTime(nTime_), nCount(0){}
    
    /*virtual*/ void Update()
    {
        ++nCount;
        if(nCount % 10 == 0)
            --nTime;
        if(nTime == 0)
            bExist = false;
    }

    /*virtual*/ void Draw(Drawer* pDr)
    {
        pNum->DrawNumber(nTime, Point(22, 2));
    }

    /*virtual*/ Point GetPosition()
    {
        return Point(0,0);
    }
};

struct PhysicalEntity: virtual public ScreenEntity
{
    virtual unsigned GetRadius(){return 0;}
    
    bool HitDetection(PhysicalEntity* pPh)
    {
        Point d = GetPosition() - pPh->GetPosition();
        unsigned r1 = GetRadius(), r2 = pPh->GetRadius();
        return ( d.x * d.x + d.y * d.y ) < ( r1 * r1 + r2 * r2 );
    }
};

struct fPoint
{
    float x,y;

    fPoint():x(0), y(0){}
    fPoint(float x_, float y_):x(x_), y(y_){}

    fPoint(Point p): x(p.x), y(p.y){}

    operator Point() {return Point(int(x), int(y));}

    void Normalize()
    {
        float d = sqrt(x * x + y * y);
        x /= d;
        y /= d;
    }
};

inline fPoint& operator += (fPoint& f1, const fPoint& f2){f1.x += f2.x; f1.y += f2.y; return f1;}
inline fPoint& operator -= (fPoint& f1, const fPoint& f2){f1.x -= f2.x; f1.y -= f2.y; return f1;}
inline fPoint operator + (const fPoint& f1, const fPoint& f2){return fPoint(f1) += f2;}
inline fPoint operator - (const fPoint& f1, const fPoint& f2){return fPoint(f1) -= f2;}
inline bool operator == (const fPoint& f1, const fPoint& f2){return (f1.x == f2.x) && (f1.y == f2.y);}
inline bool operator != (const fPoint& f1, const fPoint& f2){return (f1.x != f2.x) || (f1.y != f2.y);}

struct ConsumableEntity: virtual public PhysicalEntity
{
    virtual char GetType()=0;
    virtual void OnHit(){bExist = false;}
    virtual unsigned GetImage()=0;
};

struct Critter: virtual public PhysicalEntity, public SimpleVisualEntity
{
    unsigned nRadius;
    fPoint fPos;
    fPoint fVel;

    Rectangle rBound;
    bool bDieOnExit;

    /*virtual*/ unsigned int GetRadius(){return nRadius;}
    /*virtual*/ Point GetPosition(){return fPos;}
    /*virtual*/ void Move()
    {
        fPos += fVel;
        if(!InsideRectangle(rBound, fPos))
        {
            fPos -= fVel;
            if(bDieOnExit)
                bExist = false;
        }
    }

    Critter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
        double dPriority, const ImageSequence& seq, unsigned nPeriod)
        :SimpleVisualEntity(dPriority, seq, true, nPeriod), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true){} 
    
    Critter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
        double dPriority, const ImageSequence& seq, bool bStep = false)
        :SimpleVisualEntity(dPriority, seq, true, true), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true){} 
};



struct BasicController: public GameController
{
    std::list<VisualEntity*> lsDraw;
    std::list<EventEntity*>  lsUpdate;
    std::list<ConsumableEntity*>  lsPpl;

    void Add(VisualEntity* pVs){lsDraw.push_back(pVs);}
    void Add(EventEntity* pEv){lsUpdate.push_back(pEv);}
    
    template<class T>
    void AddBoth(T t)
    {
        lsDraw.push_back(t);
        lsUpdate.push_back(t);
    }

    void AddBackground(Color c)
    {
        unsigned nBckImg = pGl->pDr->pGr->GetBlankImage(rBound.sz);
        pGl->pDr->pGr->RectangleOnto(nBckImg, rBound.sz, c);
        pGl->pDr->Scale(nBckImg);
        
        Animation* pBkg = new Animation(0, nBckImg, 1, rBound.p);

        AddBoth(pBkg);
    }

    BasicController(GlobalController* pGl_, Rectangle rBound, Color c):GameController(pGl_, rBound)
    {
        AddBackground(c);
    }

    /*virtual*/ void Update()
    {
        {
        for(std::list<EventEntity*>::iterator itr = lsUpdate.begin(); itr != lsUpdate.end();)
        {
            if(!(*itr)->bExist)
            {
                lsUpdate.erase(itr++);
                continue;
            }

            (*itr)->Move();
            ++itr;
        }
        }

        {
        for(std::list<EventEntity*>::iterator itr = lsUpdate.begin(); itr != lsUpdate.end();)
        {
            if(!(*itr)->bExist)
            {
                lsUpdate.erase(itr++);
                continue;
            }

            (*itr)->Update();
            ++itr;
        }
        }
 
        {
            std::list<VisualEntity*>::iterator itr;
                
            std::set<double> st;
            
            for(itr = lsDraw.begin(); itr != lsDraw.end();)
            {
                if(!(*itr)->bExist)
                {
                    lsDraw.erase(itr++);
                    continue;
                }

                st.insert((*itr)->GetPriority());
                ++itr;
            }


            for(std::set<double>::iterator sitr = st.begin(); sitr != st.end(); ++sitr)
                for(itr = lsDraw.begin(); itr != lsDraw.end(); ++itr)
                {
                    if((*itr)->GetPriority() == *sitr)
                        (*itr)->Draw(pGl->pDr);
                }
        }

        pGl->pGraph->RefreshAll();
    }

    /*virtual*/ void OnKey(char c, bool bUp)
    {
        if(bUp)
            return;
        
        if(pGl->nActive == pGl->vCnt.size() - 1)
            pGl->nActive = 0;
        else
            ++pGl->nActive;
    }

};




struct Fireball: public Critter
{
    BasicController* pBc;

    Fireball(Point p, fPoint v, Rectangle rBound_, BasicController* pBc_)
        :Critter(5, p, v, rBound_, 6, ImageSequence(pBc_->pGl->pDr, "fireball.txt", "fireball\\"), 5U),
        pBc(pBc_){}

    /*virtual*/ void Update()
    {
        for(std::list<ConsumableEntity*>::iterator itr = pBc->lsPpl.begin(); itr != pBc->lsPpl.end();)
        {
            if(!(*itr)->bExist)
            {
                pBc->lsPpl.erase(itr++);
                continue;
            }

            if (this->HitDetection(*itr))
            {
                bExist = false;
                (*itr)->OnHit();
            }
            ++itr;
        }

        Critter::Update();
    }
};

struct AdvancedController;

struct Castle: public Critter
{
    unsigned nPrincesses;
    AdvancedController* pAv;
    
    Castle(Point p, Rectangle rBound_, AdvancedController* pAv_);

    void OnKnight();

    /*unsigned*/ void Draw(Drawer* pDr);
};

inline Point Center(Size sz){return Point(sz.x/2, sz.y/2);}



struct CritterGenerator;
struct Dragon;


struct AdvancedController: public BasicController
{
    std::vector<Castle*> vCs;
    Dragon* pDr;

    bool bCh;

    ImageSequence imgPr;
    
    AdvancedController(GlobalController* pGl_, Rectangle rBound, Color c, int lvl = 1);

    /*virtual*/ void OnKey(char c, bool bUp);

    /*virtual*/ void Update()
    {
        BasicController::Update();

        unsigned i;
        for(i = 0; i < vCs.size(); ++i)
        {
            if(vCs[i]->nPrincesses < 4)
                break;
        }
        
        if(i == vCs.size())
        {
            if(pGl->nActive == pGl->vCnt.size() - 1)
                pGl->nActive = 0;
            else
                ++pGl->nActive;
        }
    }

};

struct Princess:  public Critter, public ConsumableEntity
{
    AdvancedController* pAc;

    Princess(const Critter& cr, AdvancedController* pAc_):Critter(cr), pAc(pAc_){}
    
    void Add()
    {
        pAc->AddBoth(this);
        pAc->lsPpl.push_back(this);
    }

    /*virtual*/ unsigned GetImage()
    {
        return seq.vImage[0];
    }

    /*virtual*/ char GetType(){return 'P';}
};

struct Knight: public Critter, public ConsumableEntity
{
    AdvancedController* pAc;
    
    Knight(const Critter& cr, AdvancedController* pAc_):Critter(cr), pAc(pAc_){}

    /*virtual*/ void Update()
    {
        for(unsigned i = 0; i < pAc->vCs.size(); ++i)
            if(this->HitDetection(pAc->vCs[i]))
            {
                pAc->vCs[i]->OnKnight();
                bExist = false;
                break;
            }
        
        Critter::Update();
    }

    void Add()
    {
        pAc->AddBoth(this);
        pAc->lsPpl.push_back(this);
    }

    /*virtual*/ unsigned GetImage()
    {
        return seq.vImage[0];
    }

    /*virtual*/ char GetType(){return 'K';}
};


struct CritterGenerator: virtual public EventEntity
{
    double dRate;
    Rectangle rBound;
    AdvancedController* pBc;
    ImageSequence seq;

    CritterGenerator(double dRate_, Rectangle rBound_, AdvancedController* pBc_)
        :dRate(dRate_), rBound(rBound_), pBc(pBc_), seq(pBc_->pGl->pDr, "knight.txt", "knight\\") {}

    /*virtual*/ void Update()
    {
        if(double(rand())/RAND_MAX < dRate)
        {
            Point p;
            
            double d = double(rand())/RAND_MAX;
            if(rand()%2)
            {
                p.x = rBound.sz.x * d;
                if(rand()%2)
                    p.y = 0;
                else
                    p.y = rBound.sz.y - 1;
            }
            else
            {
                p.y = rBound.sz.y * d;
                if(rand()%2)
                    p.x = 0;
                else
                    p.x = rBound.sz.x - 1;
            }
            
            unsigned n = rand()%pBc->vCs.size();
            
            fPoint v = pBc->vCs[n]->GetPosition() - p;
            v.Normalize();
            v.x *= .75;
            v.y *= .75;
            p += rBound.p;
            
            Knight* pCr = new Knight(Critter(7, p, v, rBound, 0, seq, true), pBc);
            pCr->Add();
        }
    }
};

struct PrincessGenerator: virtual public EventEntity
{
    double dRate;
    Rectangle rBound;
    AdvancedController* pBc;
    ImageSequence seq;

    PrincessGenerator(double dRate_, Rectangle rBound_, AdvancedController* pBc_)
        :dRate(dRate_), rBound(rBound_), pBc(pBc_), seq(pBc_->imgPr) {}

    /*virtual*/ void Update()
    {
        if(double(rand())/RAND_MAX < dRate)
        {
            Point p; fPoint v(.5, 0);
            
            if(rand()%2)
                p = Point(0, rBound.sz.y / 4);
            else
                p = Point(0, rBound.sz.y * 3 / 4);
            
            if(rand()%2)
            {
                p.x = rBound.sz.x  - 1;
                v = fPoint(-.5, 0);
            }

            p += rBound.p;
            
            Princess* pCr = new Princess(Critter(7, p, v, rBound, 0, seq, true), pBc);
            pCr->Add();
        }
    }
};



struct Dragon: public Critter
{
    bool bFly;
    bool bCarry;
    bool bPrincess;
    unsigned imgCarry;
    
    AdvancedController* pAd;
    
    ImageSequence imgStable;
    ImageSequence imgFly;

    Dragon(Point p, AdvancedController* pAd_):pAd(pAd_),
        imgStable(pAd_->pGl->pDr, "stable.txt", "dragon\\"),
        imgFly(pAd_->pGl->pDr, "fly.txt", "dragon\\"),
        Critter(10, p, Point(), pAd_->rBound, 6, imgStable, 5U),
        bFly(false), bCarry(false), bPrincess(false)
    {
        SimpleVisualEntity::seq = imgStable;
        Critter::bDieOnExit = false;
    }

    /*virtual*/ void Draw (Drawer* pDr)
    {
        if(bCarry)
            pDr->Draw(imgCarry, GetPosition(), true);

        Critter::Draw(pDr);
    }

    void Toggle()
    {
        for(unsigned i = 0; i < pAd->vCs.size(); ++i)
            if(this->HitDetection(pAd->vCs[i]))
            {
                bFly = !bFly;
                if(bFly)
                {
                    SimpleVisualEntity::seq = imgFly;
                    SimpleVisualEntity::dPriority = 10;
                }
                else
                {
                    if(bPrincess)
                        ++pAd->vCs[i]->nPrincesses;

                    bCarry = false;
                    bPrincess = false;

                    
                    SimpleVisualEntity::dPriority = 6;

                    SimpleVisualEntity::seq = imgStable;
                    Critter::fPos = pAd->vCs[i]->fPos;
                    Critter::fVel = Point();
                }

                return;
            }

        if(!bFly)
            return;

        for(std::list<ConsumableEntity*>::iterator itr = pAd->lsPpl.begin(); itr != pAd->lsPpl.end();)
        {
            if(!(*itr)->bExist)
            {
                pAd->lsPpl.erase(itr++);
                continue;
            }

            if (this->HitDetection(*itr))
            {
                if(bCarry)
                {
                    bCarry = false;
                    bPrincess = false;
                }
                else
                {
                    bCarry = true;
                    imgCarry = (*itr)->GetImage();
                    if((*itr)->GetType() == 'P')
                        bPrincess = true;
                }

                (*itr)->bExist = false;

                return;
            }
            ++itr;
        }

        if(bCarry)
        {
            bPrincess = false;
            bCarry = false;
        }
    }
};

struct AlmostBasicController: public BasicController
{
    AlmostBasicController(const BasicController& b): BasicController(b){}

    /*virtual*/ void OnKey(char c, bool bUp)
    {
        if(bUp)
            return;
        
        if(pGl->nActive == pGl->vCnt.size() - 1)
        {
            pGl->nActive = 0;
            delete pGl->vCnt[1];
            pGl->vCnt[1] = new AdvancedController(pGl, rBound, Color(0, 0, 0));
            delete pGl->vCnt[2];
            pGl->vCnt[2] = new AdvancedController(pGl, rBound, Color(0, 0, 0), 2);
        }
        else
            ++pGl->nActive;
    }
};