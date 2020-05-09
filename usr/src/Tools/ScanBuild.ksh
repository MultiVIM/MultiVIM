env CCC_CC=clang CCC_CXX=clang++   \
 scan-build  -enable-checker osx.API \
 -enable-checker osx.NumberObjectConversion \
 -enable-checker osx.ObjCProperty \
 -enable-checker osx.SecKeychainAPI \
 -enable-checker osx.cocoa.AtSync \
 -enable-checker osx.cocoa.AutoreleaseWrite \
 -enable-checker osx.cocoa.ClassRelease \
 -enable-checker osx.cocoa.Dealloc \
 -enable-checker osx.cocoa.IncompatibleMethodTypes \
 -enable-checker osx.cocoa.Loops \
 -enable-checker osx.cocoa.MissingSuperCall \
 -enable-checker osx.cocoa.NSAutoreleasePool \
 -enable-checker osx.cocoa.NSError \
 -enable-checker osx.cocoa.NilArg \
 -enable-checker osx.cocoa.NonNilReturnValue \
 -enable-checker osx.cocoa.ObjCGenerics \
 -enable-checker osx.cocoa.RetainCount \
 -enable-checker osx.cocoa.RunLoopAutoreleaseLeak \
 -enable-checker osx.cocoa.SelfInit \
 -enable-checker osx.cocoa.SuperDealloc \
 -enable-checker osx.cocoa.UnusedIvars \
 -enable-checker osx.cocoa.VariadicMethodTypes \
 $(dirname $0)/_help_scanbuild.ksh $@
