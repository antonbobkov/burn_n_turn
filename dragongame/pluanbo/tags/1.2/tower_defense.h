#include "GuiGen.h"

#include <vector>
#include <set>
#include <map>
#include <string>

#include <iostream>
#include <sstream>
#include <fstream>

#include<stdlib.h>
#include<math.h>

#include "SDL.h"
#include "Preloader.h"

#pragma warning (disable : 4250)

using namespace Gui;

typedef Gui::GraphicalInterface<IndexImg> Graphic;

struct GlobalController;


struct fPoint
{
    float x,y;

    fPoint():x(0), y(0){}
    fPoint(float x_, float y_):x(x_), y(y_){}

    fPoint(Point p): x(float(p.x)), y(float(p.y)){}

    operator Point() {return Point(int(x), int(y));}

    void Normalize(float f = 1)
    {
        if(x == 0 && y == 0)
            return;
        
        float d = sqrt(x * x + y * y);
        x /= d;
        y /= d;
        x *= f;
        y *= f;
    }

    float Length()
    {
        return sqrt(x*x + y*y);
    }
};

inline fPoint& operator += (fPoint& f1, const fPoint& f2){f1.x += f2.x; f1.y += f2.y; return f1;}
inline fPoint& operator -= (fPoint& f1, const fPoint& f2){f1.x -= f2.x; f1.y -= f2.y; return f1;}
inline fPoint operator + (const fPoint& f1, const fPoint& f2){return fPoint(f1) += f2;}
inline fPoint operator - (const fPoint& f1, const fPoint& f2){return fPoint(f1) -= f2;}
inline bool operator == (const fPoint& f1, const fPoint& f2){return (f1.x == f2.x) && (f1.y == f2.y);}
inline bool operator != (const fPoint& f1, const fPoint& f2){return (f1.x != f2.x) || (f1.y != f2.y);}


struct Polar
{
	float r,a;
	Polar():r(0),a(0){}
	Polar(float a_, float r_):a(a_),r(r_){}
	Polar(fPoint p):r(p.Length())
	{
		if(p.y == 0 && p.x == 0)
			a = 0;
		else
			a = atan2(p.y,p.x);
	}
	Polar operator*(Polar p) { return Polar(a+p.a, r*p.r); }
	
	fPoint TofPoint(){ return fPoint(r*cos(a),r*sin(a)); }
};

inline fPoint GetWedgeAngle(fPoint fDir, float dWidth, unsigned nWhich, unsigned nHowMany)
{
    if(nHowMany == 1)
        return fDir;
    
    float d = 3.1415F * 2 * dWidth / (nHowMany - 1) * nWhich;

    return (Polar(fDir) * Polar(d - 3.1415F * dWidth, 1)).TofPoint();
}

inline fPoint RandomAngle(fPoint fDir, float fRange)
{
    return (Polar(fDir) * Polar((float(rand())/RAND_MAX - .5F)*fRange * 2 * 3.1415F, 1)).TofPoint();
}

struct Drawer: virtual public Smart
{
    SP<Graphic> pGr;
    virtual void Draw(IndexImg nImg, Point p, bool bCentered = true)=0;
};

struct ScalingDrawer: public Drawer
{
    unsigned nFactor;
    Color cTr;

    ScalingDrawer(SP<Graphic> pGr_, unsigned nFactor_, Color cTr_ = Color(0, 255, 255))
        :nFactor(nFactor_), cTr(cTr_){pGr = pGr_;}

    void Scale(IndexImg& pImg, int nFactor_ = -1)
    {
        if(nFactor_ < 0)
            nFactor_ = nFactor;
        
        Image* pOrig = pGr->GetImage(pImg);
        IndexImg pRet = pGr->GetBlankImage(Size(pOrig->GetSize().x * nFactor_, pOrig->GetSize().y * nFactor_));
        Image* pFin = pGr->GetImage(pRet);
        
        Point p, s;
        for(p.y = 0; p.y < pOrig->GetSize().y; ++p.y)
        for(p.x = 0; p.x < pOrig->GetSize().x; ++p.x)
        for(s.y = 0; s.y < nFactor_; ++s.y)
        for(s.x = 0; s.x < nFactor_; ++s.x)
            pFin->SetPixel(Point(p.x * nFactor_ + s.x, p.y * nFactor_ + s.y), pOrig->GetPixel(p));

        pImg = pRet;
    }

    /*virtual*/ void Draw(IndexImg nImg, Point p, bool bCentered = true)
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

    /*virtual*/ IndexImg LoadImage(std::string strFile)
    {
        IndexImg n = pGr->LoadImage(strFile);
        pGr->GetImage(n)->ChangeColor(Color(0,0,0), Color(0,0,0,0));
        return n;
    }
};

template<class T>
void CleanUp(std::list<T>& ar)
{
    for(std::list<T>::iterator itr = ar.begin(), etr = ar.end(); itr != etr;)
    {
        if(!(*itr)->bExist)
            ar.erase(itr++);
        else
            ++itr;
    }
}

struct GameController: virtual public Smart
{
    SP<GlobalController> pGl;

    Rectangle rBound;

    GameController(SP<GlobalController> pGl_, Rectangle rBound_ = Rectangle()):pGl(pGl_), rBound(rBound_){}

    virtual void Update(){};
    virtual void OnKey(int c, bool bUp){};
    virtual void OnMouse(Gui::Point pShift){};
};


struct NumberDrawer: virtual public Smart
{
    SP<ScalingDrawer> pDr;
    std::vector<IndexImg> vImg;

    NumberDrawer(SP<ScalingDrawer> pDr_):pDr(pDr_)
    {
        IndexImg nImg = pDr->LoadImage("numbers.bmp");
        Image*   pImg = pDr->pGr->GetImage(nImg);
        for(unsigned i = 0; i <= 9; ++i)
        {
            IndexImg nCurr = pDr->pGr->GetBlankImage(Size(3, 5));
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

struct GlobalController: virtual public Smart
{
    std::vector< SP<GameController> > vCnt;
    unsigned nActive;

    SP<Graphic> pGraph;
    SP<ScalingDrawer> pDr;
    SP<NumberDrawer> pNum;

    MyPreloader pr;

    GlobalController(SP<ScalingDrawer> pDr_, SP<NumberDrawer> pNum_);

};

struct SimpleController: public GameController
{
    IndexImg nImage;

    SimpleController(SP<GlobalController> pGraph, std::string strFileName):GameController(pGraph)
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

    /*virtual*/ void OnKey(int c, bool bUp)
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
    IndexImg nImage, nText;
    unsigned nTimer;
    bool bShow;

    FlashingController(SP<GlobalController> pGraph, std::string strFileName, std::string strTextName)
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

    /*virtual*/ void OnKey(int c, bool bUp)
    {
        if(bUp)
            return;
        
        if(pGl->nActive == pGl->vCnt.size() - 1)
            pGl->nActive = 0;
        else
            ++pGl->nActive;
    }
};

struct Entity: virtual public Smart
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
    virtual void Draw(SP<ScalingDrawer> pDr){}
    virtual float GetPriority(){return 0;}
};

struct Timer: virtual public Smart
{
    unsigned nTimer, nPeriod;

    Timer(unsigned nPeriod_ = 1):nTimer(0), nPeriod(nPeriod_){}

    bool Tick(){return (++nTimer % nPeriod == 0);}
    bool Check(){return (nTimer % nPeriod == 0);}
};



struct SimpleVisualEntity : virtual public EventEntity, public VisualEntity
{
    float dPriority;

    Timer t;
    
    bool bTimer, bStep, bCenter;

    Point pPrev;

    ImageSequence seq;


    SimpleVisualEntity(float dPriority_, const ImageSequence& seq_, bool bCenter_, unsigned nPeriod) 
        : dPriority(dPriority_), seq(seq_), t(nPeriod), bCenter(bCenter_),
        bTimer(true), bStep(false) 
    {}
    
    SimpleVisualEntity(float dPriority_, const ImageSequence& seq_, bool bCenter_, bool bStep_ = false) 
        : dPriority(dPriority_), seq(seq_), bCenter(bCenter_),
        bTimer(false), bStep(bStep_) 
    {}

    /*virtual*/ void Draw(SP<ScalingDrawer> pDr)
    {
        pDr->Draw(seq.GetImage(), GetPosition(), bCenter);
    }

    /*virtual*/ float GetPriority() { return dPriority; }

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
    
    Animation(float dPriority_, const ImageSequence& seq, unsigned nTimeMeasure_, Point p, bool bCenter = false)
        :SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p){}

    /*virtual*/ Point GetPosition(){return pos;}
};

struct AnimationOnce: public SimpleVisualEntity
{
    Point pos;
    bool bOnce;
    
    AnimationOnce(float dPriority_, const ImageSequence& seq, unsigned nTimeMeasure_, Point p, bool bCenter = false)
        :SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p), bOnce(true){}

    /*virtual*/ Point GetPosition(){return pos;}
    /*virtual*/ void Update()
    {
        SimpleVisualEntity::Update();

        if(SimpleVisualEntity::seq.nActive == 0)
        {
            if(!bOnce)
                bExist = false;
        }
        else
        {
            bOnce = false;
        }
    }
};

struct StaticImage: public VisualEntity
{
    IndexImg img;
    float dPriority;
    Point p;

    StaticImage(IndexImg img_, Point p_ = Point(0,0), float dPriority_ = 0)
        :img(img_), dPriority(dPriority_), p(p_){}

    /*virtual*/ void Draw(SP<ScalingDrawer> pDr)
    {
        pDr->Draw(img, GetPosition(), false);
    }

    /*virtual*/ Point GetPosition()
    {
        return p;
    }

    /*virtual*/ float GetPriority(){return dPriority;}
};

struct Countdown: public VisualEntity, public EventEntity
{
    SP<NumberDrawer> pNum;
    unsigned nTime, nCount;

    Countdown(SP<NumberDrawer> pNum_, unsigned nTime_):pNum(pNum_), nTime(nTime_), nCount(0){}
    
    /*virtual*/ void Update()
    {
        ++nCount;
        if(nCount % 10 == 0)
            --nTime;
        if(nTime == 0)
            bExist = false;
    }

    /*virtual*/ void Draw(SP<ScalingDrawer> pDr)
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
    
    bool HitDetection(SP<PhysicalEntity> pPh)
    {
        Point d = GetPosition() - pPh->GetPosition();
        unsigned r1 = GetRadius(), r2 = pPh->GetRadius();
        return unsigned( d.x * d.x + d.y * d.y ) < ( r1 * r1 + r2 * r2 );
    }
};

struct TrackballTracker
{
	std::list<Point> lsMouse;
	int nMaxLength;
	bool trigFlag;
	int threshold;
	TrackballTracker()
	{
		nMaxLength = 2;
		for(int i = 0; i < nMaxLength; i++)
			lsMouse.push_back(Point(0,0));
		trigFlag = false;
		threshold = 25;
	}

	void Update()
	{
		Point p;
		SDL_GetRelativeMouseState(&p.x,&p.y);
		
        //std::cout << p.x << " " << p.y << "\n";
        
        lsMouse.push_front(p);
		lsMouse.pop_back();
	}

	bool IsTrigger()
	{
		int p = GetDerivative();
		if(!trigFlag && p >= threshold*threshold)
		{
			trigFlag = true;
			return true;
		}
		else if(trigFlag && p < threshold*threshold)
		{
			trigFlag = false;
		}

		return false;
	}

	Point GetMovement()
	{
		return lsMouse.front();
	}

	fPoint GetAvMovement()
	{
        fPoint p;
        for(std::list<Point>::iterator itr = lsMouse.begin(), etr = lsMouse.end(); itr != etr; ++itr)
            p += *itr;

        p.x /= lsMouse.size();
        p.y /= lsMouse.size();

        return p;
	}

    int GetLengthSq(Point p)
    {
        return p.x*p.x + p.y*p.y;
    }

	int GetDerivative()
	{
		if(nMaxLength > 1)
			return GetLengthSq(lsMouse.front()) - GetLengthSq(*++lsMouse.begin());
		else
			return 0;
	}



};

struct ConsumableEntity: virtual public PhysicalEntity
{
    virtual char GetType()=0;
    virtual void OnHit(){bExist = false;}
    virtual IndexImg GetImage()=0;
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
            if(bDieOnExit)
                bExist = false;
            else
            {
                if(InsideRectangle(rBound, fPos - fPoint(0,fVel.y)))
                    fPos.y -= fVel.y;
                else if(InsideRectangle(rBound, fPos - fPoint(fVel.x, 0)))
                    fPos.x -= fVel.x;
                else
                    fPos -= fVel;
            }
        }
    }

    Critter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
        float dPriority, const ImageSequence& seq, unsigned nPeriod)
        :SimpleVisualEntity(dPriority, seq, true, nPeriod), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true){} 
    
    Critter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
        float dPriority, const ImageSequence& seq, bool bStep = false)
        :SimpleVisualEntity(dPriority, seq, true, true), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true){} 
};



struct BasicController: public GameController
{
    std::list< SP<VisualEntity> > lsDraw;
    std::list< SP<EventEntity> >  lsUpdate;
    std::list< SP<ConsumableEntity> >  lsPpl;

    void AddV(SP<VisualEntity> pVs){lsDraw.push_back(pVs);}
    void AddE(SP<EventEntity> pEv){lsUpdate.push_back(pEv);}
    
    template<class T>
    void AddBoth(T t)
    {
        lsDraw.push_back(t);
        lsUpdate.push_back(t);
    }

    void AddBackground(Color c)
    {
        IndexImg nBckImg = pGl->pDr->pGr->GetBlankImage(rBound.sz);
        pGl->pDr->pGr->RectangleOnto(nBckImg, rBound.sz, c);
        pGl->pDr->Scale(nBckImg);
        
        SP<StaticImage> pBkg = new StaticImage(nBckImg, rBound.p, -1);

        AddV(pBkg);
    }

    BasicController(SP<GlobalController> pGl_, Rectangle rBound, Color c):GameController(pGl_, rBound)
    {
        AddBackground(c);
    }

    /*virtual*/ void Update()
    {
        CleanUp(lsUpdate);
        CleanUp(lsDraw);
        CleanUp(lsPpl);

        std::list< SP<EventEntity> >::iterator itr;
        for(itr = lsUpdate.begin(); itr != lsUpdate.end(); ++itr)
        {
            if(!(*itr)->bExist)
                continue;
            (*itr)->Move();
        }

        for(itr = lsUpdate.begin(); itr != lsUpdate.end(); ++itr)
        {
            if(!(*itr)->bExist)
                continue;

            (*itr)->Update();
        }
 
        {
            std::list< SP<VisualEntity> >::iterator itr;
                
            std::multimap<float, SP<VisualEntity> > mmp;
            
            for(itr = lsDraw.begin(); itr != lsDraw.end(); ++itr)
            {
                if(!(*itr)->bExist)
                    continue;

                mmp.insert(std::pair<float, SP<VisualEntity> > ((*itr)->GetPriority(), *itr));
            }


            for(std::multimap<float, SP<VisualEntity> >::iterator
                mitr = mmp.begin(), metr = mmp.end(); mitr != metr; ++mitr)
                mitr->second->Draw(pGl->pDr);
        }

        pGl->pGraph->RefreshAll();
    }

    /*virtual*/ void OnKey(int c, bool bUp)
    {
        if(bUp)
            return;
        
        if(pGl->nActive == pGl->vCnt.size() - 1)
            pGl->nActive = 0;
        else
            ++pGl->nActive;
    }

};

template<class T>
void Union(std::map<std::string, T>& TarMap, const std::map<std::string, T>& srcMap)
{
    for(std::map<std::string, T>::const_iterator itr = srcMap.begin(), etr = srcMap.end();
        itr != etr; ++itr)
        TarMap[itr->first] += itr->second; 
}

inline void Union(std::map<std::string, bool>& TarMap, const std::map<std::string, bool>& srcMap)
{
    for(std::map<std::string, bool>::const_iterator itr = srcMap.begin(), etr = srcMap.end();
        itr != etr; ++itr)
        TarMap[itr->first] |= itr->second; 
}

template<class T>
std::ostream& Out(std::ostream& ofs, const std::map<std::string, T>& srcMap)
{
    for(std::map<std::string, T>::const_iterator itr = srcMap.begin(), etr = srcMap.end();
        itr != etr; ++itr)
        ofs << itr->first << " = " << itr->second << "; ";
    return ofs;
}

struct FireballBonus: virtual public Entity
{
    std::map<std::string, float> fMap; 
    std::map<std::string, unsigned> uMap; 
    std::map<std::string, bool> bMap;

    FireballBonus(bool bDef = false)
    {
        if(bDef)
        {
            fMap["speed"] = 3;    
            uMap["pershot"] = 1;    
            uMap["total"] = 5;    
        }
    }

    void Add(std::string str, float f){fMap[str] += f;}
    void Add(std::string str, unsigned u){uMap[str] += u;}
    void Add(std::string str, bool b){bMap[str] |= b;}

    template<class T>
    FireballBonus(std::string str, T t){Add(str, t);}
    
    FireballBonus& operator += (const FireballBonus& f)
    {
        Union(fMap, f.fMap);
        Union(uMap, f.uMap);
        Union(bMap, f.bMap);

        return *this; 
    }

};

inline std::ostream& operator << (std::ostream& ofs, FireballBonus b)
{
    Out(ofs, b.fMap) << "\n";
    Out(ofs, b.uMap) << "\n";
    Out(ofs, b.bMap) << "\n";

    return ofs;
}

struct Chain
{
    bool bInfinite;
    unsigned nGeneration;

    Chain(bool bInfinite_ = false):bInfinite(bInfinite_), nGeneration(0){}
    Chain(unsigned nGeneration_):bInfinite(false), nGeneration(nGeneration_){}

    Chain Evolve()
    {
        if(bInfinite)
            return Chain(true);
        else if(nGeneration == 0)
            return Chain();
        else
            return Chain(nGeneration - 1);
    }

    bool IsLast()
    {
        return (!bInfinite) && (nGeneration == 0);
    }
};

inline ImageSequence Reset(ImageSequence imgSeq)
{
    imgSeq.nActive = 0;
    return imgSeq;
}

struct ChainExplosion: virtual public AnimationOnce, virtual public PhysicalEntity
{
    float r_in, r;
    float delta;

    Chain ch;

    SP<BasicController> pBc;
    
    ChainExplosion(const AnimationOnce& av, float r_, float delta_, SP<BasicController> pBc_, Chain ch_ = Chain())
        :AnimationOnce(av), r(r_), r_in(r_), delta(delta_), pBc(pBc_), ch(ch_){}

    /*virtual*/ unsigned GetRadius(){return unsigned(r);}
    /*virtual*/ void Update()
    {
        if(SimpleVisualEntity::t.Check())
        {
            r += delta;
        }
        
        CleanUp(pBc->lsPpl);
        
        for(std::list< SP<ConsumableEntity> >::iterator itr = pBc->lsPpl.begin(); itr != pBc->lsPpl.end(); ++itr)
        {
            if(!(*itr)->bExist)
                continue;

            if (this->HitDetection(*itr))
            {
                (*itr)->OnHit();

                if(!ch.IsLast())
                {
                    SP<ChainExplosion> pCE = new ChainExplosion(
                        AnimationOnce(GetPriority(), Reset(seq),
                        SimpleVisualEntity::t.nPeriod, (*itr)->GetPosition(), true)
                        , r_in, delta, pBc, ch.Evolve());
                    pBc->AddBoth(pCE);
                }
            }
        }

        AnimationOnce::Update();
    }

    void Draw(SP<ScalingDrawer> pDr)
    {
        AnimationOnce::Draw(pDr);
    }
};

inline unsigned DiscreetAngle(float a, unsigned nDiv)
{
    return unsigned((- a / 2 / 3.1415 + 2 - 1.0/4 + 1.0 / 2 / nDiv) * nDiv)%nDiv;
}


struct Fireball: public Critter
{
    SP<BasicController> pBc;
    bool bThrough;
    FireballBonus fb;
    
    Chain ch;
    unsigned nChain;

    Fireball(Point p, fPoint v, SP<BasicController> pBc_, FireballBonus& fb_,
        Chain ch_ = Chain(), unsigned nChain_ = 1)
        :Critter(5, p, v, pBc_->rBound, !(fb_.bMap["big"]) ? 5.F : 10.F, 
        ImageSequence(), 1U),
        pBc(pBc_), fb(fb_), ch(ch_), nChain(nChain_)
    {
        Critter::fVel.Normalize(fb.fMap["speed"]);

        if(!fb.bMap["through"])
            if(fb.bMap["big"])
                Critter::seq = pBc->pGl->pr("fireball_big");
            else
                Critter::seq = pBc->pGl->pr("fireball");
        else
        {
            Polar pol(Critter::fVel);
            unsigned n = DiscreetAngle(pol.a, 16);
            if(fb.bMap["big"])
                Critter::seq = ImageSequence(pBc->pGl->pr("laser_big").vImage[n]);
            else
                Critter::seq = ImageSequence(pBc->pGl->pr("laser").vImage[n]);
        }
    }

    /*virtual*/ void Update()
    {
        CleanUp(pBc->lsPpl);
        
        for(std::list< SP<ConsumableEntity> >::iterator itr = pBc->lsPpl.begin(); itr != pBc->lsPpl.end(); ++itr)
        {
            if(!(*itr)->bExist)
                continue;

            if (this->HitDetection(*itr))
            {
                if(!fb.bMap["through"])
                    bExist = false;
                
                (*itr)->OnHit();

                if(!ch.IsLast())
                {
                    fPoint v = RandomAngle(fVel, 1.F/12);

                    for(unsigned i = 0; i < nChain; ++i)
                    {
                        SP<Fireball> pFb = new Fireball(GetPosition(),
                            GetWedgeAngle(v, 1.F/6, i, nChain), pBc, fb, ch.Evolve(), nChain);
                        pBc->AddBoth(pFb);
                    }
                }

                if(fb.uMap["explode"] > 0)
                {
                    SP<ChainExplosion> pEx;
                    if(!fb.bMap["through"] && !fb.bMap["big"])
                    {
                        pEx = new ChainExplosion(
                            AnimationOnce(GetPriority(),
                            pBc->pGl->pr("explosion"),
                            1U, GetPosition(), true), 5, 2.5F, pBc, Chain(fb.uMap["explode"] - 1));
                    }
                    if(!fb.bMap["through"] && fb.bMap["big"])
                    {
                        pEx = new ChainExplosion(
                            AnimationOnce(GetPriority(),
                            pBc->pGl->pr("explosion_big"),
                            1U, GetPosition(), true), 10, 5.F, pBc, Chain(fb.uMap["explode"] - 1));
                    }
                    else if(fb.bMap["through"] && !fb.bMap["big"])
                    {
                        pEx = new ChainExplosion(
                            AnimationOnce(GetPriority(),
                            pBc->pGl->pr("laser_expl"),
                            1U, GetPosition(), true), 3, 3.9F, pBc, Chain(fb.uMap["explode"] - 1));
                    }
                    else if(fb.bMap["through"] && fb.bMap["big"])
                    {
                        pEx = new ChainExplosion(
                            AnimationOnce(GetPriority(),
                            pBc->pGl->pr("laser_expl_big"),
                            1U, GetPosition(), true), 6, 7.8F, pBc, Chain(fb.uMap["explode"] - 1));
                    }
                    pBc->AddBoth(pEx);
                }
            }
        }

        Critter::Update();
    }
};


struct AdvancedController;
struct Dragon;

struct Castle: public Critter
{
    unsigned nPrincesses;
    SP<AdvancedController> pAv;
    SP<Dragon> pDrag;
    
    Castle(Point p, Rectangle rBound_, SP<AdvancedController> pAv_);

    void OnKnight();

    /*unsigned*/ void Draw(SP<ScalingDrawer> pDr);
};

inline Point Center(Size sz){return Point(sz.x/2, sz.y/2);}



struct CritterGenerator;
struct Dragon;

struct Road: virtual public VisualEntity
{
    bool bVertical;
    unsigned nCoord;
    Rectangle rBound;

    Road(bool bVertical_, unsigned nCoord_, Rectangle rBound_)
        :bVertical(bVertical_), nCoord(nCoord_), rBound(rBound_){}

    /*virtual*/ float GetPriority(){return 0;}
    /*virtual*/ Point GetPosition(){return Point();}

    /*virtual*/ void Draw(SP<ScalingDrawer> pDr)
    {
        unsigned n = pDr->nFactor;
        if(bVertical)
            pDr->pGr->DrawRectangle
            (Rectangle((nCoord - 5) * n, rBound.p.y * n, (nCoord + 5) * n, rBound.sz.y * n),
            Color(63, 63, 63), false);
        else
            pDr->pGr->DrawRectangle
            (Rectangle(rBound.p.x * n, (nCoord - 5) * n, rBound.sz.x * n, (nCoord + 5) * n),
            Color(63, 63, 63), false);
    }

    void RoadMap(Point& p, Point& v)
    {
        if(rand()%2)
        {
            if(!bVertical)
            {
                p.y = nCoord - 7;
                p.x = rBound.p.x;
                v = Point(1, 0);
            }
            else
            {
                p.x = nCoord;
                p.y = rBound.p.y;
                v = Point(0, 1);
            }
        }
        else
        {
            if(!bVertical)
            {
                p.y = nCoord - 7;
                p.x = rBound.sz.x - 1;
                v = Point(-1, 0);
            }
            else
            {
                p.x = nCoord;
                p.y = rBound.sz.y - 1;
                v = Point(0, -1);
            }
        }
    }
};


struct AdvancedController: public BasicController
{
    std::vector< SP<Castle> > vCs;
    std::vector< SP<Road> > vRd;
    std::vector< SP<Dragon> > vDr;
    TrackballTracker tr;

    bool bCh;

    AdvancedController(SP<GlobalController> pGl_, Rectangle rBound, Color c, int lvl = 1);
    ~AdvancedController()
    {
        std::cout << "~AdvancedController\n";
    }

    /*virtual*/ void OnKey(int c, bool bUp);

    /*virtual*/ void Update();

};

struct Princess:  public Critter, public ConsumableEntity
{
    SP<AdvancedController> pAc;

    Princess(const Critter& cr, SP<AdvancedController> pAc_):Critter(cr), pAc(pAc_){}

    /*virtual*/ IndexImg GetImage()
    {
        return seq.vImage[0];
    }

    /*virtual*/ char GetType(){return 'P';}

    /*virtual*/ void OnHit()
    {
        bExist = false;

        SP<AnimationOnce> pAn = new AnimationOnce(GetPriority(),
            fVel.x < 0 ? pAc->pGl->pr("princess_die_f") : pAc->pGl->pr("princess_die")
            , 2U, GetPosition(), true);

        pAc->AddBoth(pAn);
    }
};

struct Trader:  public Critter, public ConsumableEntity
{
    SP<AdvancedController> pAc;

    Trader(const Critter& cr, SP<AdvancedController> pAc_):Critter(cr), pAc(pAc_){}

    /*virtual*/ IndexImg GetImage()
    {
        return seq.vImage[0];
    }

    /*virtual*/ char GetType(){return 'T';}

    /*virtual*/ void OnHit()
    {
        bExist = false;

        SP<AnimationOnce> pAn = new AnimationOnce(GetPriority(),
            fVel.x < 0 ? pAc->pGl->pr("trader_die") : pAc->pGl->pr("trader_die_f")
            , 2U, GetPosition(), true);

        pAc->AddBoth(pAn);
    }
};

struct Knight: public Critter, public ConsumableEntity
{
    SP<AdvancedController> pAc;
    
    Knight(const Critter& cr, SP<AdvancedController> pAc_):Critter(cr), pAc(pAc_){}

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

    /*virtual*/ void OnHit()
    {
        bExist = false;

        SP<AnimationOnce> pAn = new AnimationOnce(dPriority,
            pAc->pGl->pr("knight_die"),
            2U, GetPosition(), true);
        pAc->AddBoth(pAn);
    }

    /*virtual*/ IndexImg GetImage()
    {
        return seq.vImage[0];
    }

    /*virtual*/ char GetType(){return 'K';}
};


struct CritterGenerator: virtual public EventEntity
{
    float dRate;
    Rectangle rBound;
    SP<AdvancedController> pBc;
    ImageSequence seq;

    CritterGenerator(float dRate_, Rectangle rBound_, SP<AdvancedController> pBc_)
        :dRate(dRate_), rBound(rBound_), pBc(pBc_), seq(pBc_->pGl->pr("knight")) {}

    /*virtual*/ void Update()
    {
        if(float(rand())/RAND_MAX < dRate)
        {
            Point p;
            
            float d = float(rand())/RAND_MAX;
            if(rand()%2)
            {
                p.x = int(rBound.sz.x * d);
                if(rand()%2)
                    p.y = 0;
                else
                    p.y = rBound.sz.y - 1;
            }
            else
            {
                p.y = int(rBound.sz.y * d);
                if(rand()%2)
                    p.x = 0;
                else
                    p.x = rBound.sz.x - 1;
            }
            
            unsigned n = unsigned(rand()%pBc->vCs.size());
            
            fPoint v = pBc->vCs[n]->GetPosition() - p;
            v.Normalize();
            v.x *= .75;
            v.y *= .75;
            p += rBound.p;
            
            SP<Knight> pCr = new Knight(Critter(7, p, v, rBound, 3, seq, true), pBc);
            pBc->AddBoth(pCr);
            pBc->lsPpl.push_back(pCr);
        }
    }
};

struct PrincessGenerator: virtual public EventEntity
{
    float dRate;
    Rectangle rBound;
    SP<AdvancedController> pBc;

    PrincessGenerator(float dRate_, Rectangle rBound_, SP<AdvancedController> pBc_)
        :dRate(dRate_), rBound(rBound_), pBc(pBc_) {}

    /*virtual*/ void Update()
    {
        if(float(rand())/RAND_MAX < dRate)
        {
            Point p, v;
            
            pBc->vRd[rand()%pBc->vRd.size()]->RoadMap(p, v);

            fPoint vel(v);
            vel.Normalize(.5);
            
            SP<Princess> pCr = new Princess(Critter(7, p, vel, rBound, 3,
                vel.x < 0 ? pBc->pGl->pr("princess_f") :  pBc->pGl->pr("princess")
                , true), pBc);
            pBc->AddBoth(pCr);
            pBc->lsPpl.push_back(pCr);
        }
    }
};

struct TraderGenerator: virtual public EventEntity
{
    float dRate;
    Rectangle rBound;
    SP<AdvancedController> pBc;

    TraderGenerator(float dRate_, Rectangle rBound_, SP<AdvancedController> pBc_)
        :dRate(dRate_), rBound(rBound_), pBc(pBc_) {}

    /*virtual*/ void Update()
    {
        if(float(rand())/RAND_MAX < dRate)
        {
            Point p, v;
            
            pBc->vRd[rand()%pBc->vRd.size()]->RoadMap(p, v);

            fPoint vel(v);
            vel.Normalize(1);
            
            SP<Trader> pCr = new Trader(Critter(7, p, v, rBound, 3,
                vel.x < 0 ? pBc->pGl->pr("trader") :  pBc->pGl->pr("trader_f")
                , true), pBc);
            pBc->AddBoth(pCr);
            pBc->lsPpl.push_back(pCr);
        }
    }
};



struct TimedFireballBonus: public FireballBonus, virtual public EventEntity
{
    Timer t;

    TimedFireballBonus(const FireballBonus& fb, unsigned nPeriod):FireballBonus(fb), t(nPeriod){} 

    /*virtual*/ void Update()
    {
        if(t.Tick())
            bExist = false;
    }
};

struct DragonLeash
{
	fPoint lastVel;
	//tilt in radians per frame
	float tilt;
	float speed; //in pixels/frame
	float trackballScaleFactor;
	float naturalScaleFactor;
    float maxTilt;
	
    
    DragonLeash():lastVel(0,-1)
	{
		tilt = 0;
		speed = 6;
		trackballScaleFactor = float(1)/200;
		naturalScaleFactor = .25F;
        maxTilt = .19F;
	}

	void ModifyTilt(Point trackball)
	{
		tilt -= tilt*naturalScaleFactor;
		tilt += trackball.x*trackballScaleFactor;

        if(tilt > maxTilt)
            tilt = maxTilt;
        if(tilt < -maxTilt)
            tilt = -maxTilt;
	}

	fPoint GetNewVelocity(Point trackball)
	{
		ModifyTilt(trackball);
		Polar p = Polar(lastVel);
        p.r = speed;
		p.a += tilt;
		lastVel = p.TofPoint();
		return lastVel;
	}
};

struct ButtonSet
{
    std::vector<int> vCodes;

    
    ButtonSet(int q, int w, int e, int d, int c, int x, int z, int a, int sp)
    {
        vCodes.push_back(q);
        vCodes.push_back(w);
        vCodes.push_back(e);
        vCodes.push_back(d);
        vCodes.push_back(c);
        vCodes.push_back(x);
        vCodes.push_back(z);
        vCodes.push_back(a);
        vCodes.push_back(sp);
    }

    bool IsSpace(int nCode)
    {
        return nCode == vCodes[8];
    }

    Point GetPoint(int nCode)
    {
        Point p = Point();

        if(nCode == vCodes[0]) p = Point(-1, -1);
        if(nCode == vCodes[1]) p = Point( 0, -1);
        if(nCode == vCodes[2]) p = Point( 1, -1);
        if(nCode == vCodes[3]) p = Point( 1,  0);
        if(nCode == vCodes[4]) p = Point( 1,  1);
        if(nCode == vCodes[5]) p = Point( 0,  1);
        if(nCode == vCodes[6]) p = Point(-1,  1);
        if(nCode == vCodes[7]) p = Point(-1,  0);

        return p;
    }
};

struct Dragon: public Critter
{
    std::list< SP<FireballBonus> > lsBonuses;
    std::list< SP<Fireball> > lsBalls;

    DragonLeash leash;

    FireballBonus GetAllBonuses()
    {
        CleanUp(lsBonuses);
        FireballBonus fbRet(true);

        for(std::list< SP<FireballBonus> >::iterator itr = lsBonuses.begin(), etr = lsBonuses.end();
            itr != etr; ++itr)
            fbRet += **itr;

        return fbRet;
    }
    
    bool bFly;
    bool bCarry;
    char cCarry;
    unsigned nTimer;
    IndexImg imgCarry;

    SP<AdvancedController> pAd;
    
    SP<Castle> pCs;
    
    ImageSequence imgStable;
    ImageSequence imgFly;

    ButtonSet bt;

    Dragon(SP<Castle> pCs_, SP<AdvancedController> pAd_,
        ImageSequence imgStable_, ImageSequence imgFly_, ButtonSet bt_)
        :pAd(pAd_), imgStable(imgStable_), imgFly(imgFly_),
        Critter(10, pCs_ == 0 ? pAd_->vCs[0]->GetPosition() : pCs_->GetPosition(),
        Point(), pAd_->rBound, 1, ImageSequence()),
        bFly(), bCarry(false), cCarry(' '), nTimer(0), pCs(pCs_), bt(bt_)
    {
        if(pCs != 0 && pCs->pDrag == 0)
        {
            pCs->pDrag = this;
            bFly = false;
            Critter::dPriority = 1;
            Critter::fPos = pCs->GetPosition();
        }
        else
        {
            bFly = true;
            Critter::dPriority = 5;
            Critter::fPos = pAd->vCs[0]->GetPosition();
        }

        SimpleVisualEntity::seq = imgStable;
        Critter::bDieOnExit = false;
    }

    /*virtual*/ void Draw (SP<ScalingDrawer> pDr)
    {
        if(bCarry)
            pDr->Draw(imgCarry, GetPosition(), true);

        if(!bFly)
        {
            if(!nTimer)
                pDr->Draw(SimpleVisualEntity::seq.vImage[0], pCs->GetPosition() - Point(0, 22)); 
            else
                pDr->Draw(SimpleVisualEntity::seq.vImage[1], pCs->GetPosition() - Point(0, 22)); 
        }
        else
        {
            Polar p(Critter::fVel);
            pDr->Draw(imgFly.vImage[DiscreetAngle(p.a, 16)], GetPosition());
        }
        
        if (nTimer > 0)
            --nTimer;
    }

    void AddBonus(SP<TimedFireballBonus> pBonus)
    {
        lsBonuses.push_back(pBonus);
        pAd->AddE(pBonus);
    }

    SP<TimedFireballBonus> GetBonus(unsigned n, unsigned nTime = 100U)
    {
        SP<TimedFireballBonus> pBonus;

        if(n == 0)
            pBonus = new TimedFireballBonus(FireballBonus("speed", 3.F), nTime);
        else if(n == 1)
            pBonus = new TimedFireballBonus(FireballBonus("pershot", 2U), nTime);
        else if(n == 2)
            pBonus = new TimedFireballBonus(FireballBonus("through", true), nTime);
        else if(n == 3)
            pBonus = new TimedFireballBonus(FireballBonus("big", true), nTime);
        else if(n == 4)
            pBonus = new TimedFireballBonus(FireballBonus("total", 5U), nTime);
        else if(n == 5)
            pBonus = new TimedFireballBonus(FireballBonus("explode", 1U), nTime);
        else if(n == 6)
        {
            //FireballBonus fb = GetAllBonuses();
            //if(fb.uMap["fireballchain"] == 0 && fb.uMap["fireballchainnum"] == 0)
            //{
                pBonus = new TimedFireballBonus(FireballBonus(), nTime);
                pBonus->Add("fireballchain", 1U);
                pBonus->Add("fireballchainnum", 2U);
            //}
            //else
            //{
            //    if(rand()%2)
            //       pBonus = new TimedFireballBonus(FireballBonus("fireballchain", 1U), nTime);
            //    else
            //        pBonus = new TimedFireballBonus(FireballBonus("fireballchainnum", 1U), nTime);
            //}
        }
        else
        {
            pBonus = new TimedFireballBonus(FireballBonus(), 1);

            CleanUp(pAd->lsPpl);

            for(std::list<SP<ConsumableEntity> >::iterator itr = pAd->lsPpl.begin(),
                etr = pAd->lsPpl.end(); itr != etr; ++itr)
                if((*itr)->GetType() == 'K')
                {
                    (*itr)->OnHit();

                    /*
                    SP<ChainExplosion> pEx = new ChainExplosion(
                        AnimationOnce(5.F,
                        pAd->pGl->pr("explosion"),
                        1U, (*itr)->GetPosition(), true), 5, 2.5, pAd);
                    pAd->AddBoth(pEx);
                    */
                }
        }

        return pBonus;
    }
    
    void RandomBonus(unsigned nTime = 100U)
    {
        unsigned n = rand()%7;
        if(rand()%15 == 0)
            n = 100;
        
        AddBonus(GetBonus(n, nTime));
    }
    
    void Fire(fPoint fDir)
    {
        if(fDir == fPoint())
            return;


        CleanUp(lsBalls);
        
        FireballBonus fb = GetAllBonuses();

        if(fb.uMap["total"] <= lsBalls.size())
            return;

        nTimer = 4;
        
        unsigned nNumber = fb.uMap["pershot"];

        for(unsigned i = 0; i < nNumber; ++i)
        {
            SP<Fireball> pFb =
                new Fireball(GetPosition(), GetWedgeAngle(fDir, 1.F/6, i, nNumber), pAd, fb,
                Chain(fb.uMap["fireballchain"]), fb.uMap["fireballchainnum"]);
            pAd->AddBoth(pFb);
            if(i == nNumber / 2)
                lsBalls.push_back(pFb);
        }
    }

    void Toggle()
    {
        if(!bFly)
        {
            bFly = true;
            
            SimpleVisualEntity::seq = imgFly;
            SimpleVisualEntity::dPriority = 5;

            pCs->pDrag = 0;
            pCs = 0;

            return;
        }
        
        for(unsigned i = 0; i < pAd->vCs.size(); ++i)
            if(this->HitDetection(pAd->vCs[i]))
            {
                if(pAd->vCs[i]->pDrag != 0)
                    continue;

                bFly = false;
                
                pCs = pAd->vCs[i];
                pCs->pDrag = this;

                if(cCarry == 'P')
                    ++pAd->vCs[i]->nPrincesses;
                if(cCarry == 'T')
                    RandomBonus(1000U);

                bCarry = false;
                cCarry = ' ';

                
                SimpleVisualEntity::dPriority = 1;

                SimpleVisualEntity::seq = imgStable;
                Critter::fPos = pAd->vCs[i]->GetPosition();
                Critter::fVel = Point();

                return;
            }


        CleanUp(pAd->lsPpl);

        for(std::list< SP<ConsumableEntity> >::iterator itr = pAd->lsPpl.begin(); itr != pAd->lsPpl.end(); ++itr)
        {
            if(!(*itr)->bExist)
                continue;

            if (this->HitDetection(*itr))
            {
                if(!bCarry)
                {
                    bCarry = true;
                    imgCarry = (*itr)->GetImage();
                    cCarry = (*itr)->GetType();
                }
                else
                {
                    bCarry = false;
                    cCarry = ' ';
                }

                (*itr)->bExist = false;

                return;
            }
        }

        if(bCarry)
        {
            bCarry = false;
            cCarry = ' ';
        }

    }
};

struct AlmostBasicController: public BasicController
{
    AlmostBasicController(const BasicController& b): BasicController(b){}

    /*virtual*/ void OnKey(int c, bool bUp)
    {
        if(bUp)
            return;
        
        if(pGl->nActive == pGl->vCnt.size() - 1)
        {
            pGl->nActive = 0;
            pGl->vCnt[1] = new AdvancedController(pGl, rBound, Color(0, 0, 0));
            pGl->vCnt[2] = new AdvancedController(pGl, rBound, Color(0, 0, 0), 2);
            pGl->vCnt[3] = new AdvancedController(pGl, rBound, Color(0, 0, 0), 3);
        }
        else
            ++pGl->nActive;
    }
};