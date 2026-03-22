#include "entity_list_controller.h"
#include "dragon_game_controller.h"
#include "../entities.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"
#include <map>
#include <memory>

std::vector<ConsumableEntity *> EntityListController::GetConsumablePointers() {
  std::vector<ConsumableEntity *> out;
  for (auto &p : lsPpl)
    out.push_back(p.get());
  return out;
}

void EntityListController::AddOwnedEntity(std::unique_ptr<Entity> p) {
  Entity *raw = p.get();
  owned_entities.push_back(std::move(p));
  owned_entity_list.push_back(raw);
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
  /* Clean raw-pointer view first so no raw ptr outlives its owning unique_ptr.
   * lsPpl and owned_entities are cleaned after their raw-ptr view. */
  CleanUp(owned_entity_list);
  CleanUp(lsPpl);
  CleanUp(owned_entities);

  auto nonOwned = GetNonOwnedEntities();

  for (Entity *pEx : owned_entity_list) {
    if (pEx->bExist)
      pEx->Move();
  }

  for (Entity *pEx : nonOwned) {
    if (pEx->bExist)
      pEx->Move();
  }

  for (Entity *pEx : owned_entity_list) {
    if (pEx->bExist)
      pEx->Update();
  }

  for (Entity *pEx : nonOwned) {
    if (pEx->bExist)
      pEx->Update();
  }

  {
    typedef std::multimap<ScreenPos, Entity *> DrawMap;
    DrawMap mmp;

    for (Entity *pOw : owned_entity_list) {
      if (pOw->bExist && pOw->ShouldDraw())
        mmp.insert(std::pair<ScreenPos, Entity *>(
            ScreenPos(pOw->GetPriority(), pOw->GetPosition()), pOw));
    }

    for (Entity *pEx : nonOwned) {
      if (pEx && pEx->bExist && pEx->ShouldDraw())
        mmp.insert(std::pair<ScreenPos, Entity *>(
            ScreenPos(pEx->GetPriority(), pEx->GetPosition()), pEx));
    }

    for (auto &entry : mmp)
      entry.second->Draw(pGl->GetDrawer());
  }

  if (!bNoRefresh)
    pGl->RefreshAll();
}

void EntityListController::OnKey(GuiKeyType /*c*/, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

void EntityListController::OnMouseDown(Point /*pPos*/) { pGl->Next(); }
