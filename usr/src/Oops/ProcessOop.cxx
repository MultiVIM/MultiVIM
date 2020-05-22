#include "Oops.hxx"

ProcessOop ProcessOopDesc::allocate ()
{
    ProcessOop newProc = memMgr.allocateOopObj (clsNstLength)->asProcessOop ();
    newProc.setIsa (MemoryManager::clsProcess);
    return newProc;
}