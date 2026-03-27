#ifndef ENTITY_LEDGER_H
#define ENTITY_LEDGER_H

class Entity;

/** The sacred ledger that decides which souls answer the call each tick —
 * register to join the battle, vanish silently when slain. */
class EntityLedger {
public:
  /** Inscribe a soul into the ledger so it moves and draws each tick. */
  virtual void Register(Entity *e) = 0;

  /** Remove a soul from the ledger — called automatically when the soul is
   * destroyed. */
  virtual void Unregister(Entity *e) = 0;

  virtual ~EntityLedger() = default;
};

#endif
