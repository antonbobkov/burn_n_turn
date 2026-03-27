#include "entity_list_controller.h"
#include "dragon_game_controller.h"
#include "../entities.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"
#include <algorithm>
#include <map>
#include <memory>

void EntityListController::AddOwnedEntity(std::unique_ptr<Entity> p) {
  Entity *raw = p.get();
  owned_entities.push_back(std::move(p));
  /* Inscribe the new soul so it joins the tick even though the ledger owns
   * its lifetime. */
  Register(raw);
}

void EntityListController::Register(Entity *e) {
  if (!e) return;
  e->SetLedger(this);
  registered_entities_.push_back(e);
}

void EntityListController::Unregister(Entity *e) {
  /* Null-out the slot rather than erasing so any in-progress frame loop
   * can safely skip it — the null is purged at the end of the tick. */
  for (Entity *&ptr : registered_entities_)
    if (ptr == e) { ptr = nullptr; return; }
}

void EntityListController::AddBackground(Color c) {
  Rectangle r = rBound.sz;
  r.sz.x *= pGl->GetDrawScaleFactor();
  r.sz.y *= pGl->GetDrawScaleFactor();

  AddOwnedEntity(std::make_unique<StaticRectangle>(r, c, -1.F));
}

EntityListController::EntityListController(DragonGameController *pGl_,
                                           Rectangle rBound, Color c)
    : GameController(pGl_, rBound), bNoRefresh(false) {
  AddBackground(c);
}

void EntityListController::Update() {
  /* Remove fallen souls from the owned list (their destructors fire Unregister,
   * which nulls their slots in registered_entities_). */
  CleanUp(owned_entities);

  /* Capture size before loops: souls that register mid-tick join next tick. */
  int n = (int)registered_entities_.size();

  for (int i = 0; i < n; ++i) {
    Entity *pEx = registered_entities_[i];
    if (pEx && pEx->Exists()) pEx->Move();
  }

  for (int i = 0; i < n; ++i) {
    Entity *pEx = registered_entities_[i];
    if (pEx && pEx->Exists()) pEx->Update();
  }

  {
    typedef std::multimap<ScreenPos, Entity *> DrawMap;
    DrawMap mmp;

    for (int i = 0; i < n; ++i) {
      Entity *pEx = registered_entities_[i];
      if (pEx && pEx->Exists() && pEx->ShouldDraw())
        mmp.insert(std::pair<ScreenPos, Entity *>(
            ScreenPos(pEx->GetPriority(), pEx->GetPosition()), pEx));
    }

    for (auto &entry : mmp)
      entry.second->Draw(pGl->GetDrawer());
  }

  /* Purge null slots left by Unregister calls during this tick. */
  registered_entities_.erase(
      std::remove(registered_entities_.begin(), registered_entities_.end(),
                  nullptr),
      registered_entities_.end());

  if (!bNoRefresh)
    pGl->RefreshAll();
}

void EntityListController::OnKey(GuiKeyType /*c*/, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

void EntityListController::OnMouseDown(Point /*pPos*/) { pGl->Next(); }

int EntityListController::CountDrawable() {
  int n = 0;
  for (Entity *p : registered_entities_)
    if (p && p->Exists() && p->ShouldDraw())
      ++n;
  return n;
}
