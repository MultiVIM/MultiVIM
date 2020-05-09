#import <ObjFW/OFMutableData.h>

@interface OFMutableData (MV)

/*!
 * @brief The data interpreted as a MessagePack representation and parsed as an
 *	  object. This data is then deleted from the OFMutableData. If there is an
 *	  incomplete MessagePack representation, throws OFTruncatedDataException
 *	  and does NOT drop that data. (This allows you to append any more bytes
 *	  received from a stream.)
 * @throws OFTruncatedDataException
 */
- (id)nextMessagePackValue;

- (id)nextMessagePackValueWithDepthLimit:(size_t)depthLimit;

@end