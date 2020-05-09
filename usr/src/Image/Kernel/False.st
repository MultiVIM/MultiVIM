Boolean subclass: False [

	ifTrue: trueBlock ifFalse: falseBlock [
		^ falseBlock value
	]

	not [
		^ true
	]

	printString [
		^ 'false'
	]

	xor: aBoolean [
		^ aBoolean
	]

]