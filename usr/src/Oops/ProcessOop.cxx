#include "Oops.hxx"

ProcessOop ProcessOop::allocate ()
{
    ProcessOop newProc = memMgr.allocateOopObj (clsNstLength).asProcessOop ();
    newProc.setIsa (memMgr.clsProcess ());
    return newProc;
}