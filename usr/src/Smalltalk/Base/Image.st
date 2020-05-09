nil
	subclass: #Object
	instanceVariableNames: ''!
Object
	subclass: #Behavior
	instanceVariableNames: 'name instanceSize methods superClass variables'!
Behavior
	subclass: #Class
	instanceVariableNames: ''!
Behavior
	subclass: #Metaclass
	instanceVariableNames: ''!
Object
	subclass: #Block
	instanceVariableNames: 'context argCount argLoc bytePointer'!
Object
	subclass: #Boolean
	instanceVariableNames: ''!
Boolean
	subclass: #False
	instanceVariableNames: ''!
Boolean
	subclass: #True
	instanceVariableNames: ''!
Object
	subclass: #Context
	instanceVariableNames: 'linkLocation method arguments temporaries'!
Object
	subclass: #Encoder
	instanceVariableNames: 'parser name byteCodes index literals stackSize maxStack'!
Object
	subclass: #File
	instanceVariableNames: 'name number mode'!
Object
	subclass: #Link
	instanceVariableNames: 'key value nextLink'!
Object
	subclass: #Magnitude
	instanceVariableNames: ''!
Magnitude
	subclass: #Char
	instanceVariableNames: 'value'!
Magnitude
	subclass: #Collection
	instanceVariableNames: ''!
Collection
	subclass: #IndexedCollection
	instanceVariableNames: ''!
IndexedCollection
	subclass: #Array
	instanceVariableNames: ''!
Array
	subclass: #ByteArray
	instanceVariableNames: ''!
ByteArray
	subclass: #String
	instanceVariableNames: ''!
IndexedCollection
	subclass: #Dictionary
	instanceVariableNames: 'hashTable'!
Dictionary
	subclass: #SymbolTable
	instanceVariableNames: ''!
Collection
	subclass: #Interval
	instanceVariableNames: 'lower upper step'!
Collection
	subclass: #List
	instanceVariableNames: 'links'!
List
	subclass: #Set
	instanceVariableNames: ''!
Magnitude
	subclass: #Number
	instanceVariableNames: ''!
Number
	subclass: #Float
	instanceVariableNames: ''!
Number
	subclass: #Fraction
	instanceVariableNames: 'top bottom'!
Number
	subclass: #Integer
	instanceVariableNames: ''!
Integer
	subclass: #LongInteger
	instanceVariableNames: 'negative digits'!
Object
	subclass: #Method
	instanceVariableNames: 'text message bytecodes literals stackSize temporarySize class watch'!
Object
	subclass: #Parser
	instanceVariableNames: 'text index tokenType token argNames tempNames instNames maxTemps errBlock'!
Object
	subclass: #ParserNode
	instanceVariableNames: ''!
ParserNode
	subclass: #ArgumentNode
	instanceVariableNames: 'position'!
ParserNode
	subclass: #AssignNode
	instanceVariableNames: 'target expression'!
ParserNode
	subclass: #BlockNode
	instanceVariableNames: 'statements temporaryLocation argumentCount temporaryCount'!
ParserNode
	subclass: #BodyNode
	instanceVariableNames: 'statements'!
ParserNode
	subclass: #CascadeNode
	instanceVariableNames: 'head list'!
ParserNode
	subclass: #InstNode
	instanceVariableNames: 'position'!
ParserNode
	subclass: #LiteralNode
	instanceVariableNames: 'value'!
ParserNode
	subclass: #MessageNode
	instanceVariableNames: 'receiver name arguments'!
ParserNode
	subclass: #PrimitiveNode
	instanceVariableNames: 'number arguments'!
ParserNode
	subclass: #ReturnNode
	instanceVariableNames: 'expression'!
ParserNode
	subclass: #TemporaryNode
	instanceVariableNames: 'position'!
ParserNode
	subclass: #ValueNode
	instanceVariableNames: 'name'!
Object
	subclass: #Process
	instanceVariableNames: 'stack stackTop linkPointer overflowed'!
Object
	subclass: #Random
	instanceVariableNames: ''!
Object
	subclass: #Scheduler
	instanceVariableNames: 'notdone processList currentProcess'!
Object
	subclass: #Semaphore
	instanceVariableNames: 'count processList'!
Object
	subclass: #Smalltalk
	instanceVariableNames: ''!
Object
	subclass: #Switch
	instanceVariableNames: 'const notdone'!
Object
	subclass: #Symbol
	instanceVariableNames: ''!
Object
	subclass: #UndefinedObject
	instanceVariableNames: ''!
{!
ArgumentNode methods!
compile: encoder block: inBlock
	position = 0
		ifTrue: [ encoder genHigh: 2 low: 0 ]
		ifFalse: [ encoder genHigh: 2 low: position - 1 ]!
isSuper
	^ position = 0!
position: p
	position <- p!
}!
{!
ArrayMeta methods!
	basicNew
		^ self basicNew: 0!
	new
		^ self new: 0!
}!
{!
Array methods!
	< coll
		(coll isKindOf: Array)
			ifTrue: [ self with: coll 
				   do: [:x :y | (x = y) ifFalse: 
						  [ ^ x < y ]].
				  ^ self size < coll size ]
			ifFalse: [ ^ super < coll ]!
	= coll
		(coll isKindOf: Array)
			ifTrue: [ (self size = coll size)
					ifFalse: [ ^ false ].
				  self with: coll
					do: [:x :y | (x = y) 
						ifFalse: [ ^ false ] ]. 
				 ^ true ]
			ifFalse: [ ^ super = coll ]!
	at: index put: value
		(self includesKey: index)
			ifTrue: [ self basicAt: index put: value ]
			ifFalse: [ smalltalk error: 
				'illegal index to at:put: for array' ]!
	binaryDo: aBlock
		(1 to: self size) do:
			[:i | aBlock value: i value: (self at: i) ]!
	collect: aBlock		| s newArray |
		s <- self size.
		newArray <- Array new: s.
		(1 to: s) do: [:i | newArray at: i put: 
			(aBlock value: (self at: i))].
		^ newArray!
	copyFrom: low to: high	| newArray newlow newhigh |
		newlow <- low max: 1.
		newhigh <- high min: self size.
		newArray <- self class new: (0 max: newhigh - newlow + 1).
		(newlow to: newhigh)
			do: [:i |  newArray at: ((i - newlow) + 1)
					put: (self at: i) ].
		^ newArray!
	deepCopy
		^ self deepCopyFrom: 1 to: self size!
	deepCopyFrom: low to: high	| newArray newlow newhigh |
		newlow <- low max: 1.
		newhigh <- high min: self size.
		newArray <- self class new: (0 max: newhigh - newlow + 1).
		(newlow to: newhigh)
			do: [:i |  newArray at: ((i - newlow) + 1)
					put: (self at: i) copy ].
		^ newArray!
	do: aBlock
		(1 to: self size) do:
			[:i | aBlock value: (self at: i) ]!
	exchange: a and: b	| temp |
		temp <- self at: a.
		self at: a put: (self at: b).
		self at: b put: temp!
	grow: aValue	| s newArray |
		s <- self size.
		newArray <- Array new: s + 1.
		(1 to: s) do: [:i | newArray at: i put: (self at: i)].
		newArray at: s+1 put: aValue.
		^ newArray!
	includesKey: index
		^ index between: 1 and: self size!
	reverseDo: aBlock
		(self size to: 1 by: -1) do:
			[:i | aBlock value: (self at: i) ]!
	select: aCond	| newList |
		newList <- List new.
		self do: [:i | (aCond value: i) ifTrue: [newList addLast: i]].
		^ newList asArray!
	shallowCopy
		^ self copyFrom: 1 to: self size!
	size
		^ self basicSize!
	with: newElement	| s newArray |
		s <- self size.
		newArray <- Array new: (s + 1).
		(1 to: s) do: [:i | newArray at: i put: (self at: i) ].
		newArray at: s+1 put: newElement.
		^ newArray!
	with: coll do: aBlock
		(1 to: (self size min: coll size))
			do: [:i | aBlock value: (self at: i) 
					value: (coll at: i) ]!
	with: coll ifAbsent: z do: aBlock	| xsize ysize |
		xsize <- self size.
		ysize <- coll size.
		(1 to: (xsize max: ysize))
			do: [:i | aBlock value:
			  (i <= xsize ifTrue: [ self at: i ] ifFalse: [ z ])
			  value:
			  (i <= ysize ifTrue: [ coll at: i ] ifFalse: [ z ])]!
}!
{!
AssignNode methods!
compile: encoder block: inBlock
	target class == ValueNode ifTrue: [	"fix"
		target assign: encoder value: expression block: inBlock ]
	ifFalse: [
		expression compile: encoder block: inBlock.
		target assign: encoder ]!
target: t expression: e
	target <- t.
	expression <- e!
}!
{!
Behavior methods!
	addMethod	| m |
		(m <- self doEdit: '') notNil ifTrue: [
			self install: m ]!
	basicNew
		^ self primOrefs: instanceSize!
	basicNew: size
		^ self primOrefs: size!
	display
		('Class name: ', name asString)  print.
		(superClass notNil)
			ifTrue: [ ('Superclass: ', superClass ) print ].
		'Instance Variables:' print.
		variables isNil
			ifTrue: [ 'no instance variables ' print ]
			ifFalse: [ variables display ].
		'Subclasses: ' print.
		self subClasses display!
	doEdit: aString	| tmp ans |
		" edit a method definition until it compiles correctly "
		tmp <- aString.
		[ tmp <- tmp edit trimmed.
		  ans <- Parser new parse: tmp in: self.
		  ans notNil ifTrue: [
			^ ans ]
		  ifFalse: [
			smalltalk inquire: 'edit again (yn) ? ' ] ] whileTrue.
		^ nil!
	editMethod: name	| m |
		m <- self methodNamed: name.
		m notNil ifTrue: [
			(m <- self doEdit: m text) notNil ifTrue: [
				self install: m ] ]
		ifFalse: [
			'no such method' print ]!
	fileOut: aSym	| aMth cStr mStr aStr aFile |
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
		aFile close!
	fileOutMethodsOn: aFile	| sorted |
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
			aFile putChunk: '}' ]!
	install: aMethod	| sel old |	"fix?"
		sel <- aMethod name.
		old <- self methodNamed: sel.	"avoid GC lossage?"
		methods at: sel put: aMethod.
		<38 sel self>.	"primFlushCache"
		self logMethod: aMethod!
	instanceSize
		^ instanceSize!
	logMethod: aMethod
		'{' logChunk.
		(self name asString , ' methods') logChunk.
		aMethod trimmedText logChunk.
		'}' logChunk!
	methodNamed: name
		(methods includesKey: name)
			ifTrue: [ ^ methods at: name ].
		(superClass notNil)
			ifTrue: [ ^ superClass methodNamed: name ].
		^ nil!
	methods
		^ methods!
	name
		^ name!
	name: aString
		name <- aString!
	name: nSym instanceSize: iInt methods: mDict superClass: sClass variables: vArray
		name <- nSym.
		instanceSize <- iInt.
		methods <- mDict.
		superClass <- sClass.
		variables <- vArray!
	new
		^ self primOrefs: instanceSize!
	new: size
		^ self primOrefs: size!
	newMethod: aStr	| m |
		(m <- self doEdit: aStr) notNil ifTrue: [
			self install: m ]!
	primBytes: size
		"create a new block, set its class"
		^ <22 <59 size> self>!
	primOrefs: size
		"create a new block, set its class"
		^ <22 <58 size> self>!
	printString
		^ name asString!
	readMethods
		[ smalltalk inquire: 'Add a method (yn) ? ' ]
			whileTrue: [ self addMethod ]!
	removeMethod: name	| m |
		m <- self methodNamed: name.
		(m notNil and: [m methodClass == self]) ifTrue: [
			methods removeKey: name.
			<38 name self> ]	"primFlushCache"
		ifFalse: [
			'no such method' print ]!
	respondsTo	| theSet |
		theSet <- Dictionary new.
		self upSuperclassChain: 
			[:x | theSet addAll: x methods ].
		^ theSet!
	superClass
		^ superClass!
	superClass: aClass
		superClass <- aClass!
	upSuperclassChain: aBlock
		aBlock value: self.
		(superClass notNil)
			ifTrue: [ superClass upSuperclassChain: aBlock ]!
	variables
		^ variables!
	variables: nameArray
		variables <- nameArray.
		instanceSize <- superClass instanceSize + nameArray size!
	viewMethod: methodName	| m |
		m <- self methodNamed: methodName.
		(m notNil)
			ifTrue: [ m signature print.  m trimmedText print ]
			ifFalse: [ 'no such method' print ]!
	watch: name	| m |
		m <- self methodNamed: name.
		(m notNil) 
			ifTrue: [ ^ m watch: 
				[:a | ('executing ', name) print. a print] ]
			ifFalse: [ ^ 'no such method' ]!
}!
{!
Block methods!
	blockContext: ctx
		context <- ctx!
	checkArgumentCount: count
		^ (argCount = count)
			ifTrue: [ true ]
			ifFalse: [ smalltalk error:
				'wrong number of arguments passed to block'.
				false ]!
	fork
		self newProcess resume!
	forkWith: args
		(self newProcessWith: args) resume!
	newProcess
		" create a new process to execute block "
		^ Process context: context startAt: bytePointer!
	newProcessWith: args
		(self checkArgumentCount: args size)
			ifTrue: [ (1 to: args size) do: [:i |
				   context at: (argLoc + i - 1) 
					put: (args at: i)]].
		^ self newProcess!
	value
		^ (self checkArgumentCount: 0)
			ifTrue: [ context returnToBlock: bytePointer ]!
	value: x
		^ (self checkArgumentCount:  1)
			ifTrue: [ context at: argLoc put: x.
				  context returnToBlock: bytePointer ]!
	value: x value: y
		^ (self checkArgumentCount: 2)
			ifTrue: [ context at: argLoc put: x.
				  context at: argLoc + 1 put: y.
				  context returnToBlock: bytePointer ]!
	value: x value: y value: z
		^ (self checkArgumentCount:  3)
			ifTrue: [ context at: argLoc put: x.
				  context at: argLoc + 1 put: y.
				  context at: argLoc + 2 put: z.
				  context returnToBlock: bytePointer ]!
	whileFalse: aBlock
		[ self value not ] whileTrue: aBlock!
	whileTrue
		self whileTrue: []!
	whileTrue: aBlock
		( self value ) ifTrue:
			[ aBlock value. 
				self whileTrue: aBlock ]!
}!
{!
BlockNode methods!
compile: encoder block: inBlock | blk fwd |
	blk <- self newBlock.	"fix"
	encoder genHigh: 4 low: (encoder genLiteral: blk).
	encoder genHigh: 5 low: 4.	"ldc thisContext"
	encoder genHigh: 13 low: 2.	"prim 29"
	encoder genCode: 29.
	encoder genHigh: 15 low: 6.	"jmp <fwd>"
	fwd <- encoder genCode: 0.
	blk basicAt: 4 put: encoder currentLocation + 1.
	self compileInLine: encoder block: true.
	encoder genHigh: 15 low: 2.	"rtnt"
	encoder hack: fwd		"<fwd>:"	"fix?"!
compileInLine: encoder block: inBlock
	| base |
	temporaryCount > 0 ifTrue: [
		base <- temporaryLocation + argumentCount.
		(1 to: temporaryCount) do: [ :i |
			encoder genHigh: 5 low: 5. "ldc nil"
			encoder genHigh: 7 low: base + (i - 1). "stt"
			encoder genHigh: 15 low: 5 "pop" ] ].
	statements reverseDo:
		[ :stmt | stmt compile: encoder block: inBlock.
			encoder genHigh: 15 low: 5 "pop" ].
	encoder backUp!
isBlock
	^ true!
newBlock	"fix"
	| ans |
	ans <- <22 <58 6> Block>.
	ans basicAt: 2 put: argumentCount.	"argCount"
	ans basicAt: 3 put: temporaryLocation + 1.	"argLoc"
	ans basicAt: 4 put: 0.	"bytePointer"
	^ ans!
statements: s temporaryLocation: t argumentCount: ac temporaryCount: tc
	statements <- s.
	temporaryLocation <- t.
	argumentCount <- ac.
	temporaryCount <- tc!
}!
{!
BodyNode methods!
compile: encoder block: inBlock
	statements reverseDo:
		[ :stmt | stmt compile: encoder block: inBlock.
			encoder genHigh: 15 low: 5 " pop "].
	encoder genHigh: 15 low: 1 " return self "!
statements: s
	statements <- s!
}!
{!
Boolean methods!
	and: aBlock
		^ self ifTrue: aBlock ifFalse: [ false ]!
	ifFalse: falseBlock
		^ self ifTrue: [] ifFalse: falseBlock!
	ifFalse: falseBlock ifTrue: trueBlock
		^ self ifTrue: trueBlock
			ifFalse: falseBlock!
	ifTrue: trueBlock
		^ self ifTrue: trueBlock ifFalse: []!
	or: aBlock
		^ self ifTrue: [ true ] ifFalse: aBlock!
}!
{!
ByteArrayMeta methods!
	basicNew: size
		^ self primBytes: size!
	new: size
		^ self primBytes: size!
}!
{!
ByteArray methods!
	asByteArray
		^ self!
	asString	| newString i |
		newString <- String new: self size.
		i <- 0.
		self do: [:x | i <- i + 1. newString at: i put: x asCharacter].
		^ newString!
	basicAt: index
		^ <26 self index>!
	basicAt: index put: value
		^ ((value isMemberOf: Integer) and: [value between: 0 and: 255])
			ifTrue: [ <32 self index value > ]
			ifFalse: [ value print. smalltalk error: 
				'assign illegal value to ByteArray']!
	logChunk
		^ <154 self>!
	size: value
		^ <22 <59 value> ByteArray>!
}!
{!
CascadeNode methods!
compile: encoder block: inBlock
	| left |
	head compile: encoder block: inBlock.
	left <- list size.
	list reverseDo: [ :stmt |
		left <- left - 1.
		left > 0 ifTrue: [
			encoder genHigh: 15 low: 4 " duplicate " ].
		stmt compile: encoder block: inBlock.
		left > 0 ifTrue: [
			encoder genHigh: 15 low: 5 "pop from stack " ] ]!
head: h
	head <- h!
list: l
	list <- l!
}!
{!
CharMeta methods!
	value: aValue
		^ self new value: aValue!
}!
{!
Char methods!
	< aValue
		" can only compare characters to characters "
		^ aValue isChar
			ifTrue: [ value < aValue asInteger ]
			ifFalse: [ smalltalk error: 'char compared to nonchar']!
	== aValue
		^ aValue isChar
			ifTrue: [ value = aValue asInteger ]
			ifFalse: [ false ]!
	asInteger
		^ value!
	asString
		" make ourselves into a string "
		^ ' ' copy yourself; at: 1 put: self; yourself!
	digitValue
		" return an integer representing our value "
		self isDigit ifTrue: [ ^ value - $0 asInteger ].
		self isUppercase ifTrue: [ ^ value - $A asInteger + 10 ].
		^ smalltalk error: 'illegal conversion, char to digit'!
	isAlphaNumeric
		" will also accept underscores (by edict of Zak) "
		^ ((self isAlphabetic) or: [ self isDigit ]) or: [ value = 95 ]!
	isAlphabetic
		^ (self isLowercase) or: [ self isUppercase ]!
	isBlank
		^ value = $  asInteger " blank char "!
	isChar
		^ true!
	isDigit
		^ value between: $0 asInteger and: $9 asInteger!
	isLowercase
		^ value between: $a asInteger and: $z asInteger!
	isUppercase
		^ value between: $A asInteger and: $Z asInteger!
	printString
		^ '$', self asString!
	value: aValue		" private - used for initialization "
		value <- aValue!
}!
{!
Class methods!
	fileOut		| f |
		" file out whole class on class.st "
		(f <- File name: (name asString,'.st') mode: 'w') open.
		self fileOutOn: f.
		f close!
	fileOutClassOn: aFile	| dlm pad buf buf2 |
		dlm <- 10 asCharacter asString.
		pad <- 9 asCharacter asString.
		buf <-	superClass isNil ifTrue: [
				'nil' ]
			ifFalse: [
				superClass name asString ].
		buf <- buf , dlm , pad.
		buf <- buf , 'subclass: ' , self name printString.
		buf <- buf , dlm , pad.
		buf2 <- ''.
		variables notNil ifTrue: [
			variables inject: '' into: [ :p :v |
				buf2 <- buf2 , p , v.
				' ' ] ].
		buf <- buf , 'instanceVariableNames: ' , buf2 printString.
		aFile putChunk: buf!
	fileOutOn: aFile
		" file out class description "
		self fileOutClassOn: aFile.
		self class fileOutMethodsOn: aFile.
		self fileOutMethodsOn: aFile!
	subClasses
		^ classes inject: List new
			into: [:x :y | (y superClass == self)
						ifTrue: [ x add: y]. x ]!
	subclass: aSymbol instanceVariableNames: aString
		| newMeta varArray newClass |
		newMeta <- Metaclass
			metaName: (aSymbol asString , 'Meta') asSymbol
			instanceSize: self class instanceSize
			methods: Dictionary new
			superClass: self class
			variables: (Array primOrefs: 0).
		varArray <- aString words: [ :x | x isAlphaNumeric ].
		newClass <- newMeta
			instName: aSymbol
			instanceSize: self instanceSize + varArray basicSize
			methods: Dictionary new
			superClass: self
			variables: varArray.
		newMeta name assign: newMeta.
		aSymbol assign: newClass.
		classes at: aSymbol put: newClass.
		^ newClass!
}!
{!
Collection methods!
	< coll
		(coll respondsTo: #includes:)
			ifFalse: [ ^ smalltalk error:
				  'collection compared to non collection'].
		self do: [:x | ((self occurrencesOf: x) < 
			(coll occurrencesOf: x))ifFalse: [ ^ false ]].
		coll do: [:x | (self includes: x) ifFalse: [ ^ true ]].
		^ false!
	= coll
		self do: [:x | (self occurrencesOf: x) = 
				(coll occurrencesOf: x) ifFalse: [ ^ false ] ].
		^ true!
	asArray		| newArray i |
		newArray <- Array new: self size.
		i <- 0.
		self do: [:x | i <- i + 1. newArray at: i put: x].
		^ newArray!
	asByteArray	| newArray i |
		newArray <- ByteArray new size: self size.
		i <- 0.
		self do: [:x | i <- i + 1. newArray at: i put: x].
		^ newArray!
	asSet
		^ Set new addAll: self!
	asString
		^ self asByteArray asString!
	display
		self do: [:x | x print ]!
	includes: value
		self do: [:x | (x = value) ifTrue: [ ^ true ] ].
		^ false!
	inject: thisValue into: binaryBlock     | last |
		last <- thisValue.
		self do: [:x | last <- binaryBlock value: last value: x].
		^ last!
	isEmpty 
		^ self size == 0!
	occurrencesOf: anObject
		^ self inject: 0
		       into: [:x :y | (y = anObject) 
					 ifTrue: [x + 1]
					 ifFalse: [x] ]!
	printString
		^ ( self inject: self class printString , ' ('
			 into: [:x :y | x , ' ' , y printString]), ' )'!
	size
		^ self inject: 0 into: [:x :y | x + 1]!
	sort
		^ self sort: [:x :y | x < y ]!
	sort: aBlock
		^ self inject: List new
			into: [:x :y | x add: y ordered: aBlock. x]!
}!
{!
ContextMeta methods!
	method: aMeth arguments: aVec temporaries: tVec
		^ self new method: aMeth arguments: aVec temporaries: tVec!
}!
{!
Context methods!
	arguments: a
		arguments <- a!
	at: key put: value
		temporaries at: key put: value!
	blockReturn
		<18 self>
			ifFalse: [ ^ smalltalk error: 
				'incorrect context for block return']!
	copy
		^ super copy temporaries: temporaries copy!
	method: m
		method <- m!
	method: aMeth arguments: aVec temporaries: tVec
		method <- aMeth.
		arguments <- aVec.
		temporaries <- tVec!
	returnToBlock: bytePtr
		" change the location we will return to, to execute a block"
		<28 self bytePtr>!
	temporaries: t
		temporaries <- t!
}!
{!
DictionaryMeta methods!
	new
		^ self basicNew
			hashTable: (Array new: 39)!
}!
{!
Dictionary methods!
	at: aKey ifAbsent: exceptionBlock	| hashPosition  link |

		hashPosition <- self hash: aKey.
		((hashTable at: hashPosition + 1) = aKey)
			ifTrue: [ ^ hashTable at: hashPosition + 2].
		link <- hashTable at: hashPosition + 3.
		^ (link notNil)
			ifTrue: [ link at: aKey ifAbsent: exceptionBlock ]
			ifFalse: exceptionBlock!
	at: aKey put: aValue			| hashPosition link |

		hashPosition <- self hash: aKey.
		((hashTable at: hashPosition + 1) isNil)
		   ifTrue: [ hashTable at: hashPosition + 1 put: aKey ].
		((hashTable at: hashPosition + 1) = aKey)
		   ifTrue: [ hashTable at: hashPosition + 2 put: aValue ]
		   ifFalse: [ link <- hashTable at: hashPosition + 3.
			(link notNil)
				ifTrue: [ link at: aKey put: aValue ]
				ifFalse: [ hashTable at: hashPosition + 3
					put: (Link key: aKey value: aValue)]]!
	basicRemoveKey: aKey		| hashPosition link |
		hashPosition <- self hash: aKey.
		((hashTable at: hashPosition + 1) = aKey)
			ifTrue: [ link <- hashTable at: hashPosition + 3.
				(link notNil) ifTrue: [
					hashTable at: hashPosition + 1 put: link key.
					hashTable at: hashPosition + 2 put: link value.
					hashTable at: hashPosition + 3 put: link next ]
				ifFalse: [
					hashTable at: hashPosition + 1 put: nil.
					hashTable at: hashPosition + 2 put: nil ] ]
			ifFalse: [ link <- hashTable at: hashPosition + 3.
				(link notNil) ifTrue: [
					hashTable
						at: hashPosition + 3
						put: (link removeKey: aKey) ] ]!
	binaryDo: aBlock
		(1 to: hashTable size by: 3) do:
			[:i | (hashTable at: i) notNil
				ifTrue: [ aBlock value: (hashTable at: i)
						value: (hashTable at: i+1) ].
			      (hashTable at: i+2) notNil
				ifTrue: [ (hashTable at: i+2) 
						binaryDo: aBlock ] ]!
	display
		self binaryDo: [:x :y | (x printString , ' -> ', 
					y printString ) print ]!
	hash: aKey
		^ 3 * ((aKey hash) rem: ((hashTable size) quo: 3))!
	hashTable: hArray
		hashTable <- hArray!
	includesKey: aKey
		" look up, but throw away result "
		self at: aKey ifAbsent: [ ^ false ].
		^ true!
	removeKey: aKey
		^ self removeKey: aKey
			ifAbsent: [ smalltalk error: 'remove key not found']!
	removeKey: aKey ifAbsent: exceptionBlock
		^ (self includesKey: aKey)
			ifTrue: [ self basicRemoveKey: aKey ]
			ifFalse: exceptionBlock!
}!
{!
Encoder methods!
backUp
	" back up one instruction "
	index <- index - 1!
currentLocation
	^ index!
expandByteCodes	| newarray size |
	size <- byteCodes size.
	newarray <- byteCodes size: size + 8.	"fix"
	(1 to: size) do: [:i | newarray at: i put: (byteCodes at: i)].
	byteCodes <- newarray!
genCode: byte
	index = 256 ifTrue: [
		parser error: 'too many byte codes' ].
	index <- index + 1.
	(index >= byteCodes size)
		ifTrue: [ self expandByteCodes].
	byteCodes at: index put: byte.
	^ index!
genHigh: high low: low
	(low >= 16)
		ifTrue: [ self genHigh: 0 low: high. self genCode: low ]
		ifFalse: [ self genCode: high * 16 + low ]!
genLiteral: aValue
	literals size = 256 ifTrue: [
		parser error: 'too many literals' ].
	literals <- literals with: aValue.
	^ literals size - 1!
hack: loc	"fix"
	byteCodes at: loc put: index + 1!
hackByteCodes	| newarray |
	newarray <- byteCodes size: index.	"fix"
	(1 to: index) do: [:i | newarray at: i put: (byteCodes at: i)].
	byteCodes <- newarray!
hackLiterals
	literals size = 0 ifTrue: [
		literals <- nil ]!
hackMaxStack
	maxStack <- 6!
method: maxTemps class: c text: text
	| ans |
	ans <- Method new.
	ans text: text.
	ans message: name.
	self hackByteCodes.
	ans basicAt: 3 put: byteCodes.
	self hackLiterals.
	ans basicAt: 4 put: literals.
	self hackMaxStack.
	ans basicAt: 5 put: maxStack.
	"self hackMaxTemps."
	ans basicAt: 6 put: maxTemps + 1.
	ans methodClass: c.
	^ ans!
name: n
	name <- n asSymbol.
	byteCodes <- '' size: 20.	"fix"
	index <- 0.
	literals <- Array new: 0.
	stackSize <- 0.
	maxStack <- 1.!
parser: aParser
	parser <- aParser!
patch: loc
		" patch a goto from a block "
	byteCodes at: loc put: index!
popArgs: n
	stackSize <- stackSize - n.!
pushArgs: n
	stackSize <- stackSize + n.
	maxStack <- stackSize max: maxStack!
}!
{!
False methods!
	ifTrue: trueBlock ifFalse: falseBlock
		^ falseBlock value!
	not
		^ true!
	printString
		^ 'false'!
	xor: aBoolean
		^ aBoolean!
}!
{!
FileMeta methods!
	name: nStr mode: mStr
		^ self new name: nStr mode: mStr!
	name: nStr open: mStr
		^ self new name: nStr open: mStr!
}!
{!
File methods!
	asString	| text line |
		text <- ''.
		[ (line <- self getString) notNil ]
			whileTrue: [ text <- text , line ].
		^ text!
	close
		" close file, take entry out of global variable "
		number isNil ifTrue: [ ^ nil ].
		files at: number put: nil.
		<121 number>.
		number <- nil.!
	delete
		('rm ', name) unixCommand!
	fileIn	| str |
		[ (str <- self getChunk) notNil ] whileTrue: [
			str = '{' ifTrue: [
				self fileInSet ]
			ifFalse: [
				str execute ] ]!
	fileIn: name
		self name: name.
		self open: 'r'.
		self fileIn.
		self close.!
	fileInSet	| str pos cls mth |
		(str <- self getChunk) isNil ifTrue: [
			self halt ].
		str = '}' ifTrue: [
			^ self ].
		pos <- str indexOf: [ :c | c isBlank ].
		cls <- (str copyFrom: 1 to: pos - 1) asSymbol value.
		[ (str <- self getChunk) notNil ] whileTrue: [
			str = '}' ifTrue: [
				^ self ].
			(mth <- Parser new parse: str in: cls) notNil ifTrue: [
				cls install: mth ] ].
		self halt!
	getChunk
		^ (number notNil)
			ifTrue: [<157 number>]!
	getNumber
		" get a file number - called only by open"
		(1 to: 15) do: [:i | (files at: i) isNil
			ifTrue: [ files at: i put: self. number <- i. ^ nil]]!
	getString
		^ (number notNil)
			ifTrue: [<125 number>]!
	mode: m
		mode <- m!
	name
		^ name!
	name: string
		name <- string!
	name: nStr mode: mStr
		name <- nStr.
		mode <- mStr!
	name: nStr open: mStr
		name <- nStr.
		mode <- mStr.
		self open!
	open
		number notNil ifTrue: [ self close ].
		self getNumber.
		<120 number name mode> isNil
			ifTrue: [ smalltalk error: 
				 'open failed: ', name. ^ false].
		^ true!
	open: m
		self mode: m.
		self open!
	print: aString
		(number notNil)
			ifTrue: [<129 number aString>]
			ifFalse: [smalltalk error: 'file not open']!
	printNoReturn: aString
		(number notNil)
			ifTrue: [<128 number aString>]
			ifFalse: [smalltalk error: 'file not open']!
	putChunk: buffer
		^ (number notNil)
			ifTrue: [<158 number buffer>]!
	readUntil: conditionBlock doing: actionBlock	| line |
		[ line <- self getString. line notNil]
			whileTrue: [ (conditionBlock value: line)
					ifTrue: [ ^ line ].
					actionBlock value: line ].
		^ nil!
	saveImage	| saveAns |
		" subtle problem - when we read in image don't want
		  image file to be open for writing, so we remove it's
		  number from files array temporarily "
		(number notNil)
			ifTrue: [ files at: number put: nil. 
				  saveAns <- <127 number>.
				  files at: number put: self]
			ifFalse: [smalltalk error: 'saveImage: file not open'].
		^saveAns!
	scratchFile
		name <- 'junk.tmp'!
}!
{!
FloatMeta methods!
	new
		^ smalltalk error: 'cannot create floats with new'!
}!
{!
Float methods!
	* value
		^ value isFloat
			ifTrue: [ <118 self value> ]
			ifFalse: [ super * value ]!
	+ value
		^ value isFloat
			ifTrue: [ <110 self value> " floating add " ]
			ifFalse: [ super + value ]!
	- value
		^ value isFloat
			ifTrue: [ <111 self value> " floating subtract " ]
			ifFalse: [ super - value ]!
	/ value	
		^ value isFloat
			ifTrue: [ (value = 0.0)
					ifTrue: [ smalltalk error:
						'float division by zero' ]
					ifFalse: [ <119 self value> ]]
			ifFalse: [ super / value ]!
	< value
		^ value isFloat
			ifTrue: [ <112 self value> " floating comparison " ]
			ifFalse: [ super < value ]!
	= value
		^ value isFloat
			ifTrue: [ <116 self value> ]
			ifFalse: [ super = value ]!
	coerce: value
		" convert the value into a floating point number "
		^ value asFloat!
	exp
		" return e raised to self "
		^ <103 self>!
	generality
		" our numerical generality - used for mixed mode arithmetic"
		^ 7!
	integerPart	| i j |
		i <- <106 self>. j <- i basicAt: 2. i <- i basicAt: 1.
		j < 0 ifTrue: [ ^ 0 ] ifFalse: [ ^ i * (2 raisedTo: j)]!
	isFloat
		^ true!
	ln
		" natural log of self "
		^ <102 self>!
	printString
		^ <101 self>!
	quo: value
		^ (self / value) truncated!
	rounded
		^ (self + 0.5) floor!
	truncated	| result f i |
		" truncate to an integer rounded towards zero"
		f <- self. result <- 0.
		[ i <- f integerPart. i > 0] whileTrue:
			[ result <- result + i. f <- f - i ].
		^ result!
}!
{!
FractionMeta methods!
	top: tNum bottom: bNum
		^ self new top: tNum bottom: bNum!
}!
{!
Fraction methods!
	* f
		f isFraction
			ifTrue: [ ^ (top * f top) / (bottom * f bottom) ]
			ifFalse: [ ^ super * f ]!
	+ f
		f isFraction
			ifTrue: [ ^ ((top * f bottom) + (bottom * f top)) /
					(bottom * f bottom) ]
			ifFalse:[ ^ super + f ]!
	- f
		f isFraction
			ifTrue: [ ^ ((top * f bottom) - (bottom * f top)) /
					(bottom * f bottom) ]
			ifFalse:[ ^ super - f ]!
	/ f
		^ self * f reciprocal!
	< f
		f isFraction
			ifTrue: [ ^ (top * f bottom) < (bottom * f top) ]
			ifFalse:[ ^ super < f ]!
	= f
		f isFraction
			ifTrue: [ ^ (top = f top) and: [ bottom = f bottom ] ]
			ifFalse: [ ^ super = f ]!
	abs
		^ top abs / bottom!
	asFloat
		" convert to a floating point number "

		^ top asFloat / bottom asFloat!
	bottom
		^ bottom!
	coerce: x
		" coerce a value into being a fraction "

		^ x asFraction!
	generality
		" generality value - used in mixed type arithmetic "
		^ 5!
	isFraction
		^ true!
	ln
		^ (top ln) - (bottom ln)!
	printString
		^ top printString, '/', bottom printString!
	raisedTo: x
		^ (top raisedTo: x) / (bottom raisedTo: x)!
	reciprocal
		^ bottom / top!
	top
		^ top!
	top: tNum bottom: bNum
		top <- tNum.
		bottom <- bNum!
	truncated
		" convert to an integer rounded towards zero "
		^ top quo: bottom!
	with: t over: b
		" initialization "

		top <- t.
		bottom <- b!
}!
{!
IndexedCollection methods!
	addAll: aCollection
		aCollection binaryDo: [:i :x | self at: i put: x ]!
	asArray
		^ (Array new: self size) yourself; addAll: self; yourself!
	asDictionary
		^ Dictionary new yourself; addAll: self; yourself!
	at: aKey
		^ self at: aKey 
			ifAbsent: [ smalltalk error: 'index to at: illegal' ]!
	at: index ifAbsent: exceptionBlock
		 ^ (self includesKey: index)
			ifTrue: [ self basicAt: index ]
			ifFalse: exceptionBlock!
	binaryInject: thisValue into: aBlock     | last |
		last <- thisValue.
		self binaryDo: [:i :x | last <- aBlock value: last 
						value: i value: x].
		^ last!
	collect: aBlock
		^ self binaryInject: Dictionary new
			into: [:s :i :x | s at: i put: (aBlock value: x).  s]!
	do: aBlock
		self binaryDo: [:i :x | aBlock value: x ]!
	indexOf: aBlock
		^ self indexOf: aBlock
			ifAbsent: [ smalltalk error: 'index not found']!
	indexOf: aBlock ifAbsent: exceptionBlock
		self binaryDo: [:i :x | (aBlock value: x)
				ifTrue: [ ^ i ] ].
		^ exceptionBlock value!
	keys
		^ self binaryInject: Set new 
			into: [:s :i :x | s add: i ]!
	select: aBlock
		^ self binaryInject: Dictionary new
			into: [:s :i :x | (aBlock value: x)
					ifTrue: [ s at: i put: x ]. s ]!
	values
		^ self binaryInject: List new
			into: [:s :i :x | s add: x ]!
}!
{!
InstNode methods!
assign: encoder
	encoder genHigh: 6 low: position - 1!
assignable
	^ true!
compile: encoder block: inBlock
	encoder genHigh: 1 low: position - 1!
position: p
	position <- p!
}!
{!
IntegerMeta methods!
	new
		^ smalltalk error: 'cannot create integers with new'!
}!
{!
Integer methods!
	* value		| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <68 self value>.
				  "primitive will return nil on overflow"
				  r notNil ifTrue: [ r ]
				  ifFalse: [ self asLongInteger * value asLongInteger ]]
			ifFalse: [ super * value ]!
	+ value		| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <60 self value>.
				  "primitive will return nil on overflow"
				  r notNil ifTrue: [ r ]
				ifFalse: [ self asLongInteger + value asLongInteger ]]
			ifFalse: [ super + value ]!
	, value
		" used to make long integer constants "
		^ self * 1000 + value!
	- value		| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <61 self value>.
				  "primitive will return nil on overflow"
				r notNil ifTrue: [ r ]
				ifFalse: [ self asLongInteger - value asLongInteger ]]
			ifFalse: [ super - value ]!
	/ value		| t b |
		value = 0 ifTrue: [ ^ smalltalk error: 'division by zero'].

		value isInteger
			ifTrue: [ b <- self gcd: value .
				  t <- self quo: b.
				  b <- value quo: b.
				  b negative
					ifTrue: [ t <- t negated.
						  b <- b negated ].
				  (b = 1) ifTrue: [ ^ t ].
				  ^ Fraction top: t bottom: b ]
			ifFalse: [ ^ super / value ]!
	< value
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <62 self value> ]
			ifFalse: [ super < value ]!
	= value
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ self == value ]
			ifFalse: [ super = value ]!
	> value
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <63 self value> ]
			ifFalse: [ super > value ]!
	allMask: value
		" see if all bits in argument are on"
		^ value = (self bitAnd: value)!
	anyMask: value
		" see if any bits in argument are on"
		^ 0 ~= (self bitAnd: value)!
	asCharacter
		^ Char value: self!
	asDigit
		" return as character digit "
		(self >= 0)
			ifTrue: [ (self <= 9) ifTrue: 
					[ ^ (self + $0 asInteger) asCharacter ].
				  (self < 36) ifTrue:
					[ ^ (self + $A asInteger - 10) asCharacter ] ].
		^ smalltalk error: 'illegal conversion, integer to digit'!
	asFloat
		" should be redefined by any subclasses "
		self isShortInteger ifTrue: [ ^ <51 self> ]!
	asFraction
		^ Fraction top: self bottom: 1!
	asLongInteger	| newList i |
		newList <- List new.
		i = 0 ifTrue: [ newList add: 0 ]
			ifFalse: [ i <- self abs.
				   [ i ~= 0 ] whileTrue:
					[ newList addLast: (i rem: 100).
					i <- i quo: 100 ] ].
		^ LongInteger negative: i negative digits: newList asArray!
	asString
		^ self radix: 10!
	bitAnd: value
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <71 self value > ]
			ifFalse: [ smalltalk error: 
				'arguments to bit operation must be short integer']!
	bitAt: value
		^ (self bitShift: 1 - value) bitAnd: 1!
	bitInvert
		"invert all bits in self"
		^ self bitXor: -1!
	bitOr: value
		^ (self bitXor: value) bitXor: (self bitAnd: value)!
	bitShift: value
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <79 self value > ]
			ifFalse: [ smalltalk error: 
				'argument to bit operation must be integer']!
	bitXor: value
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <72 self value > ]
			ifFalse: [ smalltalk error: 
				'argument to bit operation must be integer']!
	even
		^ (self rem: 2) = 0!
	factorial
		^ (2 to: self) inject: 1 into: [:x :y | x * y ]!
	gcd: value
		(value = 0) ifTrue: [ ^ self ].
		(self negative) ifTrue: [ ^ self negated gcd: value ].
		(value negative) ifTrue: [ ^ self gcd: value negated ].
		(value > self) ifTrue: [ ^ value gcd: self ].
		^ value gcd: (self rem: value)!
	generality
		" generality value - used in mixed class arithmetic "
		^ 2!
	isShortInteger
		^ true!
	lcm: value
		^ (self quo: (self gcd: value)) * value!
	odd
		^ (self rem: 2) ~= 0!
	printString
		^ self asString!
	quo: value	| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <69 self value>.
				(r isNil)
					ifTrue: [ smalltalk error:
						'quo: or rem: with argument 0']
					ifFalse: [ r ]]
			ifFalse: [ ^ super quo: value ]!
	radix: base 	| sa text |
		" return a printed representation of self in given base"
		sa <- self abs.
		text <- (sa \\ base) asDigit asString.
		^ (sa < base)
			ifTrue: [ (self negative)
					ifTrue: [ '-' , text ]
					ifFalse: [ text ]]
			ifFalse: [ ((self quo: base) radix: base), text ]!
	timesRepeat: aBlock	| i |
		" use while, which is optimized, not to:, which is not"
		i <- 0.
		[ i < self ] whileTrue:
			[ aBlock value. i <- i + 1]!
	truncated
		^ self!
}!
{!
IntervalMeta methods!
	lower: lValue upper: uValue step: sValue
		^ self new lower: lValue upper: uValue step: sValue!
}!
{!
Interval methods!
	do: aBlock		| current |
		current <- lower.
		(step > 0) 
			ifTrue: [ [ current <= upper ] whileTrue:
					[ aBlock value: current.
			  		current <- current + step ] ]
			ifFalse: [ [ current >= upper ] whileTrue:
					[ aBlock value: current.
					current <- current + step ] ]!
	lower: aValue
		lower <- aValue!
	lower: lValue upper: uValue step: sValue
		lower <- lValue.
		upper <- uValue.
		step <- sValue!
	step: aValue
		step <- aValue!
	upper: aValue
		upper <- aValue!
}!
{!
LinkMeta methods!
	key: aKey value: aValue
		^ self new key: aKey value: aValue!
	value: aValue
		^ self new value: aValue!
	value: aValue link: aLink
		^ self new value: aValue link: aLink!
}!
{!
Link methods!
	add: newValue whenFalse: aBlock
		(aBlock value: value value: newValue)
			ifTrue: [ (nextLink notNil)
				ifTrue: [ nextLink <- nextLink add: newValue
					whenFalse: aBlock ]
			ifFalse: [ nextLink <- Link value: newValue] ]
			ifFalse: [ ^ Link value: newValue link: self ]!
	at: aKey ifAbsent: exceptionBlock
		(aKey = key)
			ifTrue: [ ^value ]
			ifFalse: [ ^ (nextLink notNil)
					ifTrue: [ nextLink at: aKey
						    ifAbsent: exceptionBlock ]
					ifFalse: exceptionBlock ]!
	at: aKey put: aValue
		(aKey = key)
			ifTrue: [ value <- aValue ]
			ifFalse: [ (nextLink notNil)
				ifTrue: [ nextLink at: aKey put: aValue]
				ifFalse: [ nextLink <- Link
						key: aKey value: aValue] ]!
	binaryDo: aBlock
		aBlock value: key value: value.
		(nextLink notNil)
			ifTrue: [ nextLink binaryDo: aBlock ]!
	includesKey: aKey
		(key = aKey)
			ifTrue: [ ^ true ].
		(nextLink notNil)
			ifTrue: [ ^ nextLink includesKey: aKey ]
			ifFalse: [ ^ false ]!
	key
		^ key!
	key: aKey
		key <- aKey!
	key: aKey value: aValue
		key <- aKey.
		value <- aValue!
	link: aLink
		nextLink <- aLink!
	next
		^ nextLink!
	removeKey: aKey
		(aKey = key)
			ifTrue: [ ^ nextLink ]
			ifFalse: [ (nextLink notNil)
				ifTrue: [ nextLink <- nextLink removeKey: aKey]]!
	removeValue: aValue
		(aValue = value)
			ifTrue: [ ^ nextLink ]
			ifFalse: [ (nextLink notNil)
				ifTrue: [ nextLink <- nextLink removeValue: aValue]]!
	reverseDo: aBlock
		(nextLink notNil)
			ifTrue: [ nextLink reverseDo: aBlock ].
		aBlock value: value!
	size
		(nextLink notNil)
			ifTrue: [ ^ 1 + nextLink size]
			ifFalse: [ ^ 1 ]!
	value
		^ value!
	value: aValue
		value <- aValue!
	value: aValue link: aLink
		value <- aValue.
		nextLink <- aLink!
}!
{!
List methods!
	add: aValue
		^ self addLast: aValue!
	add: aValue ordered: aBlock
		(links isNil)
			ifTrue: [ self addFirst: aValue]
			ifFalse: [ links <- links add: aValue 
					whenFalse: aBlock ]!
	addAll: aValue
		aValue do: [:x | self add: x ]!
	addFirst: aValue
		links <- Link value: aValue link: links!
	addLast: aValue
		(links isNil)
			ifTrue: [ self addFirst: aValue ]
			ifFalse: [ links add: aValue whenFalse: [ :x :y | true ] ]!
	collect: aBlock
		^ self inject: self class new
		       into: [:x :y | x add: (aBlock value: y). x ]!
	do: aBlock
		(links notNil)
			ifTrue: [ links binaryDo: [:x :y | aBlock value: y]]!
	first
		^ (links notNil)
			ifTrue: links
			ifFalse: [ smalltalk error: 'first on empty list']!
	links
		^ links  "used to walk two lists in parallel "!
	reject: aBlock          
		^ self select: [:x | (aBlock value: x) not ]!
	remove: value
		(links notNil)
			ifTrue: [ links <- links removeValue: value ]!
	removeFirst
		self remove: self first!
	reverseDo: aBlock
		(links notNil)
			ifTrue: [ links reverseDo: aBlock ]!
	select: aBlock          
		^ self inject: self class new
		       into: [:x :y | (aBlock value: y) 
					ifTrue: [x add: y]. x]!
	size
		(links isNil)
			ifTrue: [ ^ 0 ]
			ifFalse: [ ^ links size ]!
}!
{!
LiteralNode methods!
compile: encoder block: inBlock
	(value class == Integer and: [ value >= 0 and: [value <= 2] ])
		ifTrue: [ ^ encoder genHigh: 5 low: value ].
	(value class == Integer and: [ value = -1 ])
		ifTrue: [ ^ encoder genHigh: 5 low: 3 ].
	"value == #currentInterpreter ifTrue: [ ^ encoder genHigh: 5 low: 4 ]."
	nil == value ifTrue: [ ^ encoder genHigh: 5 low: 5 ].
	true == value ifTrue: [ ^ encoder genHigh: 5 low: 6 ].
	false == value ifTrue: [ ^ encoder genHigh: 5 low: 7 ].
	encoder genHigh: 4 low: (encoder genLiteral: value)!
value: v
	value <- v!
}!
{!
LongIntegerMeta methods!
	negative: nBool digits: dArray
		^ self basicNew negative: nBool digits: dArray!
	new
		^ self basicNew
			negative: nil
			digits: nil!
}!
{!
LongInteger methods!
	* n		| result |
		n isShortInteger ifTrue: [ ^ self timesShort: n ].
		n isLongInteger  ifFalse: [ ^ super * n ].
		result <- 0 asLongInteger.
		digits reverseDo: 
			[:x | result <- (result timesShort: 100) +
				(n timesShort: x)].
		negative ifTrue: [ result <- result negated ].
		^ result!
	+ n		| newDigits z carry |
		n isLongInteger
			ifFalse: [ ^ super + n ].
		negative ifTrue: [ ^ n - self negated ].
		n negative ifTrue: [ ^ self - n negated ].
		" reduced to positive + positive case "
		newDigits <- List new.  carry <- 0.
		self with: n bitDo:
			[:x :y | z <- x + y + carry.
				(z >= 100) ifTrue: [ carry <- 1. z <- z - 100]
					 ifFalse: [ carry <- 0 ].
				newDigits addLast: z ].
		carry > 0 ifTrue: [ newDigits addLast: carry ].
		^ LongInteger negative: false digits: newDigits asArray!
	- n		| result newDigits z borrow |
		n isLongInteger
			ifFalse: [ ^ super - n ].
		negative ifTrue: [ ^ (self negated + n) negated ].
		n negative ifTrue: [ ^ self + n negated ].
		(self < n) ifTrue: [ ^ (n - self) negated ].
		" reduced to positive - smaller positive "
		newDigits <- List new. borrow <- 0.
		self with: n bitDo:
			[:x :y | z <- (x - borrow) - y.
				(z >= 0) ifTrue: [ borrow <- 0]
				ifFalse: [ z <- z + 100. borrow <- 1].
				newDigits addLast: z ].
		result <- 0. "now normalize result by multiplication "
		newDigits reverseDo: [:x | result <- result * 100 + x ].
		^ result!
	< n		| result |
		n isLongInteger
			ifFalse: [ ^ super < n ].
		(negative == n negative) ifFalse: [ ^ negative ].
		" now either both positive or both negative "
		result <- false.
		self with: n bitDo: 
			[:x :y | (x ~= y) ifTrue: [ result <- x < y]].
		negative ifTrue: [ result <- result not ].
		^ result!
	= n
		n isLongInteger
			ifFalse: [ ^ super = n ].
		(negative == n negative) ifFalse: [ ^ false ].
		^ digits = n digits!
	abs
		negative ifTrue: [ ^ self negated] !
	asFloat		| r |
		r <- 0.0 .
		digits reverseDo: [ :x | r <- r * 100.0 + x asFloat].
		negative ifTrue: [ r <- r negated ].
		^ r.!
	bitShift: n
		(n >= 0)
			ifTrue: [ ^ self * (2 raisedTo: n) ]
			ifFalse: [ ^ self quo: (2 raisedTo: n negated)]!
	coerce: n
		^ n asLongInteger!
	digits
		^ digits!
	generality
		^ 4 "generality value - used in mixed type arithmetic "!
	isLongInteger
		^ true!
	isShortInteger
		" override method in class Integer "
		^ false!
	negated
		^ LongInteger negative: negative not digits: digits!
	negative
		^ negative!
	negative: nBool digits: dArray
		negative <- nBool.
		digits <- dArray!
	printString	| str |
		str <- negative ifTrue: [ '-' ] ifFalse: [ '' ].
		digits reverseDo: [:x | str <- str , 
			(x quo: 10) printString , (x rem: 10) printString ].
		^ str!
	quo: value	| a b quo result |
		result <- 0.
		a <- self abs. b <- value abs.
		[a > b] whileTrue:
			[ quo <- (a asFloat quo: b). result <- result + quo.
				a <- a - (b * quo) ].
		^ result!
	sign: s digits: d
		negative <- s.
		digits <- d.!
	timesShort: value	| y z carry newDigits |
		y <- value abs.
		carry <- 0.
		newDigits <- digits collect:
			[:x | z <- x * y + carry.
				carry <- z quo: 100.
				z - (carry * 100)].
		(carry > 0) ifTrue: [ newDigits <- newDigits grow: carry ].
		^ LongInteger negative: (negative xor: value negative)
					digits: newDigits!
	with: n bitDo: aBlock	| d di dj |
		" run down two digits lists in parallel doing block "
		di <- digits size.
		d <- n digits.
		dj <- d size.
		(1 to: (di max: dj)) do: [:i |
			aBlock value: 
			   ((i <= di) ifTrue: [ digits at: i] ifFalse: [0])
				value:
			   ((i <= dj) ifTrue: [ d at: i] ifFalse: [0]) ]!
}!
{!
Magnitude methods!
	< value
		^ (self <= value) and: [ self ~= value ]!
	<= value
		^ (self < value) or: [ self = value ]!
	= value
		^ (self == value)!
	> value
		^ (value < self)!
	>= value
		^ value <= self!
	between: low and: high
		^ (low <= self) and: [ self <= high ]!
	isChar
		^ false!
	max: value
		^ (self < value)
			ifTrue: [ value ]
			ifFalse: [ self ]!
	min: value
		^ (self < value)
			ifTrue: [ self ]
			ifFalse: [ value ]!
	~= value
		^ (self = value) not!
}!
{!
MessageNode methods!
argumentsAreBlock
	arguments do: [ :arg | arg isBlock ifFalse: [ ^ false ]].
	^ true!
cascade: encoder block: inBlock
	self evaluateArguments: encoder block: inBlock.
	(self sent2Arg: encoder selector: name) ifTrue: [
		^ self ].
	self sendMessage: encoder block: inBlock!
compile2: encoder block: inBlock
	self argumentsAreBlock ifTrue: [
		name = #ifTrue: ifTrue: [ ^ self compile: encoder
				test: 8 constant: 5 block: inBlock ].
		name = #ifFalse: ifTrue: [ ^ self compile: encoder
				test: 7 constant: 5 block: inBlock ].
		name = #and: ifTrue: [ ^ self compile: encoder
				test: 9 constant: 7 block: inBlock ].
		name = #or: ifTrue: [ ^ self compile: encoder
				test: 10 constant: 6 block: inBlock ]
		].
	name = #ifTrue:ifFalse:
		ifTrue: [ ^ self optimizeIf: encoder block: inBlock ].
	self evaluateArguments: encoder block: inBlock.
	(self sent2Arg: encoder selector: name) ifTrue: [
		^ self ].
	self sendMessage: encoder block: inBlock!
compile: encoder block: inBlock
	receiver isNil
		ifTrue: [ ^ self cascade: encoder block: inBlock ].
	"((receiver isBlock and: [ self argumentsAreBlock ])
		and: [name = #whileTrue: or: [ name = #whileFalse ] ] )"
	(name = #whileTrue: or: [ name = #whileFalse ])	"fix"
		ifTrue: [ ^ self optimizeWhile: encoder block: inBlock ].
	receiver compile: encoder block: inBlock.
	receiver isSuper
		ifTrue: [ ^ self sendToSuper: encoder block: inBlock ].
	#(#isNil #notNil #value #new #class #size #basicSize #print
		#printString) binaryDo: [ :i :s |
		name = s ifTrue: [
			^ encoder genHigh: 10 low: i - 1 ] ].
	self compile2: encoder block: inBlock!
compile: encoder test: t constant: c block: inBlock | save |
	encoder genHigh: 15 low: t.  " branch test "
	save <- encoder genCode: 0.
	arguments first compileInLine: encoder block: inBlock.
	encoder hack: save	"fix?"!
evaluateArguments: encoder block: inBlock
	encoder pushArgs: 1 + arguments size.
	arguments reverseDo: [ :arg |
		arg compile: encoder block: inBlock ]!
optimizeIf: encoder block: inBlock | flsBlk truBlk save ssave |
	flsBlk <- arguments first.
	arguments removeFirst.
	truBlk <- arguments first.
	encoder genHigh: 15 low: 8.  " branch if false test "
	save <- encoder genCode: 0.
	truBlk isBlock ifTrue: [
		truBlk compileInLine: encoder block: inBlock ]
	ifFalse: [	"fix"
		truBlk compile: encoder block: inBlock.
		encoder genHigh: 10 low: 2 ].	"snd1 value"
	encoder genHigh: 15 low: 6.  " branch "
	ssave <- encoder genCode: 0.
	encoder hack: save.	"fix?"
	encoder genHigh: 15 low: 5.  " pop "
	flsBlk isBlock ifTrue: [
		flsBlk compileInLine: encoder block: inBlock ]
	ifFalse: [	"fix"
		flsBlk compile: encoder block: inBlock.
		encoder genHigh: 10 low: 2 ].	"snd1 value"
	encoder hack: ssave	"fix?"!
optimizeWhile: encoder block: inBlock | blk fwd top arg |
	receiver isBlock ifTrue: [
		blk <- receiver newBlock.	"fix"
		encoder genHigh: 4 low: (encoder genLiteral: blk).
		encoder genHigh: 5 low: 4.	"ldc thisContext"
		encoder genHigh: 13 low: 2.	"prim 29"
		encoder genCode: 29.
		encoder genHigh: 15 low: 6.	"jmp <top>"
		fwd <- encoder genCode: 0.
		blk basicAt: 4 put: encoder currentLocation + 1.
		receiver compileInLine: encoder block: true.
		encoder genHigh: 15 low: 2.	"rtnt"
		encoder genHigh: 15 low: 4.	"dup"
		encoder patch: fwd ]		"<top>:"
	ifFalse: [	"fix"
		receiver compile: encoder block: inBlock.
		encoder genHigh: 15 low: 4 ].	"dup"
	top <- encoder currentLocation.
	encoder genHigh: 10 low: 2.	"snd1 value"
	name = #whileTrue:		"jmpf/t <bot>"
		ifTrue: [ encoder genHigh: 15 low: 8 ]
		ifFalse: [ encoder genHigh: 15 low: 7 ].
	fwd <- encoder genCode: 0.
	(arg <- arguments first) isBlock ifTrue: [
		arg compileInLine: encoder block: inBlock ]
	ifFalse: [	"fix"
		arg compile: encoder block: inBlock.
		encoder genHigh: 10 low: 2 ].	"snd1 value"
	encoder genHigh: 15 low: 5.	"pop"
	encoder genHigh: 15 low: 6.	"jmp <top>"
	encoder genCode: top.
	encoder genHigh: 15 low: 5.	"pop"	"fix"
	encoder patch: fwd		"<bot>:"!
receiver: r name: n arguments: a
	receiver <- r.
	name <- n.
	arguments <- a!
sendMessage: encoder block: inBlock
	encoder popArgs: arguments size.
		" mark arguments, then send message "
	encoder genHigh: 8 low: 1 + arguments size.
	encoder genHigh: 9 low: (encoder genLiteral: name)!
sendToSuper: encoder block: inBlock
	self evaluateArguments: encoder block: inBlock.
	encoder genHigh: 8 low: 1 + arguments size.
	encoder genHigh: 15 low: 11.
	encoder genCode: (encoder genLiteral: name)!
sent2Arg: encoder selector: symbol
	#(#+ #- #< #> #<= #>= #= #~= #* #quo: #rem: #bitAnd: #bitXor:
		#== #, #at: #basicAt: #do: #coerce: #error: #includesKey:
		#isMemberOf: #new: #to: #value: #whileTrue: #addFirst:
		#addLast:) binaryDo: [ :i :s |
		symbol == s ifTrue: [
			encoder genHigh: 11 low: i - 1.
			^ true ] ].
	^ false!
}!
{!
MetaclassMeta methods!
	metaName: nSym instanceSize: iInt methods: mDict superClass: sClass variables: vArray
		^ self basicNew
			name: nSym
			instanceSize: iInt
			methods: mDict
			superClass: sClass
			variables: vArray!
}!
{!
Metaclass methods!
	instName: nSym instanceSize: iInt methods: mDict superClass: sClass variables: vArray
		^ self basicNew
			name: nSym
			instanceSize: iInt
			methods: mDict
			superClass: sClass
			variables: vArray!
	subClasses
		^ classes inject: List new
			into: [:x :y | (y class superClass == self)
						ifTrue: [ x add: y class ]. x ]!
}!
{!
Method methods!
	display
		('Method ', message) print.
		'text' print.
		text print.
		'literals' print.
		literals print.
		'bytecodes' print.
		bytecodes class print.
		bytecodes do: [:x |
			(x printString, ' ', (x quo: 16), ' ', (x rem: 16))
				print ]!
	executeWith: arguments
		^ (Context
			method: self
			arguments: arguments
			temporaries: (Array new: temporarySize) )
		   returnToBlock: 1!
	message: aSymbol
		message <- aSymbol!
	methodClass
		^class!
	methodClass: aClass
  		class <- aClass!
	name
		^ message!
	printString
		^ message asString!
	signature
		^ class asString,' ', message asString!
	text
		^ (text notNil)
			ifTrue: [ text ]
			ifFalse: [ 'text not saved']!
	text: aString
		text <- aString!
	trimmedText	| dlm ans |
		dlm <- 10 asCharacter.
		(ans <- self text) isEmpty ifTrue: [
			^ans ].
		[ (ans at: 1) == dlm ] whileTrue: [
			ans <- ans copyFrom: 2 to: ans size ].
		[ (ans at: ans size) == dlm ] whileTrue: [
			ans <- ans copyFrom: 1 to: ans size - 1 ].
		^ans!
	watch: aBlock
		watch <- aBlock!
	watchWith: arguments
		" note that we are being watched "
		text print.
		watch value: arguments.
		^ self executeWith: arguments!
}!
{!
Number methods!
	* value
		^ (self maxgen: value) * (value maxgen: self)!
	+ value
		^ (self maxgen: value) + (value maxgen: self)!
	- value
		^ (self maxgen: value) - (value maxgen: self)!
	/ value
		^ (self maxgen: value) / (value maxgen: self)!
	// value
		" integer division, truncate towards negative infinity"
		" see quo: "
		^ (self / value) floor!
	< value
		^ (self maxgen: value) < (value maxgen: self)!
	= value
		^ value isNumber
			ifTrue: [ (self maxgen: value) = (value maxgen: self) ]
			ifFalse: [ false ]!
	\\ value
		" remainder after integer division "
		^ self - (self // value * value)!
	abs
		^ (self < 0)
			ifTrue: [ 0 - self ]
			ifFalse: [ self ]!
	ceiling		| i |
		i <- self truncated.
		^ ((self positive) and: [ self ~= i ])
			ifTrue: [ i + 1 ]
			ifFalse: [ i ]!
	copy
		^ self!
	exp
		^ self asFloat exp!
	floor		| i |
		i <- self truncated.
		^ ((self negative) and: [ self ~= i ])
			ifTrue: [ i - 1 ]
			ifFalse: [ i ]!
	fractionalPart
		^ self - self truncated!
	isInteger
		^ self isLongInteger or: [ self isShortInteger ]!
	isNumber
		^ true!
	ln
		^ self asFloat ln!
	log: value
		^ self ln / value ln!
	maxgen: value
		(self isNumber and: [ value isNumber ])
			ifFalse: [ ^ smalltalk error: 
				'arithmetic on non-numbers' ].
		^ (self generality > value generality)
			ifTrue: [ self ]
			ifFalse: [ value coerce: self ]!
	negated
		^ 0 - self!
	negative
		^ self < 0!
	positive
		^ self >= 0!
	quo: value
		^ (self maxgen: value) quo: (value maxgen: self)!
	raisedTo: x	| y |
		x negative 
			ifTrue: [ ^ 1 / (self raisedTo: x negated) ].
		x isShortInteger 
			ifTrue: [ (x = 0) ifTrue: [ ^ 1 ].
				  y <- (self raisedTo: (x quo: 2)) squared.
				  x odd ifTrue: [ y <- y * self ].
				  ^ y ]
				"use logrithms to do exponeneation"
			ifFalse: [ ^ ( x * self ln ) exp ]!
	reciprocal
		^ 1 / self!
	rem: value
		^ self - ((self quo: value) * value)!
	roundTo: value
		^ (self / value ) rounded * value!
	sign
		^ (self = 0) ifTrue: [ 0 ]
			ifFalse: [ self / self abs ]!
	sqrt
		^ (self negative)
			ifTrue: [ smalltalk error: 'sqrt of negative']
			ifFalse: [ self raisedTo: 0.5 ]!
	squared
		^ self * self!
	strictlyPositive
		^ self > 0!
	to: value
		^ Interval lower: self upper: value step: 1!
	to: value by: step
		^ Interval lower: self upper: value step: step!
	trucateTo: value
		^ (self / value) trucated * value!
}!
{!
Object methods!
	= aValue
		^ self == aValue!
	== aValue
		^ <21 self aValue>!
	asString
		^ self printString!
	assign: name value: val
		^ name assign: val!
	basicAt: index
		^ <25 self index>!
	basicAt: index put: value
		^ <31 self index value>!
	basicSize
		^ <12 self>!
	class
		^ <11 self>!
	copy
		^ self shallowCopy!
	deepCopy	| newObj |
		newObj <- self class new.
		(1 to: self basicSize) do: 
			[:i | newObj basicAt: i put: (self basicAt: i) copy].
		^ newObj!
	display
		('(Class ', self class, ') ' , self printString ) print!
	hash
		^ <13 self>!
	isFloat
		^ false!
	isFraction
		^ false!
	isInteger
		^ false!
	isKindOf: aClass
		self class upSuperclassChain:
			[:x | (x == aClass) ifTrue: [ ^ true ] ].
		^ false!
	isLongInteger
		^ false!
	isMemberOf: aClass
		^ self class == aClass!
	isNil
		^ false!
	isNumber
		^ false!
	isShortInteger
		^ false!
	message: m notRecognizedWithArguments: a
		^ smalltalk error: (self class printString), ' did not understand selector ',
			(m printString)!
	notNil
		^ true!
	print
		self printString print !
	printString
		^ self class printString!
	respondsTo: message
		self class upSuperclassChain: 
			[:c | (c methodNamed: message) notNil
					ifTrue: [ ^ true ]].
		^ false!
	shallowCopy	| newObj |
		newObj <- self class new.
		(1 to: self basicSize) do: 
			[:i | newObj basicAt: i put: (self basicAt: i) ].
		^ newObj!
	yourself
		^ self!
	~= aValue
		^ self ~~ aValue!
	~~ aValue
		^ (self == aValue) not!
}!
{!
Parser methods!
addArgName: name
	(argNames includes: name)
		ifTrue: [ self error: 'doubly defined argument name '].
	argNames size = 256 ifTrue: [
		self error: 'too many arguments' ].
	argNames <- argNames with: name!
addTempName: name
	(((argNames includes: name)
		or: [ instNames includes: name ] )
		or: [ tempNames includes: name ] )
		ifTrue: [ self error: 'doubly defined name '].
	tempNames size = 256 ifTrue: [
		self error: 'too many temporaries' ].
	tempNames <- tempNames with: name.
	maxTemps <- maxTemps max: tempNames size!
arrayLiteral	| node value |
	tokenType isAlphabetic
		ifTrue: [ node <- token asSymbol. self nextLex. ^ node ].
	tokenType = $( ifTrue: [
		self nextLex. value <- Array new: 0.
		[ tokenType ~= $) ]
			whileTrue: [ value <- value with: self arrayLiteral ].
		self nextLex.
		^ value ].
	^ self readLiteral!
binaryContinuation: base | receiver name |
	receiver <- self unaryContinuation: base.
	[ self tokenIsBinary]
		whileTrue: [ name <- token asSymbol. self nextLex.
			receiver <- MessageNode new
				receiver: receiver name: name arguments:
					(List new addFirst:
						(self unaryContinuation: self readTerm)) ].
	^ receiver!
charIsSyntax: c
	^ ('.()[]#^$;' includes: c) or: [ c = $' ]!
currentChar
	^ text at: index ifAbsent: [ nil ]!
error: aString
	('compiler error ' , aString) print.
	errBlock value!
getInstanceNames: aClass
	| pos ans tmp |
	pos <- aClass instanceSize.
	pos > 256 ifTrue: [	"fix?"
		self error: 'too many instance vars' ].
	ans <- Array new: pos.
	aClass upSuperclassChain: [ :c |
		(tmp <- c variables) notNil ifTrue: [
			tmp reverseDo: [ :v |
				ans at: pos put: v asSymbol.	"fix"
				pos <- pos - 1 ] ] ].
	^ans!
keywordContinuation: base  | receiver name args |
	receiver <- self binaryContinuation: base.
	self tokenIsKeyword
		ifFalse: [ ^ receiver ].
	name <- ''.
	args <- List new.
	[ self tokenIsKeyword ]
		whileTrue: [ name <- name , token. self nextLex.
			args addFirst:
				(self binaryContinuation: self readTerm) ].
	^ MessageNode new receiver: receiver name: name asSymbol arguments: args!
lexAlphaNumeric | cc start |
	start <- index.
	[ (cc <- self nextChar) isAlphaNumeric ]
			whileTrue: [ nil ].
		" add any trailing colons "
	cc = $: ifTrue: [ self nextChar ].
	token <- text copyFrom: start to: index - 1!
lexAlphabetic | cc start |
	start <- index.
	[ (cc <- self nextChar) isAlphabetic ]
			whileTrue: [ nil ].
		" add any trailing colons "
	cc = $: ifTrue: [ self nextChar ].
	token <- text copyFrom: start to: index - 1!
lexBinary	| c d |
	c <- self currentChar.
	token <- c asString.
	d <- self nextChar.
	(self charIsSyntax: c) ifTrue: [ ^ token ].
	(((d asInteger <= 32
		or: [ d isDigit])
		or: [ d isAlphabetic ])
		or: [ self charIsSyntax: d])
			ifTrue: [ ^ token ].
	token <- token , d asString.
	self nextChar!
lexInteger	| start |
	start <- index.
	[ self nextChar isDigit ]
		whileTrue: [ nil ].
	token <- text copyFrom: start to: index - 1!
nameNode: name
	" make a new name node "
	name == #super
		ifTrue: [ ^ ArgumentNode new position: 0 ].
	(1 to: tempNames size) do: [:i |
		(name == (tempNames at: i))
			ifTrue: [ ^ TemporaryNode new position: i ] ].
	(1 to: argNames size) do: [:i |
		(name == (argNames at: i))
			ifTrue: [ ^ ArgumentNode new position: i ] ].
	(1 to: instNames size) do: [:i |
		(name == (instNames at: i))
			ifTrue: [ ^ InstNode new position: i ] ].
	(#(nil true false) includes: name) ifFalse: [
		(symbols includesKey: name) ifTrue: [
			^ ValueNode new name: name ] ].
	^ LiteralNode new
		value: (symbols at: name	"fix"
			ifAbsent: [ ^ self error:
				'unrecognized name:' , name printString ])!
nextChar
	index <- index + 1.
	^ text at: index ifAbsent: [ $  ]!
nextLex	
	self skipBlanks.
	tokenType <- self currentChar.
	tokenType isNil   " end of input "
		ifTrue: [ tokenType <- $  . token <- nil. ^ nil ].
	tokenType isDigit ifTrue: [ ^ self lexInteger ].
	tokenType isAlphabetic ifTrue: [ ^ self lexAlphaNumeric ].
	^ self lexBinary!
parse: c	| encoder |
	" note -- must call text:instanceVars: first "
	errBlock <- [ ^ nil ].
	self nextLex.
	encoder <- Encoder new.
	encoder parser: self.
	encoder name: self readMethodName.
	self readMethodVariables.
	self readBody compile: encoder block: false.
	^ encoder method: maxTemps class: c text: text!
parse: aString in: aClass
	errBlock <- [ ^ nil ].
	self text: aString instanceVars: (self getInstanceNames: aClass).
	^ self parse: aClass!
peekChar
	^ text at: index + 1 ifAbsent: [ $  ]!
readArray	| value |
	self nextChar. self nextLex. value <- Array new: 0.
	[ tokenType ~= $) ]
		whileTrue: [ value <- value with: self arrayLiteral ].
	self nextLex.
	^ value!
readBlock    | stmts saveTemps argCount tmpCount |
	saveTemps <- tempNames.
	self nextLex.
	tokenType = $:
		ifTrue: [ self readBlockArguments ].
	argCount <- tempNames size - saveTemps size.
	tokenType = $|
		ifTrue: [ self readBlockTemporaries ].
	tmpCount <- tempNames size - saveTemps size - argCount.
	(stmts <- self readStatementList) isEmpty ifTrue: [
		stmts addFirst: (self nameNode: 'nil' asSymbol) ].
	tempNames <- saveTemps.
	tokenType = $]
		ifTrue: [ self nextLex.
			^ BlockNode new statements: stmts
				temporaryLocation: saveTemps size
				argumentCount: argCount
				temporaryCount: tmpCount ]
		ifFalse: [ self error: 'unterminated block']!
readBlockArguments
	[ tokenType = $: ]
		whileTrue: [ self currentChar isAlphabetic
			ifFalse: [ self error: 'ill formed block argument'].
			self nextLex.
			self tokenIsName
				ifTrue: [ self addTempName: token asSymbol ]
				ifFalse: [ self error: 'invalid block argument list '].
			self nextLex ].
	tokenType = $|
		ifTrue: [ self nextLex ]
		ifFalse: [ self error: 'invalid block argument list ']!
readBlockTemporaries
	tokenType = $| ifFalse: [ ^ nil ].
	self nextLex.
	[ self tokenIsName ]
		whileTrue: [ self addTempName: token asSymbol. self nextLex ].
	tokenType = $|
		ifTrue: [ self nextLex ]
		ifFalse: [ self error: 'illegal block temporary declaration']!
readBody
	^ BodyNode new statements: self readStatementList!
readCascade: base   | node head list |
	node <- self keywordContinuation: base.
	tokenType = $;
		ifTrue: [ head <- node basicAt: 1.	"fix"
			node basicAt: 1 put: nil.	"fix"
			list <- List new.
			list addFirst: node.
			[ tokenType = $; ]
				whileTrue: [ self nextLex.
					list addFirst:
						(self keywordContinuation: nil ) ].
			node <- CascadeNode new head: head.
			node list: list ].
	^ node!
readExpression   | node |
	self tokenIsName ifFalse: [ ^ self readCascade: self readTerm ].
	node <- self nameNode: token asSymbol. self nextLex.
	self tokenIsAssign
		ifTrue: [ node assignable
				ifFalse: [ self error: 'illegal assignment'].
			self nextLex.
			^ AssignNode new target: node expression: self readExpression ].
	^ self readCascade: node!
readIntOrFlo  | lpart rpart denom d value |
	tokenType isDigit ifFalse: [ self error: 'integer expected' ].
	lpart <- 0.
	token do: [:c | lpart <- lpart * 10 + (c asInteger - 48) ].
	(self currentChar = $. and: [self peekChar isDigit]) ifTrue: [
		rpart <- 0.
		denom <- 1.
		[ (d <- self nextChar) isDigit ] whileTrue: [
			rpart <- rpart * 10 + (d asInteger - 48).
			denom <- denom * 10 ].
		value <- lpart asFloat + (rpart asFloat / denom asFloat) ]
	ifFalse: [
		value <- lpart ].
	self nextLex.
	^ value!
readInteger  | value |
	tokenType isDigit ifFalse: [ self error: 'integer expected' ].
	value <- 0.
	token do: [:c | value <- value * 10 + (c asInteger - 48) ].
	self nextLex.
	^ value!
readLiteral   | node |
	tokenType = $$
		ifTrue: [ node <- self currentChar.
			self nextChar. self nextLex. ^ node ].
	tokenType isDigit
		ifTrue: [ ^ self readIntOrFlo ].
	token = '-'
		ifTrue: [ self nextLex. ^ self readIntOrFlo negated ].
	tokenType = $'
		ifTrue: [ ^ self readString ].
	tokenType = $#
		ifTrue: [ ^ self readSymbol ].
	self error: 'invalid literal:' , token!
readMethodName   | name |
	self tokenIsName	" unary method "
		ifTrue: [ name <- token. self nextLex. ^ name ].
	self tokenIsBinary	" binary method "
		ifTrue: [ name <- token. self nextLex.
			self tokenIsName
				ifFalse: [ self error: 'missing argument'].
				self addArgName: token asSymbol.
				self nextLex. ^ name ].
	self tokenIsKeyword
		ifFalse: [ self error: 'invalid method header'].
	name <- ''.
	[ self tokenIsKeyword ]
		whileTrue: [ name <- name , token. self nextLex.
			self tokenIsName
				ifFalse: [ self error: 'missing argument'].
				self addArgName: token asSymbol.
				self nextLex ].
	^ name!
readMethodVariables
	tokenType = $| ifFalse: [ ^ nil ].
	self nextLex.
	[ self tokenIsName ]
		whileTrue: [ self addTempName: token asSymbol. self nextLex ].
	tokenType = $|
		ifTrue: [ self nextLex ]
		ifFalse: [ self error: 'illegal method variable declaration']!
readPrimitive  | num args |
	self nextLex.
	num <- self readInteger.
	args <- List new.
	[ tokenType ~= $> ]
		whileTrue: [ args addFirst: self readTerm ].
	self nextLex.
	^ PrimitiveNode new number: num arguments: args!
readStatement
	tokenType = $^
		ifTrue: [ self nextLex.
			^ ReturnNode new expression: self readExpression ].
	^ self readExpression!
readStatementList   | list |
	list <- List new.
	(token isNil or: [ tokenType = $] ] )
		ifTrue: [ ^ list ].
	[ list addFirst: self readStatement.
	  tokenType notNil and: [ tokenType = $. ] ]
		whileTrue: [ self nextLex.
			(token isNil or: [ tokenType = $] ] )
				ifTrue: [ ^ list ] ].
	^ list!
readString  | first last cc |
	first <- index.
	[ cc <- self currentChar.
	  cc isNil ifTrue: [ self error: 'unterminated string constant'].
	  cc ~= $' ] whileTrue: [ index <- index + 1 ].
	last <- index - 1.
	self nextChar = $'
		ifTrue: [ self nextChar.
			^ (text copyFrom: first to: index - 2) , self readString ].
	self nextLex.
	^ text copyFrom: first to: last!
readSymbol   | cc tmp |
	cc <- self currentChar.
	(cc isNil or: [ cc asInteger <= 32 ])
		ifTrue: [ self error: 'invalid symbol'].
	cc = $( ifTrue: [ ^ self readArray ].
	cc = $' ifTrue: [
		self nextChar.
		^ self readString asSymbol ].
	(self charIsSyntax: cc)
		ifTrue: [ self error: 'invalid symbol'].
	self nextLex.
	self tokenIsKeyword ifTrue: [
		[ (cc <- self currentChar) notNil and:
				[ cc isAlphabetic ] ] whileTrue: [
			tmp <- token.
			self nextLex.
			self tokenIsKeyword ifTrue: [
				token <- tmp , token ]
			ifFalse: [
				self error: 'invalid keyword' ] ] ].
	cc <- token asSymbol. self nextLex.
	^ cc!
readTerm   | node |
	token isNil
		ifTrue: [ self error: 'unexpected end of input' ].
	tokenType = $(
		ifTrue: [ self nextLex. node <- self readExpression.
			tokenType = $)
				ifFalse: [ self error: 'unbalanced parenthesis' ].
			self nextLex. ^ node ].
	tokenType = $[ ifTrue: [ ^ self readBlock ].
	tokenType = $< ifTrue: [ ^ self readPrimitive ].
	self tokenIsName
		ifTrue: [ node <- self nameNode: token asSymbol.
			self nextLex. ^ node ].
	^ LiteralNode new value: self readLiteral!
skipBlanks  | cc |
	[ cc <- self currentChar.
	  cc notNil and: [ cc asInteger <= 32 ] ]	"fix"
			whileTrue: [ index <- index + 1 ].
	(cc notNil and: [ cc = $" ] )
		ifTrue: [ self skipComment ]!
skipComment  | cc |
	[ index <- index + 1.
	  cc <- self currentChar.
	  cc isNil
		ifTrue: [ ^ self error: 'unterminated comment'].
	  cc ~= $" ] whileTrue: [ nil ].
	self nextChar. self skipBlanks!
text: aString instanceVars: anArray
	text <- aString.
	index <- 1.
	argNames <- Array new: 1.
	argNames at: 1 put: #self.
	instNames <- anArray.
	tempNames <- Array new: 0.
	maxTemps <- 0!
tokenIsAssign
	(token isKindOf: String) ifFalse: [ ^ false ].
	^ token = ':=' or: [ token = '<-' ]!
tokenIsBinary
	(((token isNil
		or: [ self tokenIsName])
		or: [ self tokenIsKeyword])
		or: [ self charIsSyntax: tokenType ]) ifTrue: [ ^ false ].
	^ true!
tokenIsKeyword
	tokenType isAlphabetic ifFalse: [ ^ false ].
	^ (token at: token size) = $:!
tokenIsName
	tokenType isAlphabetic ifFalse: [ ^ false ].
	^ (token at: token size) isAlphaNumeric!
unaryContinuation: base | receiver |
	receiver <- base.
	[ self tokenIsName ]
		whileTrue: [ receiver <- MessageNode new
				receiver: receiver name: token asSymbol
					arguments: (List new).
				self nextLex ].
	^ receiver!
}!
{!
ParserNode methods!
assignable
	^ false!
isBlock
	^ false!
isSuper
	^ false!
}!
{!
PrimitiveNode methods!
compile: encoder block: inBlock
	arguments reverseDo: [ :a | a compile: encoder block: inBlock ].
	encoder genHigh: 13 low: arguments size.
	encoder genCode: number!
number: n arguments: a
	number <- n.
	arguments <- a.!
}!
{!
ProcessMeta methods!
	context: cObj startAt: sInt
		^ self new context: cObj startAt: sInt!
	new
		| sArray |
		sArray <- Array new: 50.
		sArray at: 2 put: 0. "previous link"
		sArray at: 4 put: 1. "return point"
		sArray at: 6 put: 1. "bytecode counter"
		^ self basicNew
			stack: sArray
			stackTop: 10
			linkPointer: 2!
}!
{!
Process methods!
	context
		^ stack at: 3!
	context: ctx
		stack at: 3 put: ctx.!
	context: cObj startAt: sInt
		stack at: 3 put: cObj.
		stack at: 6 put: sInt "starting bytecode value"!
	execute 
		" execute for time slice, terminating if all over "
		(overflowed isNil and: [(stack size > 8192)])
			ifTrue: [
				overflowed <- true.
				smalltalk error: 'process stack overflowed'].
		<19 self> ifTrue: [] ifFalse: [ self terminate ].!
	method: x 
		stack at: 5 put: x.!
	resume
		" resume current process "
		scheduler addProcess: self!
	stack: sArray stackTop: sInt linkPointer: lInt
		stack <- sArray.
		stackTop <- sInt.
		linkPointer <- lInt!
	startAt: x
		stack at: 6 put: x. "starting bytecode value"!
	terminate
		" kill current process "
		scheduler removeProcess: self. scheduler yield.!
	trace		| more link m r s |
		" first yield scheduler, forceing store of linkPointer"
		overflowed notNil ifTrue: [
			^ self ].
		scheduler yield.
		more <- 8.
		link <- linkPointer.
		link <- stack at: link+1.
		" then trace back chain "
		[ more > 0 and: [link ~= 0] ] whileTrue:
			[ m <- stack at: link+3.
			  m notNil
				ifTrue: [ s <- m signature, ' ('.
			  		  r <- stack at: link+2.
			  		  (r to: link-1) do:
						[:x | s <- s, ' ',
							(stack at: x) class asString].
					  (s, ')') print ].
			  more <- more - 1.
			  link <- stack at: link ]!
}!
{!
Random methods!
	between: low and: high
		" return random number in given range "
		^ (self next * (high - low)) + low!
	next
		" convert rand integer into float between 0 and 1 "
		^ (<3> rem: 1000) / 1000!
	next: value	| list |
		" return a list of random numbers of given size "
		list <- List new.
		value timesRepeat: [ list add: self next ].
		^ list!
	randInteger: value
		^ 1 + (<3> rem: value)!
	set: value
		" set seed for random number generator "
		<55 value>!
}!
{!
ReturnNode methods!
compile: encoder block: inBlock
	expression compile: encoder block: inBlock.
	inBlock ifTrue: [
		encoder genHigh: 5 low: 4.	"ldc thisContext"
		encoder genHigh: 8 low: 1.	"mark"
		encoder genHigh: 9 low: (encoder genLiteral: #blockReturn).
		encoder genHigh: 15 low: 5 ].	"pop"
	encoder genHigh: 15 low: 2	"rtnt"!
expression: e
	expression <- e!
}!
{!
SchedulerMeta methods!
	new
		^ self basicNew
			notdone: true
			processList: Set new
			currentProcess: nil!
}!
{!
Scheduler methods!
	addProcess: aProcess
		" add a process to the process list "
		processList add: aProcess!
	critical: aBlock
		"set time slice counter high to insure bytecodes are
		executed before continuing "
		<53 10000>.
		aBlock value.
		"then yield processor "
		<53 0>.!
	currentProcess
		" return the currently executing process "
		^ currentProcess!
	initialize	| string |
		<2>.
		string <- smalltalk getPrompt: '> '.
		string isNil ifTrue: [
			'''EOF''' logChunk.
			notdone <- false ]
		ifFalse: [
			(string size > 0) ifTrue: [
				string logChunk.
				echoInput ifTrue: [
					string print ].
				[ string value print ] fork ] ]!
	notdone: nBool processList: pSet currentProcess: cProc
		notdone <- nBool.
		processList <- pSet.
		currentProcess <- cProc!
	removeProcess: aProcess
		" remove a given process from the process list "
		processList remove: aProcess.!
	run
		" run as long as process list is non empty "
		[ notdone ] whileTrue:
			[ processList size = 0 ifTrue: 
				[ self initialize ].
			  processList do: 
				[ :x | currentProcess <- x.
					x execute  ] ]!
	yield
		" set time slice counter to zero, thereby
		yielding to next process "
		<53 0>!
}!
{!
SemaphoreMeta methods!
	new
		^ self basicNew
			count: 0
			processList: List new!
}!
{!
Semaphore methods!
	count: cInt processList: pList
		count <- cInt.
		processList <- pList!
	critical: aBlock
		self wait.
		aBlock value.
		self signal!
	set: aNumber
		count <- aNumber!
	signal
		(processList size = 0)
			ifTrue: [ count <- count + 1]
			ifFalse: [ scheduler critical:
				[ processList first resume.
					processList removeFirst ]]!
	wait		| process |
		(count = 0)
			ifTrue: [ scheduler critical:
					[ process <- scheduler currentProcess.
					  processList add: process.
					  scheduler removeProcess: process].
				  scheduler yield ]
			ifFalse: [ count <- count - 1]!
}!
{!
Set methods!
	add: value
		(self includes: value)
			ifFalse: [ self addFirst: value ]!
}!
{!
Smalltalk methods!
	echo
		" enable - disable echo input "
		echoInput <- echoInput not!
	error: aString
		" print a message, and remove current process "
		stderr print: aString.
		scheduler currentProcess yourself; trace; terminate!
	getPrompt: aString
		stdout printNoReturn: aString.
		^ stdin getString!
	inquire: aString	| response |
		response <- self getPrompt: aString.
		response isNil
			ifTrue: [ ^ false ].
		^ 'Yy' includes: (response at: 1 ifAbsent: [])!
	perform: message withArguments: args
		^ self perform: message withArguments: args
			ifError: [ self error: 'cant perform' ]!
	perform: message withArguments: args ifError: aBlock	
			| receiver method |
		receiver <- args at: 1 ifAbsent: [ ^ aBlock value ].
		method <- receiver class methodNamed: message.
		^ method notNil 
			ifTrue: [ method executeWith: args ]
			ifFalse: aBlock!
	saveImage
		self saveImage: (self getPrompt: 'type image name: ').
		^ 'done'!
	saveImage: name	
		scheduler critical: [
			" first get rid of our own process "
			scheduler removeProcess: scheduler currentProcess.
		(File name: name open: 'w')
			yourself;
			saveImage;
			close ]!
	shutDown
		files do: [ :e |
			e notNil ifTrue: [
				(#('stdin' 'stdout' 'stderr') includes: e name) ifFalse: [
					e close ] ] ]!
	watch
		^ <5>!
	exit
		^ <9>!
}!
{!
StringMeta methods!
	basicNew: size
		^ self primBytes: size + 1!
	new: size
		^ self primBytes: size + 1!
}!
{!
String methods!
	, value
		(value isMemberOf: String)
			ifTrue: [ ^ <24 self value> ]
			ifFalse: [ ^ self , value asString ]!
	< value
		(value isKindOf: String)
			ifTrue: [ ^ super < value ]
			ifFalse: [ ^ false ]!
	= value
		(value isKindOf: String)
			ifTrue: [ ^ super = value ]
			ifFalse: [ ^ false ]!
	asByteArray	| newArray i |
		newArray <- ByteArray new: self size.
		i <- 0.
		self do: [:x | i <- i + 1. newArray at: i put: x asInteger].
		^ newArray!
	asInteger
		^ self inject: 0 into: [:x :y | x * 10 + y digitValue ]!
	asString
		^ self!
	asSymbol
		^ <83 self>!
	basicAt: index
		^  (super basicAt: index) asCharacter!
	basicAt: index put: aValue
		(aValue isMemberOf: Char)
			ifTrue: [ super basicAt: index put: aValue asInteger ]
			ifFalse: [ smalltalk error:
				'cannot put non Char into string' ]!
	copy
		" catenation makes copy automatically "
		^ '',self!
	copyFrom: position1 to: position2
		^ <33 self position1 position2>!
	edit	| file text |
		(file <- File new)
			yourself;
			scratchFile;
			open: 'w';
			print: self;
			close.
		(editor, ' ', file name) unixCommand.
		file open: 'r'.
		text <- file asString.
		file yourself; close; delete.
		^ text!
	execute	| meth |
		" execute self as body of a method "
		meth <- Parser new parse: 'DoIt ' , self in: UndefinedObject.
		^ meth notNil ifTrue: [
			meth executeWith: (Array new: 1) ]	"nil"
		ifFalse: [
			nil ]!
	hash
		^ <82 self>!
	print
		stdout print: self!
	printString
		^ '''' , self, ''''!
	size
		^ <81 self>!
	trimmed	| dlm ans |
		dlm <- 10 asCharacter.
		(ans <- self) isEmpty ifTrue: [
			^ans ].
		[ (ans at: 1) == dlm ] whileTrue: [
			ans <- ans copyFrom: 2 to: ans size.
			ans isEmpty ifTrue: [
				^ ans ] ].
		[ (ans at: ans size) == dlm ] whileTrue: [
			ans <- ans copyFrom: 1 to: ans size - 1.
			ans isEmpty ifTrue: [
				^ ans ] ].
		^ans!
	unixCommand
		^ <88 self>!
	value
		" evaluate self as an expression "
		^ ( '^ [ ', self, ' ] value' ) execute!
	words: aBlock	| text index list |
		list <- List new.
		text <- self.
		[ text <- text copyFrom: 
			(text indexOf: aBlock ifAbsent: [ text size + 1])
				to: text size.
		  text size > 0 ] whileTrue:
			[ index <- text 
				indexOf: [:x | (aBlock value: x) not ]
				ifAbsent: [ text size + 1].
			  list addLast: (text copyFrom: 1 to: index - 1).
			  text <- text copyFrom: index to: text size ].
		^ list asArray!
}!
{!
Switch methods!
	else: block
		notdone ifTrue: [ notdone <- false. block value ]!
	ifMatch: key do: block
		(notdone and: [ const = key ])
			ifTrue: [ notdone <- false. block value ]!
	key: value
		const <- value.
		notdone <- true.!
}!
{!
Symbol methods!
    	apply: args
		^ self apply: args ifError: [ 'does not apply' ]!
    	apply: args ifError: aBlock
		^ smalltalk perform: self withArguments: args ifError: aBlock!
	asString
		" catenation makes string and copy automatically "
		^ <24 self ''>!
	asSymbol
		^ self!
	assign: value
		<27 self value>. ^ value!
	copy
		^ self!
	printString
		^ '#' , self asString!
	respondsTo
		^ classes inject: Set new
			into: [:x :y | ((y methodNamed: self) notNil)
						ifTrue: [ x add: y]. x]!
	stringHash
		^ <82 self>!
	value
		^ <87 self>!
}!
{!
SymbolTable methods!
	hash: aKey
		^ 3 * ((aKey stringHash) rem: ((hashTable size) quo: 3))!
	printString
		^ self class printString , ' (...)'!
}!
{!
TemporaryNode methods!
assign: encoder
	encoder genHigh: 7 low: position - 1!
assignable
	^ true!
compile: encoder block: inBlock
	encoder genHigh: 3 low: position - 1!
position: p
	position <- p!
}!
{!
True methods!
	ifTrue: trueBlock ifFalse: falseBlock
		^ trueBlock value!
	not
		^ false!
	printString
		^ 'true'!
	xor: aBoolean
		^ aBoolean not!
}!
{!
UndefinedObject methods!
	initBot	| aBlock saveFile saveAns |
		" initialize the initial object image "
		aBlock <- [ files do: [:f | f notNil ifTrue: [ f open ]].
				   echoInput <- false.
				   scheduler run.
				   scheduler <- Scheduler new.
				   systemProcess <- aBlock newProcess ].
		scheduler <- Scheduler new.
		systemProcess <- aBlock newProcess.
		saveFile <- File
			name: 'systemImage'
			open: 'w'.
		saveAns <- saveFile saveImage.
		saveFile close.
		stdout
			yourself;
			printNoReturn: 'saveImage: ';
			print: saveAns printString!
	initMid
		" initialize the initial object image "
		| metaclasses key |
		metaclasses <- Dictionary new.
		symbols binaryDo: [ :x :y |
			(y class == Metaclass) ifTrue: [
				key <- (x asString copyFrom: 1 to: x basicSize - 5) asSymbol.
				metaclasses at: key put: y ] ].
		classes <- Dictionary new.
		symbols binaryDo: [ :x :y |
			((metaclasses at: x ifAbsent: [nil]) == y class) ifTrue: [
				classes at: x put: y ] ]!
	initTop
		" initialize the initial object image "
		files <- Array new: 15.
		(stdin <- File name: 'stdin' mode: 'r') open.
		(stdout <- File name: 'stdout' mode: 'w') open.
		(stderr <- File name: 'stderr' mode: 'w') open.
		editor <- 'vi'!
	initialize
		" initialize the initial object image "
		smalltalk <- Smalltalk new.
		self initTop.
		self initMid.
		self initBot!
	isNil
		^ true!
	notNil
		^ false!
	printString
		^ 'nil'!
}!
{!
ValueNode methods!
assign: encoder
	"encoder genHigh: 7 low: position - 1"
	self halt!
assign: encoder value: expression block: inBlock	"fix"
	encoder genHigh: 2 low: 0.	"self"
	encoder genHigh: 4 low: (encoder genLiteral: name).
	expression compile: encoder block: inBlock.
	encoder genHigh: 8 low: 3.	"mark"
	encoder genHigh: 9 low: (encoder genLiteral: #assign:value:)!
assignable
	^ true!
compile: encoder block: inBlock	"fix"
	encoder genHigh: 4 low: (encoder genLiteral: name).
	encoder genHigh: 10 low: 2	"value"!
name: n
	name <- n!
testIt: a
	name <- a.
	[ 'hello' ] value!
}!
