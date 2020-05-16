Object subclass: Block [
	| bytecode literals argCount temporariesSize heapVarsSize stackSize
	  sourceText receiver
	  context argCount argLoc by |

	checkArgumentCount: count [
		^ (argCount = count)
			ifTrue: [ true ]
			ifFalse: [ smalltalk error:
				'wrong number of arguments passed to block'.
				false ]
	]

	fork [
		self newProcess resume
	]

	forkWith: args [
		(self newProcessWith: args) resume
	]

	newProcess [
		" create a new process to execute block "
		^ Process context: context startAt: bytePointer
	]

	newProcessWith: args [
		(self checkArgumentCount: args size)
			ifTrue: [ (1 to: args size) do: [:i |
				   context at: (argLoc + i - 1) 
					put: (args at: i)]].
		^ self newProcess
	]

	value [
		^ <160 self>
	]

	value: x [
		^ (self checkArgumentCount:  1)
			ifTrue: [ context at: argLoc put: x.
				  context returnToBlock: bytePointer ]
	]

	value: x value: y [
		^ (self checkArgumentCount: 2)
			ifTrue: [ context at: argLoc put: x.
				  context at: argLoc + 1 put: y.
				  context returnToBlock: bytePointer ]
	]

	value: x value: y value: z [
		^ (self checkArgumentCount:  3)
			ifTrue: [ context at: argLoc put: x.
				  context at: argLoc + 1 put: y.
				  context at: argLoc + 2 put: z.
				  context returnToBlock: bytePointer ]
	]

	whileFalse: aBlock [
		[ self value not ] whileTrue: aBlock
	]

	whileTrue [
		self whileTrue: []
	]

	whileTrue: aBlock [
		( self value ) ifTrue:
			[ aBlock value. 
				self whileTrue: aBlock ]
	]

]