#include "tower_defense.h"

GlobalController::GlobalController(SP<ScalingDrawer> pDr_, SP<NumberDrawer> pNum_)
    :nActive(0), pDr(pDr_), pGraph(pDr_->pGr), pNum(pNum_), pr(pDr_->pGr)
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

}

AdvancedController::AdvancedController(SP<GlobalController> pGl_, Rectangle rBound, Color c, int lvl)
:BasicController(pGl_, rBound, c), bCh(false)
{

    std::cout << "AdvancedController\n";

    SP<CritterGenerator> pGen = new CritterGenerator(.50F, rBound, this);
    SP<PrincessGenerator> pPGen = new PrincessGenerator(.007F, rBound, this);
    SP<TraderGenerator> pTGen = new TraderGenerator(.007F, rBound, this);

    vRd.push_back(new Road(false, rBound.sz.y * 1 / 4, rBound));
    vRd.push_back(new Road(false, rBound.sz.y * 3 / 4, rBound));
    vRd.push_back(new Road(true, rBound.sz.x * 1 / 4, rBound));
    vRd.push_back(new Road(true, rBound.sz.x * 3 / 4, rBound));
    

    if(lvl == 1)
        vCs.push_back(new Castle(Point(rBound.sz.x/2, rBound.sz.y/2), rBound, this));
    else if (lvl == 2)
    {
        vCs.push_back(new Castle(Point(rBound.sz.x/3, rBound.sz.y/2), rBound, this));
        vCs.push_back(new Castle(Point(rBound.sz.x*2/3, rBound.sz.y/2), rBound, this));
    }
    else
    {
        vCs.push_back(new Castle(Point(rBound.sz.x/3, rBound.sz.y/3), rBound, this));
        vCs.push_back(new Castle(Point(rBound.sz.x*2/3, rBound.sz.y/3), rBound, this));
        vCs.push_back(new Castle(Point(rBound.sz.x/3, rBound.sz.y*2/3), rBound, this));
        vCs.push_back(new Castle(Point(rBound.sz.x*2/3, rBound.sz.y*2/3), rBound, this));
    }

    vDr.push_back(new Dragon(vCs[0], this, pGl->pr("dragon_stable"), pGl->pr("dragon_fly"),
        ButtonSet('q', 'w', 'e', 'd', 'c', 'x', 'z', 'a', ' ')));
    vDr.push_back(new Dragon(vCs.size() >= 2 ? vCs[1] : 0, this, pGl->pr("bdragon_stable"), pGl->pr("bdragon_fly"),
        ButtonSet(SDLK_KP7, SDLK_KP8, SDLK_KP9, SDLK_KP6, SDLK_KP3, SDLK_KP2, SDLK_KP1, SDLK_KP4, SDLK_RETURN)));

    
    AddE(pGen);
    AddE(pPGen);
    AddE(pTGen);
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
    :Critter(10, p, Point(), rBound_, 2, pAv_->pGl->pr("castle")),
    nPrincesses(0), pAv(pAv_), pDrag(0)
{}

void Castle::OnKnight()
{
    if(!nPrincesses || (pDrag != 0) || pAv->bCh)
        return;

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
        pAv->lsPpl.push_back(pCr);
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
}
