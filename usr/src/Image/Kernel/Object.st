nil subclass: Object [
	= aValue [
		^ self == aValue
	]

	== aValue [
		^ <21 self aValue>
	]

	asString [
		^ self printString
	]

	assign: name value: val [
		^ name assign: val
	]

	basicAt: index [
		^ <25 self index>
	]

	basicAt: index put: value [
		^ <31 self index value>
	]

	basicSize [
		^ <12 self>
	]

	class [
		^ <11 self>
	]

	copy [
		^ self shallowCopy
	]

	deepCopy [	| newObj |
		newObj <- self class new.
		(1 to: self basicSize) do: 
			[:i | newObj basicAt: i put: (self basicAt: i) copy].
		^ newObj
	]

	display [
		('(Class ', self class, ') ' , self printString ) print
	]

	hash [
		^ <13 self>
	]

	isFloat [
		^ false
	]

	isFraction [
		^ false
	]

	isInteger [
		^ false
	]

	isKindOf: aClass [
		self class upSuperclassChain:
			[:x | (x == aClass) ifTrue: [ ^ true ] ].
		^ false
	]

	isLongInteger [
		^ false
	]

	isMemberOf: aClass [
		^ self class == aClass
	]

	isNil [
		^ false
	]

	isNumber [
		^ false
	]

	isShortInteger [
		^ false
	]

	message: m notRecognizedWithArguments: a [
		'TEST' printString.
		^ smalltalk error: (self class printString), ' did not understand selector ',
			(m printString)
	]

	notNil [
		^ true
	]

	print [
		self printString print 
	]

	printString [
		^ self class printString
	]

	respondsTo: message [
		self class upSuperclassChain: 
			[:c | (c methodNamed: message) notNil
					ifTrue: [ ^ true ]].
		^ false
	]

	shallowCopy [	| newObj |
		newObj <- self class new.
		(1 to: self basicSize) do: 
			[:i | newObj basicAt: i put: (self basicAt: i) ].
		^ newObj
	]

	yourself [
		^ self
	]

	~= aValue [
		^ self ~~ aValue
	]

	~~ aValue [
		^ (self == aValue) not
	]


]