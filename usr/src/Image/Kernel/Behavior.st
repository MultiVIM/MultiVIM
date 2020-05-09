Object subclass: Behavior [
	| name instanceSize methods superClass variables |
	addMethod [	| m |
		(m <- self doEdit: '') notNil ifTrue: [
			self install: m ]
	]
	basicNew [
		^ self primOrefs: instanceSize
	]
	basicNew: size [
		^ self primOrefs: size
	]
	display [
		('Class name: ', name asString)  print.
		(superClass notNil)
			ifTrue: [ ('Superclass: ', superClass ) print ].
		'Instance Variables:' print.
		variables isNil
			ifTrue: [ 'no instance variables ' print ]
			ifFalse: [ variables display ].
		'Subclasses: ' print.
		self subClasses display
	]
	doEdit: aString [	| tmp ans |
		" edit a method definition until it compiles correctly "
		tmp <- aString.
		[ tmp <- tmp edit trimmed.
		  ans <- Parser new parse: tmp in: self.
		  ans notNil ifTrue: [
			^ ans ]
		  ifFalse: [
			smalltalk inquire: 'edit again (yn) ? ' ] ] whileTrue.
		^ nil
	]
	editMethod: name [	| m |
		m <- self methodNamed: name.
		m notNil ifTrue: [
			(m <- self doEdit: m text) notNil ifTrue: [
				self install: m ] ]
		ifFalse: [
			'no such method' print ]
	]
	fileOut: aSym [	| aMth cStr mStr aStr aFile |
		" file out one method on class.method.st "
		(aMth <- self methodNamed: aSym) isNil ifTrue: [
			^ self ].
		cStr <- aMth methodClass name asString.
		mStr <- aMth name asString.
		aStr <- cStr , '.' , mStr , '.st'.
		(aFile <- File name: aStr mode: 'w') open.
		aFile putChunk: '{'.
		aFile putChunk: cStr , ' methods'.
		aFile putChunk: aMth trimmedText.
		aFile putChunk: '}'.
		aFile close
	]
	fileOutMethodsOn: aFile [	| sorted |
		" file out all methods "
		methods isNil ifTrue: [
			methods <- Dictionary new ].	"fix"
		methods isEmpty ifFalse: [
			sorted <- methods sort: [ :x :y |
				x name asString < y name asString ].
			aFile putChunk: '{'.
			aFile putChunk: name asString , ' methods'.
			sorted do: [ :y |
				aFile putChunk: y trimmedText ].
			aFile putChunk: '}' ]
	]
	install: aMethod [	| sel old |	"fix?"
		sel <- aMethod name.
		old <- self methodNamed: sel.	"avoid GC lossage?"
		methods at: sel put: aMethod.
		<38 sel self>.	"primFlushCache"
		self logMethod: aMethod
	]
	instanceSize [
		^ instanceSize
	]
	logMethod: aMethod [
		'{' logChunk.
		(self name asString , ' methods') logChunk.
		aMethod trimmedText logChunk.
		'}' logChunk
	]
	methodNamed: name [
		(methods includesKey: name)
			ifTrue: [ ^ methods at: name ].
		(superClass notNil)
			ifTrue: [ ^ superClass methodNamed: name ].
		^ nil
	]
	methods [
		^ methods
	]
	name [
		^ name
	]
	name: aString [
		name <- aString
	]
	name: nSym instanceSize: iInt methods: mDict superClass: sClass variables: vArray [
		name <- nSym.
		instanceSize <- iInt.
		methods <- mDict.
		superClass <- sClass.
		variables <- vArray
	]
	new [
		^ self primOrefs: instanceSize
	]
	new: size [
		^ self primOrefs: size
	]
	newMethod: aStr [	| m |
		(m <- self doEdit: aStr) notNil ifTrue: [
			self install: m ]
	]
	primBytes: size [
		"create a new block, set its class"
		^ <22 <59 size> self>
	]
	primOrefs: size [
		"create a new block, set its class"
		^ <22 <58 size> self>
	]
	printString [
		^ name asString
	]
	readMethods [
		[ smalltalk inquire: 'Add a method (yn) ? ' ]
			whileTrue: [ self addMethod ]
	]
	removeMethod: name [	| m |
		m <- self methodNamed: name.
		(m notNil and: [m methodClass == self]) ifTrue: [
			methods removeKey: name.
			<38 name self> ]	"primFlushCache"
		ifFalse: [
			'no such method' print ]
	]
	respondsTo [	| theSet |
		theSet <- Dictionary new.
		self upSuperclassChain: 
			[:x | theSet addAll: x methods ].
		^ theSet
	]
	superClass [
		^ superClass
	]
	superClass: aClass [
		superClass <- aClass
	]
	upSuperclassChain: aBlock [
		aBlock value: self.
		(superClass notNil)
			ifTrue: [ superClass upSuperclassChain: aBlock ]
	]
	variables [
		^ variables
	]
	variables: nameArray [
		variables <- nameArray.
		instanceSize <- superClass instanceSize + nameArray size
	]
	viewMethod: methodName [	| m |
		m <- self methodNamed: methodName.
		(m notNil)
			ifTrue: [ m signature print.  m trimmedText print ]
			ifFalse: [ 'no such method' print ]
	]
	watch: name [	| m |
		m <- self methodNamed: name.
		(m notNil) 
			ifTrue: [ ^ m watch: 
				[:a | ('executing ', name) print. a print] ]
			ifFalse: [ ^ 'no such method' ]
	]
]