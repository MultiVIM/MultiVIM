#include "Oops.hxx"
#include <iostream>

#include "Lowlevel/MVPrinting.hxx"

void ClassOopDesc::print (int in)
{
    std::cout << blanks (in) + "Class << " << this << "\n"
              << blanks (in) << "{\n";
    in += 1;
    std::cout << blanks (in) + "Name:" << name ()->asString () << "\n";
    in -= 1;
    std::cout << blanks (in) << "}\n";
}
