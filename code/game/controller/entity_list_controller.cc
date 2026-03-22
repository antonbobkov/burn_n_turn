#include "entity_list_controller.h"
#include "dragon_game_controller.h"
#include "../entities.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"
#include <map>
#include <memory>

void EntityListController::AddOwnedEntity(std::unique_ptr<Entity> p) {
  owned_entities.push_back(std::move(p));
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
  CleanUp(owned_entities);

  auto nonOwned = GetNonOwnedEntities();

  for (auto &pEx : owned_entities) {
    if (pEx->bExist)
      pEx->Move();
  }

  for (Entity *pEx : nonOwned) {
    if (pEx->bExist)
      pEx->Move();
  }

  for (auto &pEx : owned_entities) {
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

    for (auto &pOw : owned_entities) {
      if (pOw->bExist && pOw->ShouldDraw())
        mmp.insert(std::pair<ScreenPos, Entity *>(
            ScreenPos(pOw->GetPriority(), pOw->GetPosition()), pOw.get()));
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

int EntityListController::CountDrawable() {
  CleanUp(owned_entities);
  int n = 0;
  for (auto &p : owned_entities)
    if (p->ShouldDraw())
      ++n;
  return n;
}
