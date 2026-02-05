#include "tower_defense.h"

AdvancedController::AdvancedController(GlobalController* pGl_, Rectangle rBound, Color c, int lvl)
:BasicController(pGl_, rBound, c), imgPr(pGl_->pDr, "princess.txt", "princess\\"), bCh(false)
{
    CritterGenerator* pGen = new CritterGenerator(.03, rBound, this);
    PrincessGenerator* pPGen = new PrincessGenerator(.007, rBound, this);
    
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
    
    pDr = new Dragon(vCs[0]->GetPosition(), this);
    
    Add(pGen);
    Add(pPGen);
    for(unsigned i = 0; i < vCs.size(); ++i)
        AddBoth(vCs[i]);
    AddBoth(pDr);
}

/*virtual*/ void AdvancedController::OnKey(char c, bool bUp)
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
    {
        bCh = !bCh;
    }

    if(c == ' ')
    {
        pDr->Toggle();
        return;
    }

    Point p(0,0);
    
    if(c == 'q') p = Point(-1, -1);
    if(c == 'w') p = Point( 0, -1);
    if(c == 'e') p = Point( 1, -1);
    if(c == 'd') p = Point( 1,  0);
    if(c == 'c') p = Point( 1,  1);
    if(c == 'x') p = Point( 0,  1);
    if(c == 'z') p = Point(-1,  1);
    if(c == 'a') p = Point(-1,  0);

    if(p == Point())
        return;

    fPoint fp(p);
    fp.Normalize();

    if(pDr->bFly)
    {
        fp.x *= 3;
        fp.y *= 3;

        pDr->fVel = fp;
    }
    else
    {
        fp.x += (double(rand())/RAND_MAX - .5) / 2;
        fp.y += (double(rand())/RAND_MAX - .5) / 2;

        fp.x *= 3;
        fp.y *= 3;

        Fireball* pFr = new Fireball(pDr->GetPosition(), fp, rBound, this);
        AddBoth(pFr);
    }
}

Castle::Castle(Point p, Rectangle rBound_, AdvancedController* pAv_)
    :Critter(10, p, Point(), rBound_, 0, ImageSequence(pAv_->pGl->pDr, "castle.txt", "castle\\")),
    nPrincesses(0), pAv(pAv_)
{}

void Castle::OnKnight()
{
    if(!nPrincesses || (!pAv->pDr->bFly && (pAv->pDr->GetPosition() == GetPosition())) || pAv->bCh)
        return;

    for(unsigned i = 0; i < nPrincesses; ++i)
    {
        fPoint v (sin(i * 2 * 3.14 / nPrincesses), cos(i * 2 * 3.14 / nPrincesses));
        v.Normalize();
        v.x *= 2;
        v.y *= 2;

        Princess* pCr = new Princess(
            Critter(7, GetPosition(), v, rBound, 0, 
            pAv->imgPr, true),
            pAv);
        pCr->Add();
    }

    nPrincesses = 0;
}

/*unsigned*/ void Castle::Draw(Drawer* pDr)
{
    Critter::Draw(pDr);

    pAv->pGl->pNum->DrawNumber(nPrincesses, GetPosition() + Point(0, 15));
}
