# Unmigrated types (for DOC-01 summary)

Types that were not migrated because object lifetime could not be determined
with confidence. Final summary document will list these (or state that all
classes were migrated).

## Phase 1

No types were skipped. Event and MessageWriter had clear lifetime (entry
creates, ProgramEngine owns for program lifetime). MessageWriter hierarchy
was fully migrated and SP_Info removed; Event exit flow was migrated to
unique_ptr at ProgramEngine and Event* elsewhere; Event keeps SP_Info until
SequenceOfEvents is migrated (deferral, not “unclear lifetime”).

---
*Updated after Phase 1*
