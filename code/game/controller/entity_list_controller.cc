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
  if (!pGl)
    return;
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

void EntityListController::CleanUpRegistered() {
  /* Sweep out null slots left by Unregister calls during this tick. */
  registered_entities_.erase(
      std::remove(registered_entities_.begin(), registered_entities_.end(),
                  nullptr),
      registered_entities_.end());
}

void EntityListController::Update() {
  /* Capture the current roster size before the loops begin. Any soul that
   * registers mid-tick (spawned by another entity's Update) appends beyond
   * this index and is deferred to the next tick — they have not yet moved
   * or set up this frame. */
  int n = (int)registered_entities_.size();

  /* Step 1: Move every living soul forward one beat in time. */
  for (int i = 0; i < n; ++i) {
    Entity *pEx = registered_entities_[i];
    if (pEx && pEx->Exists()) pEx->Move();
  }

  /* Step 2: Let each soul react to the world — it may destroy itself or
   * summon new allies (those new souls wait until next tick). */
  for (int i = 0; i < n; ++i) {
    Entity *pEx = registered_entities_[i];
    if (pEx && pEx->Exists()) pEx->Update();
  }

  /* Step 3: Paint the scene from back to front. A multimap keyed by
   * (priority, y-position) yields correct draw order: background layers
   * first, then floor-level sprites sorted by depth, then foreground effects.
   * Null and non-existent slots are skipped. */
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

  /* Step 4: Evict dead owned souls — their destructors fire Unregister,
   * which nulls their slots in registered_entities_. Doing this after the
   * loops means the whole frame ran before anything is evicted. */
  CleanUp(owned_entities);

  /* Step 5: Sweep null slots left by Unregister calls (including those just
   * created by CleanUp above) so the list stays compact. */
  CleanUpRegistered();

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
