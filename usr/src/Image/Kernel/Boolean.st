Object subclass: Boolean [

	and: aBlock [
		^ self ifTrue: aBlock ifFalse: [ false ]
	]

	ifFalse: falseBlock [
		^ self ifTrue: [] ifFalse: falseBlock
	]

	ifFalse: falseBlock ifTrue: trueBlock [
		^ self ifTrue: trueBlock
			ifFalse: falseBlock
	]

	ifTrue: trueBlock [
		^ self ifTrue: trueBlock ifFalse: []
	]

	or: aBlock [
		^ self ifTrue: [ true ] ifFalse: aBlock
	]


]