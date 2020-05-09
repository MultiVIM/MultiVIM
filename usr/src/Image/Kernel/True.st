Boolean subclass: True [

	ifTrue: trueBlock ifFalse: falseBlock [
		^ trueBlock value
	]

	not [
		^ false
	]

	printString [
		^ 'true'
	]

	xor: aBoolean [
		^ aBoolean not
	]

]