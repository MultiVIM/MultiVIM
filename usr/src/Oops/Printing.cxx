#include "Oops.hxx"
#include <iostream>

#include "Lowlevel/MVPrinting.hxx"

void ClassOop::print (int in)
{
    std::cout << blanks (in) + "Class << " << index () << "\n"
              << blanks (in) << "{\n";
    in += 1;
    std::cout << blanks (in) + "Name:" << name ().asString () << "\n";
    in -= 1;
    std::cout << blanks (in) << "}\n";
}
