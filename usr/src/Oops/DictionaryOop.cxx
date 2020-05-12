#include <iostream>

#include "Lowlevel/MVPrinting.hxx"
#include "Oops.hxx"

void DictionaryOop::insert (int hash, Oop key, Oop value)
{
    ArrayOop table;
    LinkOop link, nwLink, nextLink;
    Oop tablentry;

    /* first get the hash table */
    table = basicAt (1).asArrayOop ();

    if (table.size () < 3)
    {
        perror ("attempt to insert into too small name table");
    }
    else
    {
        hash = 3 * (hash % (table.size () / 3));
        assert (hash <= table.size () - 3);
        tablentry = table.basicAt (hash + 1);

        /* FIXME: I adapted this from the PDST C sources, and this doesn't
         * appear to handle anything other than symbols (because of
         * tablentry == key). Make it a template in the future or make it accept
         * a comparison callback like findPairByFun() in the future, maybe? */
        if (tablentry.isNil () || (tablentry == key))
        {
            table.basicatPut (hash + 1, key);
            table.basicatPut (hash + 2, value);
        }
        else
        {
            nwLink = LinkOop::newWith (key, value);
            link = table.basicAt (hash + 3).asLinkOop ();
            if (link.isNil ())
            {
                table.basicatPut (hash + 3, nwLink);
            }
            else
                while (1)
                    /* ptrEq (orefOf (link, 1), (objRef)key)) */
                    if (link.one () == key)
                    {
                        /* get rid of unwanted Link */
                        // isVolatilePut (nwLink, false);
                        link.setTwo (value);
                        break;
                    }
                    else if ((nextLink = link.nextLink ()).isNil ())
                    {
                        link.nextLink () = nwLink;
                        break;
                    }
                    else
                        link = nextLink;
        }
    }
}

void DictionaryOop::print (int in)
{
    ArrayOop table = basicAt (1).asArrayOop ();
    Oop key, value;
    LinkOop link;
    Oop * hp;
    int tablesize;

    std::cout << blanks (in) + "Dictionary {\n";

    /* now see if table is valid */
    if ((tablesize = table.size ()) < 3)
    {
        printf ("Table Size: %d\n", tablesize);
        perror ("system error lookup on null table");
    }
    for (int i = 1; i <= table.size (); i += 3)
    {
        hp = (Oop *)table.vonNeumannSpace () + (i - 1);
        key = *hp++;   /* table at: hash */
        value = *hp++; /* table at: hash + 1 */

        std::cout << blanks (in + 1) + "{ Key:\n";
        key.print (in + 2);
        std::cout << blanks (in + 1) + " Value:\n";
        value.print (in + 2);
        std::cout << blanks (in + 1) + "}\n";

        for (link = *(LinkOop *)hp; !link.isNil (); link = *(LinkOop *)hp)
        {
            hp = (Oop *)link.vonNeumannSpace ();
            key = *hp++;   /* link at: 1 */
            value = *hp++; /* link at: 2 */
            std::cout << blanks (in + 2) + "{ Key:\n";
            key.print (in + 2);
            std::cout << blanks (in + 2) + " Value:\n";
            value.print (in + 2);
            std::cout << blanks (in + 2) + "}\n";
        }
    }

    std::cout << blanks (in) + "}\n";
}
