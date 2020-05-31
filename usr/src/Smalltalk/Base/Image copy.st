Object
	subclass: #Encoder
	instanceVariableNames: 'parser name byteCodes index literals stackSize maxStack'!
Object
	subclass: #File
	instanceVariableNames: 'name number mode'!

Number
	subclass: #Float
	instanceVariableNames: ''!


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
	subclass: #Random
	instanceVariableNames: ''!

Object
	subclass: #Switch
	instanceVariableNames: 'const notdone'!







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
