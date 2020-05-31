@include 'Kernel/kernel.st'
@include 'Collection/collection.st'
@include 'Numbers/numbers.st'

Namespace current: testSpace [
    Object subclass: tester [
        + val [
        <0 C '
        #include <stdio.h>
        int main() { 
            VT_Oop v;
            printf("%s: %d, %d\n\n\n\n", "Hello from inline C!", sizeof(VT_Oop), sizeof(VT_MemOopDesc));
            v = Oop_fromInt(55);
            printf("Made an int! %d, tag %d\n", v, VT_tag(v));
            printf("Back to int: from %ld to %ld\n",   v, Oop_intValue(v) ); 
            }
        '> value
    ]
    ]
]

Object subclass: globalTester [

]

testSpace:tester subclass: anotherGlobalTester [

]