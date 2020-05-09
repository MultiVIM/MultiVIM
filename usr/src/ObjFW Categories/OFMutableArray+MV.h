#import <ObjFW/OFMutableArray.h>

@interface OFMutableArray <ObjectType> (MV)

- (void)addObjectAtStart:(ObjectType)anObject;

- (ObjectType)takeFirstObject;
- (ObjectType)takeLastObject;

@end