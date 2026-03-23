#include "level_controller.h"
#include "dragon_game_controller.h"
#include "../critter_generators.h"
#include "../critters.h"
#include "../dragon.h"
#include "../dragon_constants.h"
#include "../dragon_game_runner.h"
#include "../entities.h"
#include "../fireball.h"
#include "../level.h"
#include "../tutorial.h"
#include "../../game_utils/draw_utils.h"
#include "../../game_utils/game_runner_interface.h"
#include "../../game_utils/image_sequence.h"
#include "../../game_utils/sound_utils.h"
#include "../../utils/random_utils.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"

/* Local helpers for level UI: score/time/level and bonus icons. */
class AdNumberDrawer : public Entity {
public:
  bool ShouldDraw() override { return true; }
  LevelController *pAd;

  AdNumberDrawer() : pAd(0) {}
  explicit AdNumberDrawer(LevelController *ad) : pAd(ad) {}

  void Draw(ScalingDrawer * /*pDr*/) override {
    if (pAd->pGl->GetGameConfig().IsFullVersion()) {
      pAd->pGl->GetNumberDrawer()->DrawNumber(
          pAd->pGl->GetScore(), Point(pAd->rBound.sz.x - 27 * 4, 4), 7);

      if (pAd->bBlink_) {
        Color c(255, 255, 0);

        if (pAd->bTimerFlash_)
          c = Color(255, 0, 0);

        pAd->pGl->GetNumberDrawer()->DrawColorNumber(
            pAd->t_.UntilTick() / nFramesInSecond,
            Point(pAd->rBound.sz.x - 14 * 4, 4), c, 4);
        pAd->pGl->GetNumberDrawer()->DrawColorWord(
            "time:", Point(pAd->rBound.sz.x - 19 * 4, 4), c);
      }

      pAd->pGl->GetNumberDrawer()->DrawNumber(
          pAd->nLvl_, Point(pAd->rBound.sz.x - 3 * 4, 4), 2);

      pAd->pGl->GetNumberDrawer()->DrawWord("score:",
                                            Point(pAd->rBound.sz.x - 33 * 4, 4));
      pAd->pGl->GetNumberDrawer()->DrawWord("level:",
                                            Point(pAd->rBound.sz.x - 9 * 4, 4));
      if (pAd->bCh_) {
        pAd->pGl->GetNumberDrawer()->DrawColorWord(
            "invincible", Point(pAd->rBound.sz.x - 44 * 4, 4), Color(0, 255, 0));
      }
    } else {
      pAd->pGl->GetNumberDrawer()->DrawNumber(
          pAd->pGl->GetScore(), Point(pAd->rBound.sz.x - 17 * 4, 4), 7);
      pAd->pGl->GetNumberDrawer()->DrawNumber(
          pAd->nLvl_, Point(pAd->rBound.sz.x - 3 * 4, 4), 2);

      pAd->pGl->GetNumberDrawer()->DrawWord("score:",
                                            Point(pAd->rBound.sz.x - 23 * 4, 4));
      pAd->pGl->GetNumberDrawer()->DrawWord("level:",
                                            Point(pAd->rBound.sz.x - 9 * 4, 4));
    }
  }
  Point GetPosition() override { return Point(); }
  float GetPriority() override { return 10; }
};

class BonusDrawer : public Entity {
public:
  bool ShouldDraw() override { return true; }
  LevelController *pAd;

  Timer t;
  int nAnimationCounter;

  BonusDrawer()
      : pAd(0), t(int(nFramesInSecond * .1F)), nAnimationCounter(0) {}
  explicit BonusDrawer(LevelController *ad)
      : pAd(ad), t(int(nFramesInSecond * .1F)), nAnimationCounter(0) {}

  void Draw(ScalingDrawer *pDr) override {
    if (t.Tick())
      ++nAnimationCounter;

    Point p(1, 3);

    for (int nDr = 0; nDr < (int)pAd->vDr_.size(); ++nDr) {
      auto &lst = pAd->vDr_[nDr]->lsBonuses;

      for (auto itr = lst.begin(), etr = lst.end(); itr != etr; ++itr) {
        TimedFireballBonus *pBns = itr->get();

        if (pBns->t.IsActive() &&
            pBns->t.UntilTick() < 4 * nFramesInSecond)
          if (nAnimationCounter % 2 == 1) {
            p.x += 10;
            continue;
          }

        ImageSequence img = pAd->pGl->GetImgSeq(GetBonusImage(pBns->nNum));

        pDr->Draw(img.GetImageAt(nAnimationCounter % img.GetImageCount()), p, false);

        p.x += 10;
      }

      p.y += 10;

      p.x = 3;

      for (int i = 0; i < pAd->vDr_[nDr]->nFireballCount; ++i) {
        pDr->Draw(pAd->pGl->GetImgSeq("fireball_icon").GetImage(), p, false);

        p.x += 7;
      }

      p.y += 7;
      p.x = 0;
    }
  }
  Point GetPosition() override { return Point(); }
  float GetPriority() override { return 10; }
};

static const float fSpreadFactor = 2.0f;

LevelController::~LevelController() = default;

void LevelController::TutorialNotify(TutorialEvent event) {
  pTutorial_->Notify(event);
}

LevelController::LevelController(DragonGameController *pGl_, Rectangle rBound,
                                 Color c, const LevelLayout &lvl,
                                 std::unique_ptr<SoundControls> pSc)
    : EntityListController(pGl_, rBound, c),
      bFirstUpdate_(true), bGhostTime_(false), bTimerFlash_(false), bBlink_(true),
      bLeft_(false), bCh_(false), bLeftDown_(false), bRightDown_(false),
      nLastDir_(0), bWasDirectionalInput_(0), nLvl_(lvl.nLvl), nSlimeNum_(0),
      pGr_(0), bTakeOffToggle_(false),
      pTutorialText_(), pTutorial_(std::make_unique<NoopTutorial>()),
      pSc_(std::move(pSc)),
      mc_(pGl->GetImgSeq("claw"), Point()) {}

Dragon *LevelController::FindDragon(Dragon *p) {
  for (size_t i = 0; i < vDr_.size(); ++i)
    if (vDr_[i].get() == p)
      return vDr_[i].get();
  return nullptr;
}

void LevelController::Init(const LevelLayout &lvl) {
  SuppressRefresh();

  tLoseTimer_ = Timer(0);

  AddOwnedEntity(std::make_unique<AdNumberDrawer>(this));
  AddOwnedEntity(std::make_unique<BonusDrawer>(this));

  pKnightGen_ = std::make_unique<KnightGenerator>(lvl.vFreq.at(0), rBound, this,
                                                  lvl.blKnightGen);
  pPGen_ = std::make_unique<PrincessGenerator>(lvl.vFreq.at(1), rBound, this);
  pTGen_ = std::make_unique<TraderGenerator>(lvl.vFreq.at(2), rBound, this);
  pMGen_ = std::make_unique<MageGenerator>(lvl.vFreq.at(3), lvl.vFreq.at(4),
                                           rBound, this);

  pGr_ = pKnightGen_.get();
  pMgGen_ = pMGen_.get();

  int i;
  for (i = 0; i < (int)lvl.vRoadGen.size(); ++i)
    vRd_.push_back(std::make_unique<FancyRoad>(lvl.vRoadGen[i], this));

  for (i = 0; i < (int)lvl.vCastleLoc.size(); ++i)
    vCs_.push_back(std::make_unique<Castle>(lvl.vCastleLoc[i], rBound, this));

  t_ = Timer(lvl.nTimer);

  vDr_.push_back(std::make_unique<Dragon>(
      vCs_[0].get(), this, &pt_, pGl->GetImgSeq("dragon_stable"),
      pGl->GetImgSeq("dragon_fly"),
      ButtonSet('q', 'w', 'e', 'd', 'c', 'x', 'z', 'a', ' ')));
  if (vDr_.back()->pCs != nullptr)
    vDr_.back()->pCs->pDrag = vDr_.back().get();

  Point pos(pGl->GetBounds().sz.x / 2, pGl->GetBounds().sz.y);
  pTutorialText_ =
      std::make_unique<TutorialTextEntity>(1, pos, pGl->GetNumberDrawer(), pGl);

  if (pGl->GetGameConfig().IsPcVersion()) {
    if (nLvl_ == 1) {
      const GameConfig &cfg = pGl->GetGameConfig();
      pTutorial_ = std::make_unique<TutorialLevelOne>(
          pTutorialText_.get(), cfg.IsJoystickTutorial(),
          cfg.IsKeyboardControls());
    }

    if (nLvl_ == 2) {
      pTutorial_ = std::make_unique<TutorialLevelTwo>(pTutorialText_.get());
    }
  }
}

void LevelController::OnKey(GuiKeyType c, bool bUp) {

  if (pGl->GetGameConfig().IsKeyboardControls()) {
    if (!bUp) {
      if (c == GUI_LEFT)
        bLeftDown_ = true;
      else if (c == GUI_RIGHT)
        bRightDown_ = true;
    } else {
      if (c == GUI_LEFT)
        bLeftDown_ = false;
      else if (c == GUI_RIGHT)
        bRightDown_ = false;
    }
  }

  if (bUp)
    return;

  if (pGl->AreCheatsOnSetting()) {
    if (c == '\\') {
      pGl->Next();
      return;
    }

    if (c == 'i')
      bCh_ = !bCh_;

    if (c == 'g')
      pGr_->Generate(true);

    if (c == 'l')
      MegaGeneration();

    if (c == 'm')
      pMgGen_->MageGenerate();

    if (c == '6')
      std::cout << "Slimes: " << nSlimeNum_ << "\n";

    if (c == 't')
      t_.NextTick();

    if (c >= GUI_F1 && c <= GUI_F10)
      for (int i = 0; i < (int)vDr_.size(); ++i)
        vDr_[i]->AddBonus(vDr_[i]->GetBonus(c - GUI_F1 + 1, nBonusCheatTime));
  }

  if (pGl->GetGameConfig().IsPcVersion() && c == GUI_ESCAPE)
    pGl->EnterMenu();

  for (int i = 0; i < (int)vDr_.size(); ++i)
    if (vDr_[i]->bt.IsSpace(c)) {
      if (!vDr_[i]->bFly)
        vDr_[i]->Toggle();
      else {
        fPoint fFb = vDr_[0]->GetVel();
        fFb.Normalize(100);
        vDr_[0]->Fire(fFb);
      }
    }

  if (pGl->GetGameConfig().IsKeyboardControls()) {
    if (nLastDir_ == 0) {
      bool flag = true;
      if (c == GUI_LEFT)
        nLastDir_ = 1;
      else if (c == GUI_RIGHT)
        nLastDir_ = 2;
      else if (c == GUI_DOWN)
        nLastDir_ = 3;
      else if (c == GUI_UP)
        nLastDir_ = 4;
      else
        flag = false;
      if (flag)
        bWasDirectionalInput_ = true;
    } else {
      int dir = 0;
      if (c == GUI_LEFT)
        dir = 1;
      else if (c == GUI_RIGHT)
        dir = 2;
      else if (c == GUI_DOWN)
        dir = 3;
      else if (c == GUI_UP)
        dir = 4;
      fPoint fp = ComposeDirection(nLastDir_, dir);
      fp.x += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;
      fp.y += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;

      if (!vDr_[0]->bFly) {
        vDr_[0]->Fire(fp);
        pt_.UpdateLastDownPosition(Point(fp.x * 10000, fp.y * 10000));
      }
      bWasDirectionalInput_ = false;
    }
  }
}

void LevelController::OnMouse(Point pPos) {
  Size sz1 = GetProgramInfo().szScreenRez;
  Size sz2 = pGl->GetActualResolution();

  float fX = float(sz1.x) / sz2.x;
  float fY = float(sz1.y) / sz2.y;

  pPos.x *= Crd(fX);
  pPos.y *= Crd(fY);

  pt_.UpdateMouse(pPos);
  mc_.SetCursorPos(pPos);
}

void LevelController::OnMouseDown(Point pPos) {
  Size sz1 = GetProgramInfo().szScreenRez;
  Size sz2 = pGl->GetActualResolution();

  float fX = float(sz1.x) / sz2.x;
  float fY = float(sz1.y) / sz2.y;

  pPos.x *= Crd(fX);
  pPos.y *= Crd(fY);

  pt_.UpdateMouse(pPos);
  pt_.UpdateLastDownPosition(pPos);
  pt_.On();

  bool bHit = false;

  if (!vDr_[0]->bFly)
    bHit = (pt_.GetDirection(vDr_[0]->pCs->GetPosition()).Length() <
            fTowerClickRadius);
  else
    bHit =
        (pt_.GetDirection(vDr_[0]->GetPosition()).Length() < fDragonClickRadius);

  if (bHit) {
    if (!vDr_[0]->bFly)
      bTakeOffToggle_ = true;

    vDr_[0]->Toggle();
  } else {
    if (!vDr_[0]->bFly) {
      fPoint fFb = pt_.GetDirection(vDr_[0]->GetPosition() + Point(-10, -25));


      fFb.Normalize(100);
      vDr_[0]->Fire(fFb);
    }
  }
}

void LevelController::OnMouseUp() {
  float fTime = float(pt_.Off());
  fTime = fTime / nFramesInSecond;

  if (vDr_[0]->bFly && fTime <= .2 && !bTakeOffToggle_ &&
      pt_.GetDirection(vDr_[0]->GetPosition()).Length() > vDr_[0]->GetRadius()) {
    fPoint fFb = vDr_[0]->GetVel();

    fFb.Normalize(100);
    vDr_[0]->Fire(fFb);
  }

  if (bTakeOffToggle_)
    bTakeOffToggle_ = false;
}

void LevelController::Fire() {
  if (vDr_[0]->bFly) {
    fPoint fFb = vDr_[0]->GetVel();

    fFb.Normalize(100);
    vDr_[0]->Fire(fFb);
  }
}

float LevelController::GetCompletionRate() {
  float fCap = 0;
  for (int i = 0; i < (int)vCs_.size(); ++i)
    fCap += vCs_[i]->nPrincesses;

  fCap /= (4 * vCs_.size());

  return fCap;
}

void LevelController::AddBonusAnimation(
    std::unique_ptr<FireballBonusAnimation> p) {
  lsBonus_.push_back(std::move(p));
}

std::vector<FireballBonusAnimation *> LevelController::GetBonusAnimations() {
  std::vector<FireballBonusAnimation *> out;
  for (auto &u : lsBonus_)
    out.push_back(u.get());
  return out;
}

void LevelController::AddSlime(std::unique_ptr<Slime> p) {
  lsSlimes_.push_back(std::move(p));
}

void LevelController::AddSliminess(std::unique_ptr<Sliminess> p) {
  lsSliminess_.push_back(std::move(p));
}

void LevelController::AddMegaSlime(std::unique_ptr<MegaSlime> p) {
  lsMegaSlimes_.push_back(std::move(p));
}

void LevelController::AddMegaSliminess(std::unique_ptr<MegaSliminess> p) {
  lsMegaSliminess_.push_back(std::move(p));
}

void LevelController::AddSpawnedGenerator(std::unique_ptr<Entity> p) {
  lsSpawnedGenerators_.push_back(std::move(p));
}

std::vector<ConsumableEntity *> LevelController::GetPeoplePointers() {
  std::vector<ConsumableEntity *> out;
  for (auto &p : lsPpl_)
    out.push_back(p.get());
  return out;
}

std::vector<ConsumableEntity *> LevelController::GetConsumablePointers() {
  std::vector<ConsumableEntity *> out = GetPeoplePointers();
  for (auto &u : lsSlimes_)
    out.push_back(u.get());
  for (auto &u : lsMegaSlimes_)
    out.push_back(u.get());
  return out;
}

std::vector<Entity *> LevelController::GetNonOwnedEntities() {
  std::vector<Entity *> out;
  for (auto &u : lsPpl_)
    out.push_back(u.get());
  for (size_t i = 0; i < vCs_.size(); ++i)
    out.push_back(vCs_[i].get());
  for (size_t i = 0; i < vRd_.size(); ++i)
    out.push_back(vRd_[i].get());
  if (pTutorialText_)
    out.push_back(pTutorialText_.get());
  if (pSc_)
    out.push_back(pSc_.get());
  for (auto &u : lsBonus_)
    out.push_back(u.get());
  for (auto &u : lsSlimes_)
    out.push_back(u.get());
  for (auto &u : lsMegaSlimes_)
    out.push_back(u.get());
  for (auto &u : lsSliminess_) {
    out.push_back(u.get());
    if (u->pSlm_)
      out.push_back(u->pSlm_.get());
  }
  for (auto &u : lsMegaSliminess_) {
    out.push_back(u.get());
    if (u->pSlm_)
      out.push_back(u->pSlm_.get());
  }
  for (size_t i = 0; i < vDr_.size(); ++i)
    out.push_back(vDr_[i].get());
  for (size_t i = 0; i < vDr_.size(); ++i)
    for (auto &u : vDr_[i]->lsBonuses)
      out.push_back(u.get());
  if (pKnightGen_)
    out.push_back(pKnightGen_.get());
  if (pPGen_)
    out.push_back(pPGen_.get());
  if (pTGen_)
    out.push_back(pTGen_.get());
  if (pMGen_)
    out.push_back(pMGen_.get());
  for (auto &u : lsSpawnedGenerators_)
    out.push_back(u.get());
  return out;
}

void LevelController::MegaGeneration() {
  Point p;
  p.x = GetRandNum(rBound.sz.x);
  p.y = GetRandNum(rBound.sz.y);
  MegaGeneration(p);
}

void LevelController::MegaGeneration(Point p) {
  AddMegaSliminess(std::make_unique<MegaSliminess>(p, this));
}

void LevelController::StartLoseTimer() {
  if (!tLoseTimer_.IsActive())
    tLoseTimer_ = Timer(nFramesInSecond * 3);
}

int LevelController::GetLoseTimerFrame() const {
  return tLoseTimer_.GetTimer() / 2;
}

void LevelController::StopMusic() {
  if (pSc_)
    pSc_->SetTheme(-1);
}

Castle *LevelController::GetFirstCastle() {
  return vCs_[0].get();
}

std::vector<Castle *> LevelController::GetCastlePointers() {
  std::vector<Castle *> out;
  for (auto &u : vCs_)
    out.push_back(u.get());
  return out;
}

void LevelController::GetRandomRoadLocation(Point &p, Point &v) {
  vRd_[rand() % vRd_.size()]->RoadMap(p, v);
}

void LevelController::DoSlimeMassKill() {
  std::vector<Point> vDeadSlimes;

  for (auto &u : lsSlimes_) {
    if (!u->Exists())
      continue;
    vDeadSlimes.push_back(u->GetPosition());
    u->OnHit('M');
  }

  for (auto &u : lsMegaSlimes_) {
    if (!u->Exists())
      continue;
    vDeadSlimes.push_back(u->GetPosition());
    u->OnHit('M');
  }

  for (auto &u : lsSliminess_) {
    if (!u->Exists())
      continue;
    vDeadSlimes.push_back(u->GetPosition());
    u->Destroy();
  }

  for (auto &u : lsMegaSliminess_) {
    if (!u->Exists())
      continue;
    vDeadSlimes.push_back(u->GetPosition());
    u->Destroy();
  }

  if (vDeadSlimes.empty())
    throw SimpleException("No slimes found!");

  fPoint fAvg(0, 0);
  for (int i = 0; i < (int)vDeadSlimes.size(); ++i)
    fAvg += vDeadSlimes[i];
  fAvg /= float(vDeadSlimes.size());

  MegaGeneration(fAvg.ToPnt());

  for (int i = 0; i < (int)vDeadSlimes.size(); ++i) {
    AddOwnedEntity(std::make_unique<FloatingSlime>(
        pGl->GetImgSeq("slime_cloud"), vDeadSlimes[i],
        fAvg.ToPnt(), nFramesInSecond * 1));
  }
}

void LevelController::Update() {
  CleanUp(lsPpl_);
  CleanUp(lsBonus_);
  CleanUp(lsSlimes_);
  CleanUp(lsMegaSlimes_);
  CleanUp(lsSliminess_);
  CleanUp(lsMegaSliminess_);
  CleanUp(lsSpawnedGenerators_);

  pt_.Update();

  if (bFirstUpdate_) {
    bFirstUpdate_ = false;

    if (nLvl_ != 1 && nLvl_ != 4 && nLvl_ != 7 && nLvl_ != 10) {
      vDr_[0]->RecoverBonuses();
    }
    pGl->ClearBonusesToCarryOver();
  }

  if (!tLoseTimer_.IsActive()) {
    if (!bGhostTime_) {
      if (nLvl_ <= 3)
        pSc_->SetTheme(BG_BACKGROUND);
      else if (nLvl_ <= 6)
        pSc_->SetTheme(BG_BACKGROUND2);
      else
        pSc_->SetTheme(BG_BACKGROUND3);
    } else {
      if (nLvl_ <= 3)
        pSc_->SetTheme(BG_SLOW_BACKGROUND);
      else if (nLvl_ <= 6)
        pSc_->SetTheme(BG_SLOW_BACKGROUND2);
      else
        pSc_->SetTheme(BG_SLOW_BACKGROUND3);
    }
  }

  EntityListController::Update();

  if (pGl->GetGameConfig().IsPcVersion() && !pGl->GetGameConfig().IsKeyboardControls()) {
    mc_.bPressed = pt_.bPressed;
    mc_.DrawCursor(pGl->GetGraphics());
  }
  pGl->RefreshAll();

  if (tLoseTimer_.IsActive() && tLoseTimer_.Tick()) {
    pGl->ShowGameOverScreen();
    return;
  }

  tr_.Update();
  if (!vDr_[0]->bFly) {
    if (tr_.IsTrigger()) {
      fPoint p = tr_.GetMovement();

      if (p.Length() > 50) {
        if (p.Length() > 250)
          p.Normalize(250);

        vDr_[0]->Fire(p);
      }
    }
  } else {
    if (pt_.bPressed) {
      fPoint v = vDr_[0]->GetVel();
      fPoint d = pt_.GetDirection(vDr_[0]->GetPosition());

      if (d.Length() == 0)
        d = v;

      d.Normalize(v.Length());

      fPoint newVel = v * fFlightCoefficient + d;
      newVel.Normalize(vDr_[0]->leash.speed);
      vDr_[0]->SetVel(newVel);
    } else if (bLeftDown_ || bRightDown_) {
      fPoint v = vDr_[0]->GetVel();
      fPoint d(v.y, v.x);
      if (bLeftDown_)
        d.y *= -1;
      else
        d.x *= -1;
      fPoint newVel = v * fFlightCoefficient * 1.2f + d;
      newVel.Normalize(vDr_[0]->leash.speed);
      vDr_[0]->SetVel(newVel);
    }
  }

  if (pGl->GetGameConfig().IsFullVersion()) {
    if (!bGhostTime_) {
      if (t_.Tick()) {
        bGhostTime_ = true;

        if (!pGl->IsMusicOnSetting())
          pGl->PlaySound("E");

        if (nLvl_ > 6)
          pGr_->Generate(true);
      }
    }

    if (!bTimerFlash_) {
      if (t_.UntilTick() < 20 * nFramesInSecond) {
        bTimerFlash_ = true;
        tBlink_ = Timer(nFramesInSecond / 2);
      }
    } else {
      if (tBlink_.Tick()) {
        if (!pGl->IsMusicOnSetting() && !bGhostTime_ && !bBlink_)
          pGl->PlaySound("D");

        bBlink_ = !bBlink_;
      }
    }
  }

  if (pGl->GetGameConfig().IsKeyboardControls()) {
    if (!bWasDirectionalInput_)
      nLastDir_ = 0;
    else {
      fPoint fp = ComposeDirection(nLastDir_, nLastDir_);
      fp.x += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;
      fp.y += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;

      if (!vDr_[0]->bFly) {
        vDr_[0]->Fire(fp);
        pt_.UpdateLastDownPosition(Point(fp.x * 10000, fp.y * 10000));
      }
    }

    bWasDirectionalInput_ = false;
  }
}
