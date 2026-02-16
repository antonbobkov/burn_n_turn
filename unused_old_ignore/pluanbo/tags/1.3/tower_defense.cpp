#include "tower_defense.h"

GlobalController::GlobalController(SP<ScalingDrawer> pDr_, SP<NumberDrawer> pNum_, SP<SoundManager> pSnd_, FilePath fp)
    :nActive(0), pDr(this, pDr_), pGraph(this, pDr_->pGr), pNum(this, pNum_), pr(pDr_->pGr, fp), pSnd(this, pSnd_), nScore(0)
{
    typedef ImagePainter::ColorMap ColorMap;
    
    std::vector<ColorMap> vColors;
    vColors.push_back(ColorMap(Color(0, 255, 0), Color(0, 0, 255)));
    vColors.push_back(ColorMap(Color(0, 128, 0), Color(0, 0, 128)));
    vColors.push_back(ColorMap(Color(0, 127, 0), Color(0, 0, 127)));
    vColors.push_back(ColorMap(Color(0, 191, 0), Color(0, 0, 191)));
    
    unsigned nScale = 2;
    
    pr.AddTransparent(Color(0,0,0));
    pr.SetScale(nScale);

    pr.LoadTS("road.bmp", "road");
    pr.LoadTS("logomocup.bmp", "logo");
    pr.LoadTS("over.bmp", "over");
    pr.LoadTS("time.bmp", "time");
    pr.LoadTS("score.bmp", "score");
    pr.LoadTS("empty.bmp", "empty");

    pr.LoadSeqTS("bonus\\peter_bonus.txt", "bonus");
    pr.LoadSeqTS("bonus\\burning.txt", "burning_bonus");
    
    pr.LoadSeqTS("start.txt", "start");
    pr.LoadSeqTS("win\\win.txt", "win");
    pr.LoadSeqTS("castle\\castle.txt", "castle");
    pr.LoadSeqTS("castle\\castle.txt", "castle");
    
    pr.LoadSeqTS("dragon_fly\\fly.txt", "dragon_fly");
    pr.LoadSeqTS("dragon\\stable.txt", "dragon_stable");
    
    pr.AddSequence(pr("dragon_fly"), "bdragon_fly");
    pr.AddSequence(pr("dragon_stable"), "bdragon_stable");

    ForEachImage(pr("bdragon_fly"), ImagePainter(pGraph, vColors));
    ForEachImage(pr("bdragon_stable"), ImagePainter(pGraph, vColors));
    
    pr.LoadSeqTS("explosion\\explosion2.txt", "explosion");
    pr.LoadSeqTS("explosion\\laser_expl.txt", "laser_expl");
    
    pr.LoadSeqTS("explosion\\explosion2.txt", "explosion_big", Color(0,0,0,0), nScale*2);
    pr.LoadSeqTS("explosion\\laser_expl.txt", "laser_expl_big", Color(0,0,0,0), nScale*2);
    
    pr.LoadSeqTS("fireball\\fireball.txt", "fireball");
    pr.LoadSeqTS("fireball\\fireball.txt", "fireball_big", Color(0,0,0,0), nScale*2);
    pr.LoadSeqTS("fireball\\laser.txt", "laser");
    pr.LoadSeqTS("fireball\\laser.txt", "laser_big", Color(0,0,0,0), nScale*2);
    
    pr.LoadSeqTS("knight\\knight.txt", "knight");
    pr.LoadSeqTS("knight\\die.txt", "knight_die");
    pr.LoadSeqTS("knight\\fire.txt", "knight_fire");
    
    pr.LoadSeqTS("trader\\trader.txt", "trader");
    pr.LoadSeqTS("trader\\die.txt", "trader_die");
    
    pr.AddSequence(pr("trader"), "trader_f");
    pr.AddSequence(pr("trader_die"), "trader_die_f");
    ForEachImage(pr("trader_f"), ImageFlipper(pGraph));
    ForEachImage(pr("trader_die_f"), ImageFlipper(pGraph));
    
    pr.LoadSeqTS("princess\\princess.txt", "princess");
    pr.LoadSeqTS("princess\\die.txt", "princess_die");

    pr.AddSequence(pr("princess"), "princess_f");
    pr.AddSequence(pr("princess_die"), "princess_die_f");
    ForEachImage(pr("princess_f"), ImageFlipper(pGraph));
    ForEachImage(pr("princess_die_f"), ImageFlipper(pGraph));

    pr.LoadSeqTS("mage\\mage.txt", "mage");
    pr.LoadSeqTS("mage\\spell.txt", "mage_spell");
    pr.LoadSeqTS("mage\\die.txt", "mage_die");

    pr.AddSequence(pr("mage"), "mage_f");
    pr.AddSequence(pr("mage_spell"), "mage_spell_f");
    pr.AddSequence(pr("mage_die"), "mage_die_f");
    ForEachImage(pr("mage_f"), ImageFlipper(pGraph));
    ForEachImage(pr("mage_spell_f"), ImageFlipper(pGraph));
    ForEachImage(pr("mage_die_f"), ImageFlipper(pGraph));
}

KnightOnFire::KnightOnFire(const Critter& cr, SP<BasicController> pBc_, unsigned nTimer_, Chain c_)
    :Critter(cr), pBc(this, pBc_),nTimer(nTimer_), nTimer_i(nTimer_),c(c_), t(2)
{
    Critter::seq = pBc->pGl->pr("knight_fire");
    RandomizeVelocity();
    //pBc->lsPpl.push_back(this);
}



void KnightOnFire::Update()
{
    Critter::Update();
    
    for(std::list< ASSP<ConsumableEntity> >::iterator itr = pBc->lsPpl.begin(); itr != pBc->lsPpl.end(); ++itr)
    {
        if(!(*itr)->bExist)
            continue;

        if (this->HitDetection(*itr))
        {
            if(c.IsLast() || (*itr)->GetType() != 'K')
                (*itr)->OnHit();
            else
            {
                (*itr)->bExist = false;
                SP<KnightOnFire> pKn = new KnightOnFire(
                    Critter(GetRadius(), GetPosition(), fPoint(),
                    rBound, GetPriority(), ImageSequence(), true), pBc, nTimer_i, c.Evolve());
                pBc->AddBoth(pKn);
            }
        }
    }

    if(nTimer != 0 && --nTimer == 0)
    {
        bExist = false;
        
        SP<AnimationOnce> pAn = new AnimationOnce(dPriority,
            pBc->pGl->pr("knight_die"),
            2U, GetPosition(), true);
        pBc->AddBoth(pAn);
    }

    if(t.Tick() && float(rand())/RAND_MAX < .25)
        RandomizeVelocity();
}

struct AdNumberDrawer: public VisualEntity
{
    SSP<AdvancedController> pAd;

    AdNumberDrawer():pAd(this, 0){}
    
    /*virtual*/ void Draw(SP<ScalingDrawer> pDr)
    {
        unsigned n = pDr->nFactor;

        pDr->pGr->DrawRectangle(Rectangle((pAd->rBound.sz.x - 120) * n, 0, pAd->rBound.sz.x * n, 12 * n), Color(50, 50, 50), false);

        pAd->pGl->pNum->DrawNumber((pAd->t.nPeriod - pAd->t.nTimer)/10, Point(pAd->rBound.sz.x - 20, 4), 4);
        pAd->pGl->pNum->DrawNumber(pAd->pGl->nScore, Point(pAd->rBound.sz.x - 80, 4), 7);
        pDr->Draw(pAd->pGl->pr["time"], Point(pAd->rBound.sz.x - 49, 0), false);
        pDr->Draw(pAd->pGl->pr["score"], Point(pAd->rBound.sz.x - 115, 0), false);
    }
    /*virtual*/ Point GetPosition(){return Point();}
    /*virtual*/ float GetPriority(){return 10;}
};


AdvancedController::AdvancedController(SP<GlobalController> pGl_, Rectangle rBound, Color c, int lvl)
:BasicController(pGl_, rBound, c), bCh(false)
{

    SP<AdNumberDrawer> pNm = new AdNumberDrawer();
    pNm->pAd = this;
    AddV(pNm);

    SP<KnightGenerator> pGen = new KnightGenerator(.03F, rBound, this);
    SP<PrincessGenerator> pPGen = new PrincessGenerator(.007F, rBound, this);
    SP<TraderGenerator> pTGen = new TraderGenerator(.007F, rBound, this);
    SP<MageGenerator> pMGen = new MageGenerator(.007F, rBound, this);

    PushBackASSP(this, vRd, new FancyRoad(Road(false, rBound.sz.y * 1 / 4, rBound), this));
    PushBackASSP(this, vRd, new FancyRoad(Road(false, rBound.sz.y * 3 / 4, rBound), this));
    PushBackASSP(this, vRd, new FancyRoad(Road(true, rBound.sz.x * 1 / 4, rBound), this));
    PushBackASSP(this, vRd, new FancyRoad(Road(true, rBound.sz.x * 3 / 4, rBound), this));
    

    if(lvl == 1)
    {
        PushBackASSP(this, vCs, new Castle(Point(rBound.sz.x/2, rBound.sz.y/2), rBound, this));

        t = Timer(1000);
    }
    else if (lvl == 2)
    {
        PushBackASSP(this, vCs, new Castle(Point(rBound.sz.x/3, rBound.sz.y/2), rBound, this));
        PushBackASSP(this, vCs, new Castle(Point(rBound.sz.x*2/3, rBound.sz.y/2), rBound, this));
        t = Timer(2000);
    }
    else
    {
        PushBackASSP(this, vCs, new Castle(Point(rBound.sz.x/3, rBound.sz.y/3), rBound, this));
        PushBackASSP(this, vCs, new Castle(Point(rBound.sz.x*2/3, rBound.sz.y/3), rBound, this));
        PushBackASSP(this, vCs, new Castle(Point(rBound.sz.x/3, rBound.sz.y*2/3), rBound, this));
        PushBackASSP(this, vCs, new Castle(Point(rBound.sz.x*2/3, rBound.sz.y*2/3), rBound, this));
        t = Timer(4000);
    }

    PushBackASSP(this, vDr, new Dragon(vCs[0], this, pGl->pr("dragon_stable"), pGl->pr("dragon_fly"),
        ButtonSet('q', 'w', 'e', 'd', 'c', 'x', 'z', 'a', ' ')));
    //PushBackASSP(this, vDr, new Dragon(vCs.size() >= 2 ? vCs[1] : SP<Castle>(0), this, pGl->pr("bdragon_stable"), pGl->pr("bdragon_fly"),
    //    ButtonSet(SDLK_KP7, SDLK_KP8, SDLK_KP9, SDLK_KP6, SDLK_KP3, SDLK_KP2, SDLK_KP1, SDLK_KP4, SDLK_RETURN)));

    
    AddE(pGen);
    AddE(pPGen);
    AddE(pTGen);
    AddE(pMGen);
    unsigned i;
    for(i = 0; i < vCs.size(); ++i)
        AddBoth(vCs[i]);
    for(i = 0; i < vRd.size(); ++i)
        AddV(vRd[i]);
    for(i = 0; i < vDr.size(); ++i)
        AddBoth(vDr[i]);
}

/*virtual*/ void AdvancedController::OnKey(int c, bool bUp)
{
    if(bUp)
        return;
    if(c == '\\')
    {
        if(pGl->nActive == pGl->vCnt.size() - 1)
            pGl->nActive = 0;
        else
            ++pGl->nActive;
        return;
    }

    if(c == '=')
        bCh = !bCh;


    if(c >= SDLK_F1 && c <= SDLK_F15)
        for(unsigned i = 0; i < vDr.size(); ++i)
            vDr[i]->AddBonus(vDr[i]->GetBonus(c - SDLK_F1));

    for(unsigned i = 0; i < vDr.size(); ++i)
        if(vDr[i]->bt.IsSpace(c))
            vDr[i]->Toggle();

    for(unsigned i = 0; i < vDr.size(); ++i)
    {
        Point p = vDr[i]->bt.GetPoint(c);

        if(p == Point())
            continue;

        fPoint fp(p);
        fp.Normalize();

        if(vDr[i]->bFly)
        {
            fp.x *= 3;
            fp.y *= 3;

            vDr[i]->fVel = fp;
        }
        else
        {
            fp.x += (float(rand())/RAND_MAX - .5F) / 2;
            fp.y += (float(rand())/RAND_MAX - .5F) / 2;

            vDr[i]->Fire(fp);
        }
    }
}

Castle::Castle(Point p, Rectangle rBound_, SP<AdvancedController> pAv_)
    :Critter(15, p, Point(), rBound_, 3, pAv_->pGl->pr("castle")),
    nPrincesses(0), pAv(this, pAv_), pDrag(this, 0)
{}

void Castle::OnKnight()
{
    if(!nPrincesses || pAv->bCh)
        return;

    if(pDrag != 0)
    {
        --nPrincesses;

        fPoint v = RandomAngle();
        v.Normalize(2);

        SP<Princess> pCr = new Princess(
            Critter(7, GetPosition(), v, rBound, 0, 
            v.x < 0 ? pAv->pGl->pr("princess_f") : pAv->pGl->pr("princess")
            , true),
            pAv);
        pAv->AddBoth(pCr);
        PushBackASSP(pAv.GetRawPointer(), pAv->lsPpl, pCr);

        return;
    }

    float r = float(rand())/RAND_MAX*2*3.1415F;

    for(unsigned i = 0; i < nPrincesses; ++i)
    {
        fPoint v (sin(r + i * 2 * 3.1415F / nPrincesses), cos(r + i * 2 * 3.1415F / nPrincesses));
        v.Normalize();
        v.x *= 2;
        v.y *= 2;

        SP<Princess> pCr = new Princess(
            Critter(7, GetPosition(), v, rBound, 0, 
            v.x < 0 ? pAv->pGl->pr("princess_f") : pAv->pGl->pr("princess")
            , true),
            pAv);
        pAv->AddBoth(pCr);
        PushBackASSP(pAv.GetRawPointer(), pAv->lsPpl, pCr);
    }

    nPrincesses = 0;
}

/*unsigned*/ void Castle::Draw(SP<ScalingDrawer> pDr)
{
    Critter::Draw(pDr);

    pAv->pGl->pNum->DrawNumber(nPrincesses, GetPosition() + Point(0, 18));
}

/*virtual*/ void AdvancedController::Update()
{
    BasicController::Update();

    pGl->pNum->DrawNumber(t.nPeriod - t.nTimer, Point(rBound.sz.x - 20, 0));

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

    tr.Update();
    if(!vDr[0]->bFly)
    {
        if(tr.IsTrigger())
        {
            fPoint p = tr.GetMovement();
            
            vDr[0]->Fire(p);
            
            //p.Normalize(5);

            //SP<Fireball> pFr = new Fireball(pDr->GetPosition(), p, rBound, this);
            //AddBoth(pFr);
        }
    }
    else
    {
        //fPoint p = tr.GetAvMovement();
        //if(p.Length() <= 25)
        //    return;
        
        //p.Normalize(3);

        

        //if(p != fPoint())
        //    vDr[0]->fVel = p;

        vDr[0]->fVel = vDr[0]->leash.GetNewVelocity(tr.GetMovement());
    }

    if(t.Tick())
    {
        pGl->nActive = 5;
    }
}
