#include "tower_defense.h"

AdvancedController::AdvancedController(GlobalController* pGl_, Rectangle rBound, Color c)
:BasicController(pGl_, rBound, c)
{
    CritterGenerator* pGen = new CritterGenerator(.05, rBound, this);
    pCs = new Castle(Point(rBound.sz.x/2, rBound.sz.y/2), rBound, pGl->pDr);
    pDr = new Dragon(this);
    
    Add(pGen);
    AddBoth(pCs);
    AddBoth(pDr);
}

/*virtual*/ void AdvancedController::OnKey(char c, bool bUp)
{
    if(bUp)
        return;
    if(c == '`')
    {
        if(pGl->nActive == pGl->vCnt.size() - 1)
            pGl->nActive = 0;
        else
            ++pGl->nActive;
        return;
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
        fp.x += double(rand())/RAND_MAX / 3;
        fp.y += double(rand())/RAND_MAX / 3;

        fp.x *= 3;
        fp.y *= 3;

        Fireball* pFr = new Fireball(Point(rBound.sz.x/2, rBound.sz.y/2), fp, rBound, this);
        AddBoth(pFr);
    }
}
