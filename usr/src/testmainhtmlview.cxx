#include "LGPL/Controls/Fl_Html_View.H"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Shared_Image.H>

Fl_Window * window = 0;
Fl_Html_View * b = 0;

const char * test_filename ()
{
    static const char * s = __FILE__ "        ";
    static char * ret = 0;
    if (!ret)
    {
        ret = strdup (s);
        char * where = ret;
        char * slash = strrchr (ret, '/');
        if (slash)
            where = slash + 1;
#ifdef WIN32
        slash = strrchr (ret, '\\');
        if (slash && slash + 1 > where)
            where = slash + 1;
#endif // WIN32
        strcpy (where, "Fl_Html_View.html");
    }
    printf ("Ret: %s\n", ret);
    return ret;
}
char title[8];

const char * filename = 0;

void callback (Fl_Widget *, void *)
{
    /*
    delete[] test_string;
    //Fl_Xml_Object::destroy_object_list((Fl_Xml_Object*)html_list);
    read_file(filename, test_string, 1);
    Fl_Xml_Object * result = 0;
    Fl_Html_Parser * p = fl_html_parser();
    p->filename(filename);
    int error = fl_html_parser()->parse(test_string, &result);
    Fl_Html_Object * html_list = (Fl_Html_Object *) result;
    int  res = Fl_Html_Object::extract_title(html_list, title, (unsigned)
    sizeof(title)); b->html(html_list, true);
    */
    b->load (filename, 0);
    // b->initialize();
    // b->reformat();
    //  b->topline(0);
    //  b->leftline(0);
    b->redraw ();
}

void filechooser_callback (Fl_Widget *, void * data)
{

    const char * file = fl_file_chooser ("Choose HTML file", "*.html,*.htm", 0);
    if (file)
    {
        filename = file;
        b->load (filename);
    }
}

void format_callback (Fl_Widget * w, void * data)
{

    if (((Fl_Check_Button *)w)->value ())
        b->format_width (-1);
    else
        b->format_width (0);
    b->redraw ();
}

//  int get_local_filename_from_uri(char * result,  char * mark, const char *
//  directory, const char * url);

#define IMAGE                                                                  \
    "<img width=\"20\" "                                                       \
    "src=\"data:image/"                                                        \
    "png;base64,iVBORw0KGgoAAAANSUhEUgAAAPAAAADwCAYAAAA+"                      \
    "VemSAAAgAElEQVR4nOydd5gcxbmv30k7m7RZu1pJq7CrnLNQQokoggGBAWPMsbFNMBh8nH0P" \
    "wWCbi8Fcg42N0zkO2D7GgI2xTQ4CBAiEUCAqgsIqrrTaoE0z3feP2tb0dFenmZ7dlTS/"     \
    "55mnq6uqw8z0299XX1VXQ1ZZZZVVVllllVVWWWWVVVZZZZVVVln1cQV6+wSy8qwokA/"      \
    "kAv2AQgjni08wCoFcCOQAYSCE+I8VUOOgxkBtB7UD4u0QPwJKK9DS/"                   \
    "WkHjgCxXvlmWXlWFuC+"                                                      \
    "qUKgHIJVUDCooCA6sKgoMqioKFJVUhLun58fLCssDJb06xcqyMsL5OXmBqLRKNFIhEhODsFI" \
    "hEAoRCAQUAkEVBRFJR5XiMVUtbNTUTo6FKWjQ+"                                   \
    "lsa1M62tqU9pYWpa2pKd7c2hpvbG5WGg4diu9rbo7vbm+P10Os+8N+"                   \
    "oAHo6N2fJiu9sgD3vsqBwVBU179/4ajKyujIysrosMrK6KD+/"                        \
    "cMVFRXhovLyUKS8PERxcYDi4gCFhQHy8lSiUYhE1KOfQEBBgxYUQO3+"                  \
    "KKiqSKuqgqKoKIpCV5dKR4dKR4dCe7tCS4vC4cMKjY1xDh1S2L8/zr598ba9e+NN+/"       \
    "bF9+zdG9+5b198a2NjfCPENkF8G1CPsN5Z9YKyAPeswkA15I8qKyuZNHhwweTBg/"         \
    "PH1NTkDhk8OKdi0KCcyMCBYSoqgpSVBSgshNxclXBY7QZTQVEEjKoqIBRQJoDV0hqwRpDBDL" \
    "ge/"                                                                      \
    "GBQO46w3B0dCq2tKo2NCvv2xamvV9ixI87HH8fbtm+P79m+"                          \
    "Pb515874ey0t8Q0QXwfqFoSlzqoHlAU48+oPOWPLyipm1tYWz6qtLRxfV5c/"             \
    "pLY2t9+QITkMGhSmoiJAQQFEIgIqRVG6PzI4jVAmr+"                               \
    "vrWwGsWWLr8uQ8AbZCKKR2n59KW5vCoUMqu3bF2bZNYdOmOBs3Kge2bIlv2bYtvra5WVkF8b" \
    "eBzWQtdMaUBTgzGgZlM0aPLl8walTxSaNH9xs9enR+"                               \
    "8ciRudTUhCkrC5Cbq7d0ZlitALWH0g289tY5uY6srmbFFYJBlVBIWO6uLuF+"             \
    "79ypsnFjnHfeUXj3XWX3Bx/EN3z8sbIS1FdBXQscyNzPfuIpC7B/"                     \
    "GhaJVMwdN65y6fjxpXMmTCgeOW5cQXjkyBwGDAhRUCAu/"                            \
    "nhcDqsGirn96gxwAjIBlhO09gDLXHF5mRH2YFC4+8GgQmenyoEDCps3q6xbF2fNGqV5/"     \
    "Xr1nY0blZdUVX0BWE3W1U5bWYDTUwWUzJ0wofqMSZMqFk6dWjpq0qTC8MiRUfr3D5KTI1xhD" \
    "Vpny5oMsB40kZ+"                                                           \
    "au2xnSd3Dqy831jffIAIB4XKHw2L98GEB81tvKbzxhtqyZo2ybvNmngWeAtYCbT7/"        \
    "NyeEsgB7VwiYUlNTu2zq1AFnzJhRMW3GjOLcMWPyqKoKEA4LYONxI4R6QK0BtrO2gYAeHq9t" \
    "XWd49ZFq53pyeOXb691thcOHVT74QOH111VefZX9a9bw2t69/At4GvjIn7/"              \
    "pxFAWYPcqhbKFs2YNuWDmzAFL58wpHzh1aiE1NWFyc1VisTjxuNptfeRQCgDl1tZYP3k/"    \
    "1tbUTR23FtcZeGtYZVbZ6sYQDCpEIiJ/3z5YuxZefBFeeYV316/n36rK34E3gS6//"        \
    "rzjVVmAnVVTVlZz1uzZNRfNm1c9Z+7csrxx4/IoLQVVVYjFFKmVtW/"                   \
    "TurO4YntZu9PO6rqNNMtAdra4shuFzOLKbgqyuqGQSiQCHR2waRO89BI8+yz7V63imaYm/"   \
    "go8BzSn+ycer8oCbK2RNTV1F8ybN/TihQsHTp0zp5S6uhyiUYVYLK4LRMlcY7OVlddNbs/"   \
    "KBmB4TbsJUNkDaget+eZgZ3llFlgONgSDEIlAIAC7d8Orr8KTT3JkxQpW7NnDn4F/"        \
    "kw16mZQF2KxRw4ePumThwuGXLFkycOycOaXU1IQJBOLEYiIYlQAQEheqPm0FtHOb1xhFlgHp" \
    "po471znZuie3sd1s5wSv2ZU2u9YKAKr2EwKhkIC5sRFWrYJ//"                        \
    "Yuu557j1e3b+R3wONmuqKPKApzQ8OHDR31q0aK6T5966uAxc+"                        \
    "eWUl0dAuIGNxm0i84Mqp2rbARPXMz223uxwO7y9G6yzFq6tdRyEJ1htnKtNWlJVRUg5+"     \
    "RASwu8+SY89hixZ57hle3b+Q0C5MOu/"                                          \
    "tnjWFmAoXLgwNpLliwZ+bkzzhg6ecGCcqqrQyiKAFeD1spFtgPUi/"                    \
    "W1izDL+nbt2qXaUu+qy4Zb2sNrXdfKAlvBax3QsgdYW2oW+cgRYZEffZTOJ5/"            \
    "k+b17+"                                                                   \
    "QXwBCfwAxYnMsB5hYXV55166rirly2rPXnJkkpqasKoqtniGuF1A7PMndYuZHnkODMW1xu0d" \
    "hbULbxGq2129/XwJvYlZARYS2vt5GgUmptFsOuhh2h55hn+3tzMA8BKh//"               \
    "7uNQJCnDuvIULp11/"                                                        \
    "9tkjP3HGGYNyR47MPdrGTVxs6NLO4Jq7iJzbu1aQurG4Vt1HcgjdwevkUtvB68aNtnKhZe1g" \
    "mSXWFAwK17qhAZ58Ev73f6l/8UV+B/wK2Cb7x49XnWgADx47dsIXzzpr3OfPPXd49bRp/"    \
    "YhGFbq69MEpsAcWnINV1rAawdNGLaVqhY3W3Uvwymtdd21d+"                         \
    "3qym0ByfMEdyKoK4bBwrbdtg4cfhoceYs0HH3Af8BAnyMiuEwXgUEHBgPPOPnvKf15wwei5i" \
    "xdXUloaONodlIARwwCKBNBuQbV2j+2BNLdXjdB7gc3e0rpp/1pbSu/"                   \
    "5dpHo5HI5vLK0fj0SEeurV8Pvfkf7P/"                                          \
    "7Bo01N3AO8xXGuUG+fQA+odubMGd+96qr5t1x11YQR8+"                             \
    "aVkpOjdrdzxf0rEIBAIJEWCgCB7jLng9jVE/m6K677ZqH/"                           \
    "aPmJfam6MjVpO31dY1nytk7L5Dz7/"                                            \
    "Zrruck3rosbpLF+4nvpfw+7tH4ZjwuAhw6F+"                                     \
    "fMJDxnCxP37OWXXLgDe5zgOch3PAAcLC6svuOSSJfd96UuzzrvooqHRqqpQd1+"           \
    "uADYBrgZvQPeRA2kET15PTaprDWyirnF7KxiTt7EH0hpa843Bfr9qUn03+"               \
    "cb9JP8uKvrvaPzOyb+de5BjMcjNhSlTYMYMSoNBTtu6lVHt7WwBdkt+6GNexyvA1dOmTb/"   \
    "56qsX3Hb11ZOGz5hRTDCoEIslIBUyW2C99FnWQAeSttX2lVzXWC7ft5VFTpZqsw8r8JK3t1r" \
    "a78N9vjgfGeRGS2vch/m39gIyCEscj8OAATB/"                                    \
    "PsGaGsbt2cOS+nragHeBuORHPWZ1HALcb8GFFy6977rr5lx28cXDo/"                   \
    "37h7qDVBpUemubbIE16YG2g9vKehjBk7vDMgtmDZ2TpbUD03xOcsjs9imDNDnf+"          \
    "L3M1lm2L6OVTtcK661xJAKTJ8PUqZR3dnL6xo1Ux2Js4DgaAHI8ARwdOnTU1VddtfSeL31px" \
    "pT588sJhVRiMYAEvMlAJiDV1vWQo2sHH61hssSqZVnyxZ5cV+5Wm+"                    \
    "E2XuhWZcn7N7qsVhbamCc7PxXzDcjZ6hr3YV5Pvml5scL639euXLPGgwbBvHmES0qY8dFHzD" \
    "p0iO3AVo4DHS8ADzrppJO+d+"                                                 \
    "21C7995ZXjy4YNi3a3dRMAJtzaBJzJf7yx3HhhaK6y0Vqb28tmMO3y7dxks7tsFWyya+"     \
    "vK82TA6fON21hbbVm+PHhlhNR/"                                               \
    "KywDXGsbz54NI0ZQs3s3p2zfTiewjmPcpT4OAM6ZuXz5aT/"                          \
    "98pcXXHz++UNDhYXQ1aWSbD01S5sMr5Y2WmKzldXnmy+"                             \
    "yZJndZWtLbG9xrevLgkL6Y1tZWqt9u3OlUwteGSPoydukCm/"                         \
    "id3OXpyjCIo8eDdOn06+9nVPef58KReEtoJVjVMc0wMXFA5dfeeXpP/"                  \
    "3yl0+auWBBBaAQj4MANNliJtq0RnitL46jtSzu7ELWLrS+"                           \
    "jtGS2eVZwWsHmZe2sNv9yCyprPsncVx51Fm2r3RdaDflxqWqQlcXVFXB3LmEolFmffgh41pb" \
    "eQfYa/zXjgUdqwDn1NaOue7qq0+569prp9WMGVNAV5fo100G16rNqy/"                  \
    "T71Zfz6orydgmdtflZJUvd5ONZfaRYLdL94EuM8SJcuP5mi2qfl9y4P2zwl7yNMVikJ8PJ50" \
    "ElZWM3LSJuQ0NbOEYbBcfiwAXT5s249Zrr13yX5/"                                 \
    "73IR+"                                                                    \
    "VVVhOjsVAoGgDkxZhDkZ5mQrnAy7PZDmPt6EjBemuVwOlD7tri82laUV3NbbmSF0cwwnWI3r" \
    "qXQnec3TL0G41MGg6DOurWXAtm0s3rWLA8B6jiEdawBXn3zy/"                        \
    "Luvv37xtZdcMjJcUBCgq0sParL1NcJLUgAqUW4F7NGaUliN66pkP9ZdRt7awf4tzft3B7b83" \
    "OzbxE6wmsusrXDiN8Ky3KqeVR1VFSCPGgWTJlG0axdLtmyhEzHlbeIRqT6sYwngumXLTvnpD" \
    "Tcs/"                                                                     \
    "uQ55wzpnv1RgzeIEV69FTbCq7eyRnCTLwz5gAz9tub9GLtuZBYMF3VSa7u6c6OtI8oyqynra" \
    "rJ37xMeiDF4ZmWV07HCdmVOkKuqcKlramDGDHIbGlj03nvkAK9zDEyqd6wAPP6CC5b9/"     \
    "MYbF52+dOkAxAu6NFjl8BotsLaUDcrQZL6DG4Gzdo21pdlaW1ky6zrpuc/"               \
    "mCLPzNuZtk7uSnKPR9pHo5DruQHZnha3qubHG+nRXF/TvD7NnE25pYf6GDRSpKq/"         \
    "Rx8dRHwMAh6deeuk5v/"                                                      \
    "jKV05eMG9eBbFYvPshhABau1duhcEIceKP00OcsKp2Si43RlCt4JZB61THDtZ04HTap5u2sB" \
    "ls47kn1uWwJq9bgWwNoBd4ncqM+4/FoLgYZs8m0NHB7PXr6R+Ps5I+/GhiHwc4OvPyy8/"    \
    "+xY03Lpg1c2bZ0UizBq/RAify0OWBzBVOfmhBD7r5AjBaYyOMcrjtoZW5rM7QWtVxu/"      \
    "Qf7MRvJG8TW1lqeyucWWjtYI7FoKBADPro6mLaunVUxWKsRLz4vM+"                    \
    "pDwMcnfGZz5z1yxtvXDBt6tQSuro0yxs8CmrCAmvgGt1oMFpjM8zJSvypZhc5GTZ9vrXbmLw" \
    "0WthMWlQv+/QHbD3IYO16uwPZ3g2W5aUKr/"                                      \
    "F6iMchLw9mzYJYjCnr1lHd1cXL9EGI+yjAkamf/"                                  \
    "rSAd8qUYh28ehdZ6zaygtfoStu7yckusKwcjHBYWRCt3FjP2jVOdZkZVzm1bey8CzeRZ2Mdj" \
    "i5TdYm9WF4ZxNGogLizk0lr11IVi/ESfcyd7osAj7/00rN/"                          \
    "9ZWvnDwzYXmDJIDVD9YwWmIzvPJ+3WR3WZTJ6ZWB7a5LSFYPi/"                       \
    "p9AdrM1ZG52GAPslvrmoqldQM0CIhzc2HGDGhrY8rbb1MWj7OCPhTY6msA151//"          \
    "rIHvvKVk+eJNq/ebda7zwmra2wLJ/IhOeKcsMaJdpv+0Po7v9EVluXJ3WZ/"              \
    "LG1vusqZq2P+nexca3/gTac9DAmIZ86EpiamrV1LnqryMn2ki6kvAVx95plLf/"           \
    "aVryw6be7cCt2UN1ZR5kRb2Ayv3n1Glw4Y4NKU3LVj5S4bL0pZG09f7h94Xur2/"          \
    "Tp2FllfR+T5Y03d1LVyqeNxMfRy+nQCBw4wY/16VOAV+sBgj74CcMnJJ8+/"              \
    "54Yblly4dGk1imK0vAk32Qyp2Y0mqe2rudL6PyX535X92YkyM7xWlth6ab5huN/"          \
    "mWK9jfcNydq0z40p7WWqKx6FfP5gyheCOHcz68EMagTfoZfUFgKNTpky77frrl1x97rlDUNU" \
    "4ipKA0miBk+"                                                              \
    "E1utHGfJmrDPqLShZVtrOo7t1jv6yx33UyCXKqFjl55Fbi98e09NPSeoU4FoOyMhg/"       \
    "nsimTZz00Ud8hJimp9fU6wAPHz7qhmuvXfqdSy8dGQqH9Q/"                          \
    "hJ9xjc+"                                                                  \
    "RZtm5uA9tJ1sbVljILm373z7FQpyduBNZlqbSD3ZT5sdSkzbdVW0veunXM3LuXt4GP6SX1Ks" \
    "AlJdUXfvGLp9595ZUTCwoLA7pneZPdYr2bbIZXH9TS6mlH0LvTCWitLbKdu6xabmO97Msgua" \
    "mT7vl733dycKt3obWzxMOGQf/+lKxezaTDh1lBL736tBcBzpt1xRWn/"                  \
    "vy662YMHDAgrHuqKGhaGi2yzK2WjYnWKxlMfdrawnoLSvU0JG7q9DbAqZ+bLKDUG/"        \
    "AapeXH4zB2LITDVL/xBjUdHTxDL/QR9xbAg887b+kDN9wwd+rYsQXdz/"                 \
    "OaIdRb4mRI5eAmD6VMSA6vtdUV2xwP1rInzi1TNxfnNm9PQmxMi7nFYeJEaGxkzOrVhFWVF+" \
    "jhyHRvAJw3a9as//"                                                         \
    "vlL598wcKFFXR1KUlAJlvf5DwZzPIRWQk5tXPNLrObQFXWWmb+"                       \
    "uAl5tbhWAKYCv1UegKqKl6xNmADbtjFl40Z2AW+bvkAG1eMADxlSd+011yz++"            \
    "vLlw0KBQLx7vmY9qMnW15yvd5GtLLQcTm1pdJWTrW5fvaD76nFTPTf3AGtyA63TNk77cbuNJ" \
    "kWBkhKorSXy9ttM27uXVcAOyy/hs3oU4GCwaOHnPrf03i98YUJxYSG6B/"                \
    "LtIHVfJtatL6REQErLd4b62AHp+"                                              \
    "AcYnNuqVhbYuJ4OvMb9xuNiQoDCQopff5261laeAlpsv4hP6kmAB33iEwt/"              \
    "fv31s8bV1kaPTv1qnoRO7wY7wYuurv6HNV8cdtbYCmrzMt0L2a5O9saRXMdafgaxUq1jTCuK" \
    "mLK2qYlhb7xBSFV5jh5oD/cUwOHJk6d89/rr51908snl3S/"                          \
    "S1rvO4AwvhjKOpo+"                                                         \
    "mAsYLwRledxeecb0vWDA3dY6Fc7NaOqu34JWVqap4lcvYsbBpE5M2b2YjPTDII5jpAwCUlAw" \
    "4/8ILp1156qkDuodJWtVMFNjBZQWq7GOuq9+/"                                    \
    "+bju1u3PPeEluJH7CzZ1pXqMnt7Ou1KB1mlbt3WM9bu6oLoarruOgro6bgZGuf4iKaonLHDd" \
    "8uULf3bttVNqKitD0ncVaRe7+"                                                \
    "c0Jxnx9WaI82ZrK4E2GODkfzDcDJOtOywDi5wzq1rVBJrLzsNt/"                      \
    "ulbRTZ2esOpWcntu7tQXLLAmbZBHLEbla6/"                                      \
    "RLxbjSTL4+pZwpnbcrci0adO+"                                                \
    "dvHF4yfW1UXp7FQQ3T4q4gL3ssSQBtHEkE1UJwdabpGtXDe7i0hfJiLfLS2tvPPObt59dx97" \
    "9jSjKAqVlfmMHVvOpEkVlJTkAzESzSLZ/"                                        \
    "r1a+nTqpLudXV3xP+3cGWfdui42bYpx+LBCXp7K0KFBJk0KMHq0mJfZLwUCwo31sgR5md1+"  \
    "ZWVaWtufqsJll8Gbb3LJ3//Oc8CD/n3TZGUU4KKiqnPPP3/"                          \
    "y5YsW9ScWi2OEUoAn1u2hRrdN8jESoBqDWWYX2zpY5dbKGi/aEEeOtPHYY+/"             \
    "y4INreeONnRw4kDzrSnFxlGnTBnDJJWP45CdHUVKSR+JRUqv96uU31Jm+AQTYtKmLP/"      \
    "zhCI891s7GjXHa2xP7C4WgpibI0qVB/uM/"                                       \
    "wsyfn+KhJHILr9M2YL2dXX1NsRiUl8M115D77rt8a9MmXgc2+/dNE8qkCz3onHPm/"        \
    "eTaaycNGzAg1P1ybdBcYCt32egiJwNrfK0JGC9M64CVBqqCzDLLP/J9CYXZuHEPX//"       \
    "6v7njjhW8//"                                                              \
    "5+jhwxP+Pd0RHno48O88QTW3nnnQOMGlXCwIFFkmO4cU2Ny96uk3zOqqryl78c4frrD/"     \
    "PXv7azd6/S3WTS1VKhsVHl7bcVnngiTns7TJkSIBqVHde7/"                          \
    "HCj06mvKR6HoUOhtZXKlSsJKQpPk4GodKYADowePe6b11wz+"                         \
    "5IFC0qJxbShkvK2bfIb7uVpM8xaMEsvo5U15undVzdW2Bre1as/"                      \
    "5gtfeIQnntjYHVW3l6rCxo2HePnlnYwaVUxdXSmJG4rsGH0RWKs6EIup/"                \
    "PjHzXzjG41s3+6uydfSAi+"                                                   \
    "8oLB3r8q8eUHy89MHGPwLZLkB1aqeqoomwvDh8P77jN6yhbXAJk9fxIUyBHDBnM98Zt6dV1x" \
    "RV5iTo3S7F/"                                                              \
    "JAlRxYPez6MhW9W50oFxdT8rq2nZ1lTQXiCOvXb+"                                 \
    "fKKx9m9epdLn6LZB040MbKlfVMnlzOsGGlJOIbqcPT23Crqsp99zXzX/91mJYW7/"         \
    "CtXauybx8sXhwgNzc9eDWlGpQy1rHbp1U9TYoCpaVQUEDuypUMbGnhcXx+"               \
    "4CETAOcvWDD9h9dcM3XWqFG5xGIqZksrc5NlwOrXsVjXw5t8cbnvbrJyq43bhdiz5xBXX/"   \
    "0or776sftfxKBDhzpYv/4AixYNpKKiADnE5u/jful3XbuyAI880spXv3ooJXg1rVsn/sNFi/" \
    "wLbvkZYU7VQiuKiErv3MnQNWvYA6zy+"                                          \
    "j3s5DvAxcUDll955exvnHdedRi0V36CHFTr9q4zwKpkPfExAp6cTsUiB4jF4tx229P88Y9rH" \
    "X8HJ+3Zc4SDB9s588whRCJBvLnSsvPzo65Xqw4ffNDJ1Vc3sHNn+"                     \
    "j0la9fC2LEBxo5Ne1dH5cV9lpVZ1XMLvKqKSfEGDCDwxhvU7d/PU/"                    \
    "j47LDfAzn6L1o09vozzxyUG40qKIobt9QJIHN/biCgt5gKRgsq6hvzzfW8fcI899wH/PrXb/" \
    "rwMwk9/PBm/vrXjST+Bq/WViYvdY3bWK3Ly7q6FH70o8O8/74/"                       \
    "EzS2tMAdd6js8t4ysZUeNq/"                                                  \
    "WOB14NXV0wKRJcOGFjAyH+"                                                   \
    "SIguWWkJl8BHjhw2KeWLas7adSoXLq6BCyJScvMMFqlrUFTJPAagdWDKoNWkRzDCewgzc3N3" \
    "HvvSg4fbvft9+rsVPjJTzZQX9+E+a/wCrMMOisw/"                                 \
    "bkRvPBCGw891GqzD+9avRp++1tfdwl4BzVVV9qY1lvi5cth9mw+DcxK+wt1y0+"           \
    "Ahy5ePPrzp5xSFRSPCcqtr6rq8+SfZLATwMkgF+WKZFs7QO0Al3kBQZ566kOef36Lbz+"     \
    "WpjVr9vPII1tI3JTdWb/kdSfY7OBO5UYA7e0Kv/51M01N/o/X//"                      \
    "3vYetW33fr2QIb87xGpfXq6hIR6U9+kqqCAq4CIil9CYN8A3jYsBGfPuOMoRNqasLd3UbJs+" \
    "4bwTC+Q0cOoMxaWlvRBMwyeFN1oQO0tR3hj39cQ0eHoVPTB6kq/"                      \
    "PnPm2loaMEMsRcgrcpSsbpO6wHeeKOdZ57JzAwyGzfC3/6WkV0D6UWY3brNWp4e/"         \
    "FgMli2DefM4D/BlCItfANctXjzyipNPLkdV40etrFgarauAKFEms54yS2yGWp+XDK/"       \
    "s49ZdNtYLsnbtLl56aZtPP5VZa9Yc4OWXd5MYN00aS3Tr6Vhsa5hVVeGxx47Q2Ji5p+"      \
    "UeewwaG/"                                                                 \
    "3fbyrtYSeX262VjsXEjJbLl1NaVMQXgGi638cXgGtrR152yimDR1ZXh7qnyLEDEkMZyAAybq" \
    "uqRmvrxqpaBbHcWGNNCk89tZGDBzM3X1lHR5x//"                                  \
    "3sHiqJZ+HQhTsViu992z54Yzz6b2Rf1vf22aA9nQn50D9mV2+"                        \
    "2rqwtOOQXmz2cZPlhhPwCuXbCg9lPz5pWiKHHMUFqlzR+"                            \
    "9xRbphAWVWeBkq26E08rqyqC2Kg/"                                             \
    "Q3NzKSy9loEFm0Kuv7mX37hZSj0inC7ksT77NW291sHFjZl8N1NICK1Zkbv9+"            \
    "RZi91o3HobISPvEJiouK+CyQ4/"                                               \
    "nkdUob4Jqa2ouWLBk4euDAUPeQQjvrq5jK9K6v2SWWW9PkwJZbOJ3caNlNIMDGjft555296f" \
    "5Mjtq6tYn16w9C0gMcPb1Et24N82uvtSc9oJApvf46tPob5DYpnQizVV1ZWr9NVxcsXQpz5r" \
    "AMmJ3O+acL8MC5c4ddOnduKaoqt74arLL2sL3La4RcBq8MVrv+X69tYXjnnT00NGT+"       \
    "vc5tbXHWrm0gtQEdmVgiXW9vV3j77Z55u+aHH8L27Znbf7oRZq9pbT0eh6oqOOssSvPz+"    \
    "TRpDKhKC+D+/"                                                             \
    "QefvXDhwIla5FmcoD0Q7vqBZfDpoXPblrXbVmaljZ8YGzbs6e4Sy7w2bDhIPK5vB2cKRqd86" \
    "/3s3h1j8+"                                                                \
    "aeebPm3r0iIt1T8tttlllhTbGYsMIzZnAOMCHVc04H4KKZM4dcPG9eqaHf1w5QxRCMkn/"    \
    "k7rZiyLdq99oN5nAzwCMBTldXB1u29NwbM7Zta6GpSW/"                             \
    "drG+E3pZW23nf386dMfbuzdgEE0nq7IQt/"                                       \
    "ne9J8mvCLPbtKZYDAYPhlNPpToUYnmq558ywLm5ZQvnzaueU1sb0fX72llTZ7iNYCa3iZ2sq" \
    "Mxyu7HAVtsFOHy4jZ07D6f6E3nWnj1tHDjQjn07WJaXrmU27tfqOLBjR4zm5sx1Hxn10UeZP" \
    "4bfrrLbtjAIKzxuHBcANZ5PnNQBDk+ePOTCuXPL8qJRq9FViXX3AzXsrKKs/"             \
    "9cJWjfHsXahDx9uz2j3kVFNTV00NLTjbE2tytws9bIqswZ8z56YzaSE/"                 \
    "mvPHrpfepd5yUC2KvfDCnd1ialoFy1iLHC6p5PtVooAR8bNmjXglHHj8rqnypG7vNYWWDka1" \
    "EoOUCXK7QNXVi6zVzfa6mYhrtDGxiO0tHSm9hOloNbWGIcOaceT/"                     \
    "W5W+V4gTmWbxLKhoYdo6tahQ9Du3/"                                            \
    "BzS2XSbbaywtqrWZYsIVhdzQVAgdfzTgngIUNqzjrppPKBJSUB4nE3F5e1JTZHp63ASjWS7B" \
    "QEs7LWKkeOdGZk+"                                                          \
    "KSVuroUWlpiuvPBsPTTMnupm1hmYuyznVpbxdM8PaFMuc2ytKauLpg2DWbPZi4ww+"        \
    "s5pwJw6bRpA86eOrXQouso1Y+"                                                \
    "x7Wu0uMmW2LydnQW2g9c6oNXZGXc1XY5fUlVVd8PINMSpAd/ZqV/"                     \
    "PvDo6es6F1uQ3pHbHicehrAxOPpninBzO8nqungGORErmzJhRMSXRdaQHzWhhEyCZoTQDKre" \
    "ETuBZtWGtXGm31lxBUeI91oUEoKp0ezTaTcP4nezy3JSlX7enYYrHQelBo++"             \
    "3q2yV1tdVFJg7F8aO5QxgoJfz9QpwYMyYgWdOm1acnwheebWyVoAm6pjbx7IotF0Qy+"      \
    "sNQfSn0psAACAASURBVL7fUChIKNQjL68AxJ8aiWj/"                               \
    "rBW4uMhzU+"                                                               \
    "a1rlDEl4fg3CsSEVPR9rR6ygqDcKNra2H2bMbgcXy016tz0JQpFYvHjs09Grwyz7Vs1dZ1aq" \
    "/qXWiRToyHtrOa9sEy921g836j0SDhcM8BHAwGyM3Vv8kBi7RXi2zMky2d6oh0NOrx6kxT0S" \
    "iEM/36AYP8AtNpX/pgVl4ezJ1LpLiYM/"                                         \
    "AwMsvT1ZmfXzFnypSSUZWVQV3wKnHBmx8VNH+"                                    \
    "supTsH8SXweymHWws8wZxv35h8vN77uqJRkMUFYWx+"                               \
    "u1Sh9luH17qQElJz93QAIqK6J4zuueVqnvsZnujYjGYOhUmTGABMNTtOXr5NwKjR1ctnTSpM" \
    "BIOK67cZ/"                                                                \
    "uhkvJAk78BLCtr7QZiheLiKAUFPeczFhaGKC3NIfl3wiHtNc9NmXWdioqeBbisTEwK19NK1w" \
    "p73T4eFyOzpk9nODDH7Xl6+TcGTphQOn/"                                        \
    "EiCjxuNeRV3pLagxqmafFsWsfyy2w07HdRqOTtykpyaGyMt/DT5SeSkpy6N8/"            \
    "B3MQyyqdKthuyuR1Bg0K9qhLO2hQaq5rTyjd9rCsT3jGDEIlJSzBJZuuAc7JKZ05YULRyP79" \
    "g8Tjdq6ybHYMGYxWFttsceXjnjNhgfV14hQWRhg6tJ/"                              \
    "bnyhtDRqUR1lZRHLuVh9SKMdDmblOTU24R93o4cN77FAmpesqy/"                      \
    "ZnVy8eh4kTYcwY5gKD3Zyj63+itrZiwdix+TmRiFvLZwQzkZY/"                       \
    "9+"                                                                       \
    "sWMCvL6r8FDoVCjBpV6vYnSlsjRxZSUKC1gbH4Pm5glaXt8tyUieWgQUEGDuyZsHBBAYwc2S" \
    "OHSllu2snGMivFYsLjmDiRWmCam+O7Bbh0zJiSOSNH5qIo+"                          \
    "m4edw8u2AMlg0dW5jWAlaoFTj7upEkV3ROvZ16TJhUTCGiTvLuB0m3aq0U25iXK+"         \
    "vcPMnZsz/jQgwfDiBE9cihb+dkelu3DGI2eMoWc3Fx33Ukur8zo2FGjCsdWVSXcZw0IY/eQ/" \
    "UwcRtdbbh3ls3TIIJSBagV2KhDHmTChjEGDCt39TGmopCTClCnFku/"                   \
    "g1fq6LXebl1wWDgeYPr1nAnsTJsBAT8Ma/"                                       \
    "Fe67rETvEYpCowfD0OHchJQ7HR+rgCuqKiYMXp0QUl+vip97tfOkiYAN1tWq/"            \
    "mf7S2wnRW3AtqrG51oBw8d2o9p0/q7+ZnS0rhxRYwf3w/"                            \
    "xniQroLApc2uRveaZy+"                                                      \
    "bOzaG4OPNeyfz5PT9wxErpusdu62mvJR09mjHAKKf6bv6FUF1d0UkjRkSRPzmkwagH1i5wZT" \
    "fflZ0rbQW3GwssA9mdq56TE2Hp0sEY39XktxYv7k9JSVRyXki+"                       \
    "m1tY3VhaXOQl72PixBBTp2bWja6shJNPzughfFGqYFttF4+LNxqOH08ZMN3p+G4Arho+"     \
    "vHDi4MGJsc+"                                                              \
    "BgPMwSjtX2giU9QP9btu3dhbYyY12cqsVTjllEEOHZs6NLimJcOaZVd1rdvGBdD+"         \
    "kUI6prKgoyLJlaU2m6KgFC4Qr2RfkR8TZzXZafigEY8YQKCpiBti/"                    \
    "R8kFwHmja2vzh5WVBRynzZGt2wEnm9xdfuHapb1C7gSGEe4YI0aUsGzZEOefKkUtXlzBtGkl" \
    "JLvPVlAZ193AKNsmlfLE8uyzowwdmplodE4OXHhh743AspJbYL3sw6odPGIEDB3KJKDcbn+"  \
    "OAJeVlUyurc0tzM3F1urKn+s1P6XkbA1V3XZWLq+"                                 \
    "V22z1sWsDO7nvCsFgkMsuG0l5uf9DgnJzg1x++RDy8sIev5MbEGVlVvWdyvV5CmPGhFi+"    \
    "PDNWePZsOO20jOw6ZbkBT1aWCvTxuOhOqqujFqi1Oy8ngAM1NYWThgzJMbmz5rHLdpYvMVG7" \
    "caikeQI7O7is3GE3VtjOXbaqry27mDWrkk9+0va3TElnnFHFaadVIqyvm3a/Hx9SKE/"      \
    "OCwTgs5+NMmyYv8GsaBSuvloMoeyr8qPda1UWCAgLXFQEtbWUAePszsXp1y8ZPDh/"        \
    "7MCBYRTFqs1pBFNmka0soBdr6NS2tbO8VtZbdjxZvkI4HORLXxrLiBFFDj+Ze/"           \
    "Xvn8NXvlLXPd5a5j67gc3P+rK0dd6ECSGuuSbq61DHc86BT3zCv/"                     \
    "35LS+AeoVZy1NVEX0fMYJANMpEu/NxADhYU1OTO6yiIiAB2PyxeuOCO5Ct4LFLy0C1a/"     \
    "+6PbbsPLoYP76Mb31rItGoP22/"                                               \
    "L3+"                                                                      \
    "5lgULKgD9NDqpQGq3nk7aue4XvhDlzDP96euprYXvfEeMwDoWlEmYVRWGDYOqKsYBeVbb2gI" \
    "cCBTWDRqUU15QAIoijzxbjYc2A22E3RoW6+GVXl1nN260letsPC8FiHPZZSO4+"           \
    "mrH7jlHXXLJIK67rs4iOm91U7HKc1pP9YNjndLSAD/"                               \
    "4QZRx49JzpYuK4PbbxSN1fVXpusdu6mqKx8UglkGDGA5YDkSw/"                       \
    "dUrKgpGDR6cE4lErB8f1Ldpk/uAzdZRHpiSQSurZ96f+wvR6rzsgJXBECc3N8jNN0/"       \
    "h8stTbw+feWYVd945npISO9fZC1hO61ZlbtJO5QqTJ4e4994odXWpQVxQAN/"             \
    "9Llx6aUqb94oyDbOiQHk5DB5MNTbPB9v+4pWVeaOqq7XB9eagUwJMO7fWyjJYBcLsoLK+"    \
    "OVhDbudK28FrVS9GWVkO99wzg6uvHul5xo6LLhrIz38+"                             \
    "mSFD8vHuOqcKtdsyWRpDWlaucMopIX7xixymTPH2e1RVwQ9/CF/+ct99bNCodN1ju/"       \
    "3ox0UXFMDgwfTDJhJt15grmDCh6przzisbLtrA5keq9AdNPgk1aR1UQz01KV/"            \
    "UU01l7i0DknrGPC8XrFOZQn5+hKVLK6mqymXr1hYOHLCf+3TIkDy+/"                   \
    "vWR3HbbWAYMyMUeXifvIhW3OpMf8bvU1gZYuDBAayts3araTgcbDMLixXD33cLyHivwGpUpm" \
    "FVVROQ3biTw7LOsV1VelG1nNx6uorIyZ5AYwKHgMCBEO2x3vUBSnjYMUVXptuIBXV39dnZlW" \
    "hrT/t1LBrcx3y3EMfLyQnzpS6NYurSSRx/"                                       \
    "dwTPP7GHTphYOH+5CVaGoKMzw4fksWlTBhRcOZOrUkqPb+"                           \
    "gGMtzputnF7w7ROjx0b4IEHQlx0UYC//"                                         \
    "S3OqlUqu3fDkSNigEZ5OUyaBGefDeeeCxUVHFPSrKObMi91rTRoEBQXU3voENpjakmyAThU1" \
    "b9/"                                                                      \
    "pH9hIUlTqwoI9fX00BrB0ufp15OhTIZWv0SSRrJuJ9mvJANWn3aCWEsLKzdmTBHf+"        \
    "c44rruulu3bW9m/vwNVVSkvj1BTk0dZmTbgQevr1e/"                               \
    "HXyuYWn0329r9Dsl1o1FYtizAmWcGqa9X2LlTpblZTI0zYADU1PS9UVapyA9A7aSqYkx4RQU" \
    "1hw5RADQb69gAnD+woiLcLzECK2nXQLDb7Q3YQC1bl6Wd4DWCi6GeG8m+"                \
    "g13aLcQqwqJCUVGECRNKSb6g9W1x4/4UQ9oJIq/"                                  \
    "R6p7+6H8bEeMYNEhYkeNdTsCmAns8LjyW8nKqNm2iFC8AFxREB5WXhyLhsEJXlwaR/"       \
    "mN2qzWgtfxEW9YrvLI6YL4pIMmXlduVpQuvzAoZ1zMBSCb3nU7aqvzElRfXWl8ummFQUUE5o" \
    "ivJ9LpzS4CLiiIDy8tDBIPJf4Le0mptWqMLbc4X6WRX2Q5eO+"                        \
    "jBDKzbi8QJXH06XYh7Ajqn4xjznNb9ABpJ/"                                      \
    "vGnTLnP+m1VFfLzobycfkClrL4dwAOKi4WlNbrI+"                                 \
    "miytfWTA5h4rtbc9k2so9vG6Cp7dZ1l5yZbd4LYDdCZANmNm+"                        \
    "02v6cj1cenvFrUVLbV8nNyoKyMKB4BjhYXh/"                                     \
    "oXFcki0CqqqllTmSVMDlzpgU20l2XQunWfjTC7kezXlQGrT9stU4U4XZj9AMrLulWZl9/"    \
    "h+FY6bV8nqap4NrisjABQJatjBXBeYWGorLAQ3QgsmZsMCQjlEWdx8gmQ9fCm7z6n8ssYt/"  \
    "EKr7Z0C4AsT1bHaGndbOcV2HSgTeXYJ57SscYyBYNiho5gkArZS94sAc7PD5bk5SGJQFsruZ" \
    "/3aC7uA1f6JYa0bF3Lsz0rF3leIfZihfyAIdVPbw74yMqoVKx1IAD9+kF+"               \
    "PmUtLeZtrADu169fsCA3F5MLbRW46i5FZoXlaS/w+mWF3YCrT6cKsRWwsrze/"            \
    "rg5b6fva0yfmPKj79coVRUA9+tHaUsLYbQ+y25ZAVyQnx/"                           \
    "MjUSMF3UCWnMgy7rta2wHOy+xSWvrmlKxwMb8dODV53kBwiq/p4FNB3C79PEvP9u/"        \
    "VnW1SHReHoVAFHcAhwtycwO5kYh6tA2bCEDJD5Lc52sFbKDbJQ84tIWxSaPLQ5JvJVm93oZY" \
    "X+6m3ev1CSw3+"                                                            \
    "V5daKf6WfkpDeCcHAqBHKBVX24FcH40SlRYYA2spN0is7rJrnZyhFkPuHMgy7g/"          \
    "PcgY8lKRcVuv8GpLu3QqIOthlqW9gOb243W/"                                     \
    "dvVPHKVrfd1aZ1UVQ1CjUQoA08wJFgAHozk5RMRjgokdmUdW6ddlEehU3Gc9rEZwZSB7kRtw" \
    "9elMQJwqOH6BKct3Ome7dWM6K78ViUBODlHcAxzIjUQIivmfMUSXZX3CiXX7Nmxi6e0BBv3F" \
    "IXOnvag3IO5JkNMd7GFX1678xFam2sNaX3A0SgThQifJCuBoTg6BxATuVtFmpwi0Pu12abcv" \
    "bf3oecpP3yTZL+cG5HTh1aedIM7UU0qp3ECczkP2vU4s+"                            \
    "R1ttlMoBJEIYdwDTCgYJKANo0yWAEoeuHJKy2D00gZGsr1X2YGrX88UxE6AgHeY3baTvezTD" \
    "bQnLsA9qWAQQiECSGbQsQI4mJjuRpMeyO4cVd5d5K7ti0W+"                          \
    "aihDtw7WF4ws0GYn43ezS6cLsZt1NwDJglteIXeb71RPfy5ZZUqqKgAOBAgimUHHCuBA8iyS" \
    "VuA6ubx28FqVI0lD8sUic53dXEyyOn7Aqy29QuyHZeypj+wcs+"                       \
    "opBS2mGrMAWFXFCCzj4EsjzFbr+"                                              \
    "rRYaq649TPCdjcBMIOvl1Vb2O4iM5bZwaulU4HYC8jHCsxZeHtSqgqKIv/"               \
    "hrSxwPB7XAlhHd4OqClCsupMS5UYo9XX17WIk9TCkZetaXuLYzpLV8QJxKvBapd2A6lRu9fE" \
    "6y4eX/"                                                                   \
    "WYB7mlpEelugN3OiaXGYjFVBTWQgMsp4uw1cAXJfcXGfWKzrs9LRW7A1af9gtgKTFmerI6X4" \
    "Jbbem4/xu+UVU9JUSAeR8EwjBKsAe7o7FQUVVV0jWY9nGZXVx64StqnTZ7Vw/xW6/"        \
    "o8t5JdeG5ANi61tBO8+jw/"                                                   \
    "QdbDLEv7Ca3VsbPKxIMLVorFoLOTLqDTWGYBsNLe0aEoiqKGgkGODuYAWddRYl3/"         \
    "pFIy0Ga3WdRNJ4CV6q/"                                                      \
    "nBLIbiO2WPQVxOp9U553OykpOMHuB3ThjRzwOHR10AV3GunYAdyqKGgkG9TNwQAI4JOt6GL1" \
    "Z4NQCWG6ssN0vZwWuft24NOY5QZwqyD0Fs9tPVr2hQAA6O6Gzk3bcAxw/"                \
    "0tamdHR2KgXhozU0sMxt38QTRrK2r90TSIn9CIsN7i2wMd+"                          \
    "tZNtYgWwFrXHpB8ROwELmR2s5nVtWmtK1uF4sclsbdHTQggcXurWtTWnv7FTIz3cKRMnSdnn" \
    "6JS7T2jr4dzEZ9+MWXGOeW7DdQiur4wSzXdoLuFb1s+"                              \
    "otBQLirRYdHbTiHmBaWlqUto4OVWc1NWkWU7R3nbuOnOFNPBBhBB/"                    \
    "Juj7Pq2QXoxuQ7fKsIPYCsnHdC8zp1He7z6xkylS7V69AAFpb4cgRmgDT26YsAW5qije3tyu" \
    "Stq8VnE5t38TSDKzM6nrtQpId105uwNWnnSDWlqlC7CfIWXh7Q5mISgcC0NwMLS0cwsO7kdp" \
    "aW+ONLS0KwWCQZDATAGqRZqP11PftCgvtNOezDFb9ryEbyKHfDxblTmV+Q+"              \
    "wVaDfrmYY56zb3lFKx2KoKTU3Q3k6DbBtLgJublYbDh/"                             \
    "UWGOSQJgNr7zYbp+"                                                         \
    "XRl0EyyDI4vVpZK8m2SxdebekGYqcyt6C6qZO1vD0pP9xmvRQFDh0CVWW/rNwK4NihQ/"     \
    "F9jY1xAgG9BTbP82wGVZYnW2KzHYY6GPKM+V6lXagyy+43vLIyt+"                     \
    "teQQVvc2vJ6mXlRX62g40KBMQgjoMHUYC9sjqWr1Zpbo7vOXRI6Z5WNnEiyUEtzZ1OwJDoM5" \
    "a5z8bBHMmudWJfR78C5osqlWh0gI0b2/nNbxqoq8vhM58pIzdX/7pVJ+trlU7VAlul/"      \
    "YD4xLW89fXwwAPihWBXXw2FhZk5jhHKdKyu1bbaY4Tt7XDwIEfAmwWmvT2+a9++"          \
    "OIqiVREAJgPrPnBlHp1lB2+6Aazk81BVhfvu28f99x+"                              \
    "gpCTE1Km5zJxZiPxpK7t0piD2AnKmYD729dBDcPvtwnINHQoXXZS5Y9lBm47VNY7Cam2F/"   \
    "ftpwsICWzxlCBDbtX9/"                                                      \
    "vK2jQzxWqKra00n6oEdyWl+uqoqhjnGpryPbr2zd7qF02UeU79nTyYsvimnt8/"           \
    "OD5Obi4njGtF2eUx2r8lS/s9PvceLBC+JFYNoY5aeeSg8kP+XVYmvlwSA0NkJDA/"         \
    "tBHsSyA3j33r3xptZWrS/"                                                    \
    "YDlj5hSyHWLsh6OuYy60vVCtYrT7w+"                                           \
    "uutbN4sutCmT89j5MgcIO5wLKvv5QSvlxuBbN3qBmVXlg7Mx48WLUq8THzlSthueptu5pROW" \
    "9iqLBiEhgZoaGAP0CitY3PMvfv2iXZwMGhlodxbJbm11crcXNipXaixWIy//"             \
    "a0JMSgFTjstn9zcgMW5yyBNxQK72c7pmFbf3wl0p2Pr08eXRoyAuXNFetMmePrpzB4vU260p" \
    "kAA9uyBAwf4GGiT1bED+ODevfGd+/"                                            \
    "fHCYWsLrREntF9tnOnk4FN3qcZdLuL2dn6rl7dypNPCvd52LAIp59eIKlnd5GnAq+"        \
    "bG52Xm1YqNzKrm4ki/"                                                       \
    "bOPB+XkwHnniXmU43H405+EC+qnvECbLuCqCrt2QWsrW63q2AHcuX9/"                  \
    "bOvu3fKL0Ko97NT2lbvMbqyQDFAn6xvn178+"                                     \
    "xP794jnoCy7ox4gREYT7bGe13KQzCbET2Hb7dvoc3zr1VJg5U6RXroTHHsvs8fyCVvZWws5O" \
    "2LWLGKQGMI2N8Y3bt8dRFPHni0CWBm/"                                          \
    "ik9yeTYbY3toa4ZddwHbA2lvfp55q5uGHmwEYMiTMFVf0627P290gnKxuuhA7pb2A7XTcEwt" \
    "egIoK+"                                                                   \
    "PznIRyGri647z7YscPfY2QKWu25e60LqakJduzgEKkCrKrxjdu3x9va2lTEa1bMF5wdvMalE" \
    "Vj9xZZsme0uajcWGHbs6OB732vg8GEB81VXFTNxoha8sgPDyep6gdeN1U3FGnuBWUufOLrgA" \
    "jj9dJFeswZ++EMBs5/"                                                       \
    "yE1qZgkHYvx927WIXsNNqf7YAQ3zb9u1aIMv6grOC1zoKrQfW7MJaR6adgBblLS0xbrmlgdd" \
    "fbwdg4cI8Pv/5fpb1rdet0l4tsVegnayxU76+7MRTcTF885tQVSXW//u/"                \
    "xSddZRpavUIh2LkTdu1iMxZdSOAIMPXbt8e31NdrgawEfJorbWVxrWE2W16zZfZihZOB6uyM" \
    "84MfHOT3v28CoLo6xG23lVBZGfSwfzfWMVUL7BZcN+fqZJFPXC1YAF/7mgDhyBG46SZ49FF/" \
    "j5HJgJaqwtat0NDAu0gms9PkBHDrzp3x97ZtUwgEEsBqwNm3da1gtgqAGS2z24s0cbG2tyt8" \
    "//"                                                                       \
    "uHuOeew8TjkJcX4Kabijn55FzDOclgsbK6TtbUT4jdWmMnmE9seDVdfTVcfrlI798PN96YPs" \
    "SZhhZEAKujA7ZsIRaPs8HufCyHUmpqbo6v27w5rnZ1BQICsORHCcXFInve15wvTtJYBslPOI" \
    "G3MdBiSOeBA11873uHuf/+ZmIxCIcDfP3rRXz+84WIdq9Rsl/"                        \
    "XmKdK0nZ5bpappGXrdvWyAjEW+vvfF/D+618imPWlL8HBg/"                          \
    "DZzwrrnIr08OknfDSW2W1nVaaqIgC3bx9s2cJe4AO7c3HxFdRQVVX4goULg/"             \
    "m5ueIA4lWjiSVYr4M+P3lbrZ528Yl8Y9nR88DqQn3zzXa++tVGfv/7VhQF8vMDfP3r/"      \
    "fj2t4uIRp23N+fZAYUhnUmIvYKclVH9+sGcOcId3bQJWlrg+"                         \
    "edFhHf8eFHuRqla11SAjkTgvffgD39g9aFDPIBkKh1NTi40oG7ZsiW+"                  \
    "ub5eIRw2u4HyQRrOrqTZDU+4gMn7kLmLACq7dsW4887DXHxxA48/"                     \
    "nhioUlAQYPnyXPLyAhbbW+3brh1q5wqnGtByStut6/OystPw4SIqrRmE1la46y64+GL4y1/"  \
    "Euhv5MfLKzY0gEBA3m507WQu02O3P0YUGGrdti6/ZuFE5adIk7Zlgufuc/JB/"            \
    "8qOE5qeNEumE6yzWEy8Tl2vTpi4ef7yd//3fNt5809w/cOCAwp//fIQpU/"               \
    "RPUllJZo2t1v2yxOmms+B60cGD4kml7idjj+qVV+Ctt+CUU+DTn4aFCxORa6O0yzEV6+"     \
    "rWzdYmu+jogPfeI9bRwZvWRxByA7B6+"                                          \
    "LCy6p134lefe24oqIdVg838qCDo4dbDqwc7MTkAQMDQJtHqw+"                        \
    "HDKjt3KqxbF2PFik5efLGDTZsS7dp+/"                                          \
    "US7vKkpsc2DD7Zx8cVRpk3LwXyxy35BN+"                                        \
    "BapVOFV5Yng1W2npVbPfSQGJWlV0WFmOmirQ0ef1w8vTRlCixdCvPnw7hxUFkJ+"          \
    "fmJbbQmnqIkIPSjPazfLhQS45/fe496YK3Td3MDMBB/"                              \
    "+7334nsaGgIDKyoCxOPJlld3KiSmzZEFuDDkcTQ/GFQ5cEDlb3/"                      \
    "rZMcOhVhMpaVF5O3YEeejjxTq6+NJd9H8/"                                       \
    "ACLF0e45pp83nqri1tuSfhC9fVxfvrTIzzwQIgc03vN9UrFAuvT6SxTgTgrL/roI7j/"      \
    "fjE2WlNZGfzylyJQ9MtfisEenZ3wxhvik5cHQ4aIZ4qrq6G8HHJzRdt0xgwxXDMclkNsJ7f9" \
    "v1u2wMaNbAA+cqrvEmA2v/++sm7LFmXggAEB4nFZ1Blk7nXyBHckpRMWHILBAA8/"         \
    "3M411zg3SIYNC7JgQYTly6MsWZJDv34hxo0L8uCD7UmW+eGH2zn//"                    \
    "BzOOSeKaDNaycki9xbEWXDTkaqKGTreeSc5/7TTxEMPgQCcdRb84x/CCr/"               \
    "5pnh8r60NPvxQfIyqqhIR7enTk11yK8tr1+"                                      \
    "a1Anr9eqiv51Wg3ek7ugW4bccO5ZV165Qz580Tc2RprrCAUDsha3it571KWPDCQhGAam0V3y" \
    "wUEutlZQGGDg0yblyYWbPEZ/"                                                 \
    "ToEKGQtm2M4cODfOpTUb773SNH99fcrPLDH7Yya1aIqiptbi+9vICrX/"                 \
    "cLYjfprFLVihXwm98k5xUVwZVXJtq0gwfDtdfCZz4D69bBqlWwejVs3iyeBGpsFEBrsJWWQj" \
    "Rqdpft5ASxcfzzunU0KwoGp18utwATi6kr165Vmg4fDhTl5xsDAkZXGoPl1dq9oKqJoJVoB6" \
    "vd+w9w3nlhysry2bNHJRyGwsIA5eVBqquDDBgQoKgoaDhm8i/"                        \
    "zqU9FePDBIFu2JE7ulVdiPPBAG7fckqfbTiavrrM+"                                \
    "bQV2OhBnlY4OHYI77oADB5LzTz9djNIyqrAQ5s0TH0URga/"                          \
    "6eti7V+zrSLddGD8exoxJuOR2ltdte1hTOAzbtsE77/Ae2A/"                         \
    "gOLqNm0rdp7Bu3Tplw5YtgXlTpsjawarOjTa3gTWXObleIq0oKnl5AZYt005JFoW2c4Nh1Kg" \
    "gl1+ew623Jnse99/"                                                         \
    "fzoIFIZYsiWAPiZ8QpwpvVn7ogQfg2WeT84qL4YtfRDc2QK5gUAS5Kirk5bGYO/"          \
    "fZKKd6wSCsXQubN7MCOGh/"                                                   \
    "lt3buKnUrcbNm9UX16xRup9MSgyrTCz1H5GnKPI+X3NazIAZi4lPPB5HUZTu7fV9n/"       \
    "ZDCa+4IsLYsclfa/9+ldtua2Pv3tjResbt/"                                      \
    "O0PdiqX7SMrv7RiBdx7r7nb6BOfEF1FbqWqYh/"                                   \
    "dL9gmFhMfqwi0U1q2f+0TCIhBJm++SVtHBy+4PUcvANPZqT6/"                        \
    "apXS0tSkdk+"                                                              \
    "zY74o9WOZzWOlZRevcYCHGezk7WXgJT7DhsEXvxghaPhmK1bEueuuDuJx44AJN4MmnNJ2kDv" \
    "BnJWfqq+Hm28Wrq9elZVwzTUikuxGqQCZzvaRiIg+r1nDu8Bb7o7iEWBgzVtvqWs//"       \
    "FAhHDZfxNbw6kFMrOun3jE+"                                                  \
    "lZS8LzmsVlb5ssvCzJ1rHiX6i1908dBDnZhhk8HpdfSU0zkawc7Kb3V1wf/9v/"           \
    "DSS+ayK66A2bO97c9N+"                                                      \
    "9Yq7bS9UcGgiIJv3MhzWMwBLZNXgBu3bOHp119PWJbkxwq9WF5ri2xthWWWzWzp+"         \
    "vdXufHGcFInPEBLi8rNN3eyenWMZLBk1tfLObtxp7PQZlq//S38+tfm/"                 \
    "HHjxJNJNoP7kpSu9XW7L330ubERXn2V5s5OPE3F5xVgFIWnVq7kwP79GCa7c3IZndvACWsrc" \
    "6et3WbZ8c86K8BFF5mt8ObNKt/"                                               \
    "6Vhf19TJLaudKy76XG2iz4PaEXngBbr1VdPnoFYmIxwhra73tz6slTQf0SATefRdWr2Y1sNr" \
    "Ltp4BBta/9RYr165Nbk/"                                                     \
    "og1ea5U12lY0utcxVlrV5zS649Q0j8cnNVfnqV0PU1ppvu889p3DzzV20tlq5vm5caau8LLQ" \
    "9rQ8+gK9/XbR/jTr7bLj0Uvf78gtcJzdbX1dR4OWXYds2/"                           \
    "gU0uT9bV48TmhRrbSWvspKzFywgqM2En3gUMOGqJNLmRwiT01od42OHxjrJ+"             \
    "3T6VFWJvrWnn1ZNEcn161UiEZUFCzAEvIwA6teN6ax6W3v3wnXXicizUYMHw09+"          \
    "Ip5G8qKesr6qKq7P+nr42c/"                                                  \
    "Y+fHH3IqH9i+kBjDAgY4Ozpg3j8rKSnEHsYZXllYNaTuQ9XXUpG3daNw42LhRPF+pl6rCm2+" \
    "qlJerzJyp9VlbgQtZYPuemprgG98QDysYFQoJl3r5cvf7flyYUwAAGLRJREFU89P6OqU15eT" \
    "Ak0/Cb3/LI52d/"                                                           \
    "BbhyrlWKi40wM733uOfK1YkrJfMNbBPm9u2Mnc6OWqdiEhb9w0nfwoLVW66CUaONH+"       \
    "J1lb4r/9S+fOfrdzmrEvcV9XRAd/9LvzP/8jLzz9fTC/"                             \
    "rVk5RZb9day141dQEzz1HW3Mzj2Az95WVUrXAKApNgQDnLVxIQWGh2Y2GTKRVqTW2l3Cly8r" \
    "EI2PG6UXb2uC118RrOcaMSe23yKpn1dkJP/gB/"                                   \
    "OhHyU8ZaRo7Fn72M+FCe5GT++u39Y1G4fXX4ec/"                                  \
    "Z2VTEz8EOjydMGkADOxraGDiuHFMmjhRjFDRK7Mgm5dOGjdOhOpfe81c1twsnhcdORJGjXK3" \
    "v6x6R11dcOedAmDZXM9FRfDjH4sXnbmVDK5MudD6kVeKAv/"                          \
    "93yjPPMOd4O7hBaPSAVjp6KAzJ4fzFi4kkpPjnxXW5JT2AnEwCNOmibbwpk3m8qYmMUPD8OF" \
    "ZS9xX1dEhwL3jDmGFjQoG4dvfhquu8t7n68aF9sOd1hSNiq6j++7j3f37uQWP0WdN6QAMUH/" \
    "wICdNnsyI0aOFFTaC5oeF9QK9nfLzYfJkEbLft89c3tQkyqqrYeJEd/"                  \
    "vMqmfU3CzavHfdJYcXxLQ43/ue88MKVvICYDrdSiBuNr/7HTz6KD8B/"                  \
    "pnaGacPcGdLC2peHucsXEhIm6XA77awVZ6x3I0qK0V79/"                            \
    "nnxUVhVHMzvPiieLxs+nRjF1NWvaH9++Fb34Kf/lTe5gVYvFh0GfXv736/"               \
    "XqyrLC8V66uqIvK8eTP8+Mdsra/"                                              \
    "nO9i8ecFJ6QIMsKuhgXlTpjB0xIjU28L6+"                                       \
    "sa0F8vsRnV1AuTnnxdumVFtbQLizk7xpjv7KXmyyqQ2bYIbboA//"                     \
    "tE6yDRpkpgaJ5X4hQw2q7x03Gn9NqEQ/OEP8Je/"                                  \
    "8AtVRdIJ5l5+ANzW1ISam8vZCxcS1CxWqlY4lXJNXiCeOFHMffTSS+"                   \
    "abDogAycsviwnGpk8XwZGselYrV4rZMp55xrpOXR38/"                              \
    "OepPajgBKwxz5h2Krfq992yBe65h4937uSbeBy4YZQfAAPsOHCA+"                     \
    "ZMnCyusuTmZcJu9AG2nQEBMUAYieGUcqaXp7bfFVCujR3vvlsgqNSmKsFA33AAbbOalGDRIT" \
    "Fh32mne9u/F6sq2sduP075CIdH27ba+f/"                                        \
    "Z25mb5BXBbUxNdoRBnL1pEKBJJnHC6EWZjnl0E2ivEwaC4c8dionvJCuJt24RLXVIiplTJto" \
    "szp/37xetQbr1VHmjUVF0tHto/7zxv+7dzg924zrI8t651To4Yt/"                     \
    "2jH7G1vp5vAIYJf7zLL4ABPtq7l1ljx1I3dqxzW1iW57c1dqNwWMyDFI/"                \
    "bQ3zokHDlGhqE+511qf3XqlXiyaHf/"                                           \
    "tY60gwC3vvugwsvTO04dtbUT3fZ2O8bCIi2+qOP8mPgkZRO3iA/Ae5obaW5q4tzFi0iJy/"   \
    "PXUTaKs+p3K01diMNYkURF5FVpLOrS4ycefNNcRGNGOH+GFlZq7VVXNhf+"               \
    "5r4be00aFDq8HqxurK8VCDXlJsrZrv80Y94p6GBbwKHvX8Ds/"                        \
    "wEGODj+nrGDB3KxGnT5FbYmE41iCXbj1V9NwqHxYz8OTkCUrs3um/"                    \
    "fLoZlNjWJyGfWGqeuN98UXUT/7/"                                              \
    "+JmSDtVFcnupK8us1gD6pbsJ3KZe60qoomV2cn3Hsv8Wee4XbAMN1e6vIb4FhnJ/"         \
    "WHD3POvHkUlJUlP6kE/"                                                      \
    "lhju22t6rtRKARz54q5f1etMj8crteRIyJKvWqVaBuPGJH66ypPRO3bJwJQ3/"            \
    "iGiDZbNV00TZ4sxjd7DVhBau1eq/"                                             \
    "pO2xrTIAaWPPMM3HcfL7W0cDNgc2V5UyYuuV319ZQXFzN/"                           \
    "3rzkH8GPdq4VtE5lbhUMwqxZMGyYcHkaG+3r79gB//636BoYNEh8srJWezs89pgA99e/"     \
    "hsMuHMmlS0VX0UkneT+"                                                      \
    "eDDYvZVb13ECsqsKzO3AA7riDlrff5mvAOs9fwkYZsRmqyuY9e1g6ZQpVw4eb3dF0Isyy/"   \
    "bgp8wIxiGjz1Kkiarhzp33dzk7R1fTkkyKKOniw9ZzCJ6oURfS533KLeBhh40bnbYJBuPxyE" \
    "W0ePdr7Mb24ym6izfqlXT39eiQiusT+"                                          \
    "5394MBbjXuRvm09ZmXL6Gg8epK2zk2ULFxJKvBjcm+"                               \
    "ubipX1yxKDsMILFwoo333Xub72QMSzzwrLor0Y60RWPC5iCt//"                       \
    "Ptx2m2hy2EWYNZWUiAcTbrtNjJrzKq+ustX2Vvu021ZTNCrec/SDH7B11y5uAHa7/"        \
    "gIulclW28YdOxg7cCDjp00zR3b9iDB7Bdu4HzcqLxdvo8vPF1bWrl2s6cABMUzz2WdFW6+"   \
    "83Pq9s8er2trENDd33CEeMHjppcTrSZw0fjzcc4+"                                 \
    "YxzmVBxPsrKybum4DWrJ96ANXHR1w113En3qK24HHvX8TZ2US4K6ODrbu2cMZM2dSPHCgHGK" \
    "/QDXu183SrXJzhSUeM0YMQpdNnibTgQPiIn7iCTGmNydHgHw8j63evVu87e/"             \
    "228WTQ6+9JrqJ3CgUgk9+"                                                    \
    "UnQTeXmeVy8neJ3caLv9ONXX14lG4ZFH4P77ebKtjZtx8abBVJTpuOnuPXsgHufUk08mqI3Q" \
    "0oMIvWOBvUIMAuBTThFt+vfft+9q0uvwYdFd8thjwoU8fBj69ROzhKRyHn1NR46IgN+"      \
    "vfiXAfeAB8dx1u4dLdsgQ8UaFW2+"                                             \
    "FmprUzsMNvG62SRVsTdGoeDXp7beze8sWrgckT6D7o57o+Hjvo4+"                     \
    "YUF3N6KlTk62w3xHmnoC4tFR0ZYwcCR9/"                                        \
    "7N4ag3CpNm8WUesnnxTjrFtaxKOLxcXHFsxtbeK9u3/5iwhK/"                        \
    "ehHwtOor3fuEtIrHIYLLhD9wBdemP6zvF6WqQAuK9PvT+c6K//8Jz8A/"                 \
    "jelL+RSPXXJTJ80iYd/"                                                      \
    "+lOGTZmSeIRPFtTyI0iVyTaxXtu2CWvzu9+Z38XjVsGgGKAwa5YYSDJrluhT7ouDQ/"       \
    "bsEVH5114Twbq1a73dwIyaNAm+9CX41KfETSxV+QFtOnW0TyAgbkB/"                   \
    "+AN85zv8s7GRywGHjsj01JP3/Ksuuoj77r6bnKKixOwdflngdNzodC3fihViUMI//"        \
    "+kuyGWnqirhqk+"                                                           \
    "fLgYvjB0LQ4eKbqmefIiio0ME4LZsEU8EaU9lbd4sou3paOBAuOwy8arPdIejpmJ5tWW6dYz" \
    "p3FxYswauv56P169nOR5eUpaqehLgvNxc7v/Wt/jsjTcmXtPoBuKegDddiI8cgX/"         \
    "9S4zpffFF+TPGXhUICHCHDhVWetQo8YoQDejycmGpc3K8B8ZUVUDa3i7a5AcPigDUxx8LSD/" \
    "8UHgYO3emD6ym0tLEdK9z5qS/v3Tg1Zap1jGmIxHxwMt//iftf/"                      \
    "sb1wOStzT5r55udQ0fMoSH7rqLGcuWJYIcMlfauPQCorFdnEp7OVUdOiSCVb//"           \
    "vRhq6QfIRhUUiH7SsjLxKS0Vy6Ii0d2Vny+"                                      \
    "ADoXE91EUcR4dHeJG09qagPbgQXHOBw8KUL0EntyqtBSWLYP/"                        \
    "+A8RzXf7ik8reY0iy9qqbvbjxgJr7d5AAO6+G+6+"                                 \
    "m190dnIDKUwRm4p6I2xy6uzZPHjffVSOHSsuGD3AfllXJ2jdwJyOGhqES/"               \
    "2nP4n2ots+0ONJVVVw5pnCXT75ZH+"                                            \
    "6z9x05cjyrcrTscDaem6u6DL69rd5ZfduLgUcxu75p94Yfr911y66mptZOn8+ofx8+"       \
    "0EeVnluLbDV/"                                                             \
    "tzuN1Xl58OUKXDuuYnJ8fbtc98neixrzBjxPt7bbhODMfx60MMtdMZtjPl+w/"            \
    "vWW3DrrezYvJlrABdj9vxTbz0/s37TJqpCIWbMnSsubu2pJU1+RJStIPQCa7og5+aKC/"     \
    "rcc8XMidXVItDV0GD93PGxqLIy4R7feKPoz73kEjEm3I/"                            \
    "Am1tXORNLY54+PycHdu2Cm2+m9ZVX+AbwD89fLk31hgutaUBFBb+/"                    \
    "5RZOveKK5K4lbZmJIJUb+O3y/"                                                \
    "NDevfDqq+KZ4pUrxcB+N+OD+5qKi2HCBHFjOu008fBHOt1BRtmBKsvLxNIqHQ6LG/"        \
    "Ett6D+6lfcAdyMzw8quFFvAgwwqa6OP915J+"                                     \
    "NPP91bUCsTS7dlfmr7djFK66WXxPKDD0RQqS8qEBBdQBMniijyggWiq6uszP9juQHVrixTMO" \
    "uDVvfeC3ffzZ9aW7kWn2bY8KreBhjgtOnT+"                                      \
    "e0991A9bZq4qzkFtfxcei0zpv3UgQMC4HXrRLvqnXdEt05vudu5uSIQNXy4AHXGDLGsqxNt/" \
    "EwoVatrV+YnvIGAcJ3/"                                                      \
    "+Ee45RZe3LePK4Dtnr+oT+oLAANcccop3HfXXRTV1bmLTKezTLWOVV6mtH+/mDBg0ybhZm/"  \
    "ZAh99JEY/NTaKrqB0Xe9AQMyPrXVL1dQIYOvqxDO4dXXC6vbEyDA3UMryMg2zMWj173/"     \
    "Dd77Dhi1b+"                                                               \
    "Ayw1tOX9Fl9BeAA8J8XXMD377iD6IABok3cE5bYbZ6VBe4JkPVqahIu9r59oi29f7/"       \
    "4NDSI18K0topPV5ew2ooiIsChkBjmV1CQGHtdXi5eRVJZKSxtRYXos83L69nvJANPn/"      \
    "YL7HThzcsTffvf/CYfr1/PZ4EXXH/"                                            \
    "JDKmvAAyQA9x6xRV849ZbCZWWCusiA9hPS5uK29yTbrVXqWoCXO2hAu03CwZF8KWvyC2QTuW" \
    "pRpStlrJ0bq4YTvrNb7Lvtde4Fp+mhU1XfWkatjjw+rvvUtLezqyTThI/"                \
    "mt3TS16WqdZxyrNL94Y0UEMhAav2CYX6zoT0TpDq0364027rGverh/f99+H//"            \
    "B8aX36Zr0L6b1TwS30JYIAuReG1d96hsrOTqbNnC7fPaaCHHZBut0klz206KyEv4OrTfllit" \
    "3WN8G7cCDfdRPOzz/"                                                        \
    "IdemiMs1v1NYAB2mMxVq5bx4BYjMmzZtlDbCc3YFvVdXMMN3WzIFtbxVRcaFldP91rI7ybN8" \
    "NNN9H6xBP8F3C/"                                                           \
    "5ZfsJfVFgAHauiGu7upi0owZIoBgB7FfbV03+3ablulEANoOVOO639ZXlpcqvN2Wt/Vf/"    \
    "+Im4D5A4pj3rvoqwABHurp4+"                                                 \
    "e23qezoYMqMGaLvUdYf6md72CkvlbSb9eNBfoJrlU41Su0leKW1eW+"                   \
    "6ieZuy3sf4GGekZ5TXwYY4Egsxstvv01pSwvTpk0j0K+f/"                           \
    "BG9dNrDsjIrpWJ5j2eYU4XWrszPiLRdmTHiDALetWvhpps49MwzfBvhNvc5y6uprwMM0BaP8" \
    "9KaNeQePMjMKVMIlpUlIE7VqvZU8CpVePsi1LLIriw/"                              \
    "U5bYb4j16UBAwPvqq3DTTexdsYKvAr+hj+tYABigQ1V5ad064vX1zB4/"                 \
    "nsiAAelD7LZ+umnZupc8N2V+ywpWqzI3eX640Pp0OmDr08GgGB755JNwyy1se/"           \
    "NNbiDDk9H5pWMFYIAY8MoHH9C4eTOza2vJHzYsMWjBSV5daatt/"                      \
    "eoq8htefbkdfG7rOZV7Bda4nil32ovrrKqijzwQELNr3n476957j2uBJ8zfrm/"           \
    "qWAIYRFvkjW3b+GjDBqZXVlKqvTNH/"                                           \
    "8f1dp+vH5bYLt9rHSc5gWxXJx3La1eWKXdab32jUfHwzAMPwF138ez27VwLvG7+"          \
    "Rn1XxxrAmt7ds4e1b73F+EiEQRMnChfIOGpLtkwlL5V0KutO+"                        \
    "U5lfsmr5ZXl93ZEWpZnjDTX18Ndd6Hefz8PHjrEDYCLV671LR2rAAN83NjIS6tWUd3Swrjx4" \
    "8VAfKdJ5JxAleVlIvLsdrtU9uGHerINbFfmhzstC1a9/"                             \
    "TbcdhstDz7InV1d3ATsN3+"                                                   \
    "Dvq9jGWCAhvZ2nlu1isCuXUwZNoycwYPN7WK33Ua9GXn2Kxrtpr4bl9lN/"               \
    "XTbwcb1TFlf7ROJiN/nn/+E225j2/PP8w1EH2+PzCCZCR3rAIN42/kL77/"               \
    "Pzg0bmFRcTOmoUSI4YTfoQ5/uqchzX2z/aurJdrBxPRPutDE/N1c8Q/2LX8Cdd/"          \
    "L8Bx9wHfAv6Rc6hnQ8AAwiuLWuvp43Vq2i5sgRRowebe9S+"                          \
    "xF5TtfdTRXeTLrSqUamnYA15mXahdbSwWDiPb133MGRX/"                            \
    "6SBxob+RrwnvybHFs6XgDWtLO5mWdefRVl+"                                      \
    "3YmVlURHTZMFFh1NfVG8MpLnpuyTCnT7WDjerrW17jMyRHPlD/8MHz/"                  \
    "+2x++mm+HY9zNxl+X1FP6ngDGKBVVXn+gw/"                                      \
    "Y9NZbjIzHqR4xQrzO08v806mkncq85Nnlp1tXk5e2cDruszEvUxAbre6mTfDjH6Pcey9/"    \
    "27KFG4B/00fHNKeq4xFgEC71ewcO8PwrrxDdsYOxVVVEamrEn2s1QVym3eZMwNtT/"        \
    "cB29fy0vHZlbqxvNCqmY3rsMfjBD9jxyCN8v62NW4Bt8rM/"                          \
    "tnW8AqzpYCzGU+++y9bVq6ltb2fA8OGibWw3gqsvD97o7X5gu/"                       \
    "Le6ErSluGwiDJv2AD33ovyk5/"                                                \
    "wjw8+"                                                                    \
    "4CvAXziGo8xOOt4BBuEybThwgOdWrEDdsoUxBQXkDh1qnrIHTtzBG1by2g6W5WeiHWx0lxsa" \
    "xHuofvhDNj7+"                                                             \
    "OLcdOcJ3gc3WZ3986EQAWNNBVeXpTZvYsHIllXv2MLyyksDAgYlXu4D8ojyWBm/"          \
    "0ZD+wVVkmLLFxGQgIcLu64Lnn4O67afrlL/n99u18DXic49jq6tWL9/"                  \
    "ZeVTnw6cmTuWr5csaef7545248ntztZAxy9aXBG5mwyr3ZF2xctwIXRHRZUcRoqoceQnn8cZ" \
    "7btYufIoJUGXiha9/ViQqwpjHA5xYt4rLlyxl4xhkwYEBiTmXwNvBDtu62jl2+23I/"       \
    "1VP9wcZ1u75ebSTVBx/A3/8Ojz3G2vfe41eIdm6D/"                                \
    "RkfnzrRAdZ0Un4+n1+0iPPPP5+"                                               \
    "yJUvEZOexmL1Ftktb5WXbwOY8J4C19wpv3izeivDYY2xavZo/AA9ynEaX3SoLcEJhYGG/"    \
    "fnxu0SKWnXsuJYsWiTcWWLnW+rTfgzcy0fY1yq+2cLrtYBnAgYCwuKoqwH3iCfjnP9m6ahV/" \
    "BX7PcTKSKl1lATYrBwHyZ+bP58xlyyhfvFi8MwiEe62/"                             \
    "yDT1VDeS2zqpqifbwbK0NjtGZ6eYWO7JJ+"                                       \
    "Hpp9n8xhs8AvwR2OB8hieOsgBbKweYk5vLp2bP5qxTTmHQ0qUwalQi+il7/"              \
    "tgubZdnl++23E9lqi/"                                                       \
    "YKq29QaKpSQSnnn4aXniBDRs28BDwKFmLK1UWYGcFgKnABZMn84kFC5iwZAlMmyZeDqYoyVY" \
    "ZvI/i6qvtX03ptoOt0sGgcJMVBXbuhNdeg+efp23lSl7fvp2/Iqa2+SiNUz/"             \
    "u1Qcuj2NKw4DTqqs5f/"                                                      \
    "Zs5ixYQPGcOTBiRGLi+VjMHcyydaf8dOtaKRNjou2g1eahamyEd9+FV16Bl19m11tv8f/"    \
    "bO7eeJoIwgJ7pncuiLVehNmkJKSA1pjESEp584MFf65uJPpAYgwkpwRosAi4BlCq06yW9L+"  \
    "3Wh+laQKsEKVSZk3xpZ9M0282ezs43M/"                                         \
    "meFQo8Bp4DHVrmvLNQAp8PDXggBI9iMRZmZ5mcm8MVj0Mw2CwFc1pmOP9KrdNc1Tzwrz73p7" \
    "YtrcMhS6DqOiwvw9IS+ZUVEjs7PAGeAq/"                                        \
    "5zzYbtBsl8N8TAuY1jYWZGebjcSL37yPu3pUyd3fLG7paPVt9p4vKSl8E5x3nCtGsjmhZcly" \
    "7vS3HtokEpdVVUqkUi/U6z4Bl/"                                               \
    "qPtfZeNEvjicAATwJym8XByktlYjMi9e7hiMbnSy+9v3tT2Zoo/"                      \
    "9dCtjrXiopdSniWzLMTJR+"                                                   \
    "NKRRYg13VIJiGZJLe2xtvNTV7UaiwCCeDj2c9C0QolcHsQQBiIu1zMRyI8iEaJTk8TmJqSY+" \
    "axMbhxoznXeVzqll/"                                                        \
    "aAVloW1a7BnG9LoU1DNjbkwXBUims9XX2t7Z48+"                                  \
    "EDL4El5PTPweX9guuBEvhy6AeiQFzTiIdCxCIRwhMT9I+"                            \
    "PQzgMo6Myq93TI6WGpth22MfajRDNsGUVoilrLgeZjMwc6zroOjVd50DX2Xj/"            \
    "nlfVKitAEtgGiu0/4+uLEvhqGATGgTtuNzMjI0wGg4THxrgVCtE3Oip76KEhKXVfnxxLe70/" \
    "C3U8oHX293Q23I7jbfuPolaDchkKBfj2DbJZODiQwqbTVHd3+"                        \
    "ZJOk97f593hIWvAG2ADuawx184LpziJErgz6AKGgdtIsSN+P+HBQYKBACMDA/"            \
    "QPDNDr99MVCMixtKbJ6O6W4fPJntvplGFLflx0y2om00xTViUoFqWouZyMz5/BMKh//"      \
    "Uo+kyFnGGSyWT4ZBrv5PNvwI/aBLNds90+noQTuXATQC/"                            \
    "iRPfZw43XI6WSwq4uApnHT50Pz+"                                              \
    "ej1eOjxePB6vbjdblxOJw4hEA4HwrKoWxZWrYZlmhyZJkeVCmXTJF8uky8WyeXzfCmVMIDDU" \
    "2Egs8SlK7kKit+iBP63cQFe5LJPT6Ntv3c0QgB1oNYIEzhqhNmICmr+"                  \
    "VaFQKBQKhUKhUCgUCkUH8x3RL3Hh35fgRwAAAABJRU5ErkJggg==\" >"
int main (int argc, char ** argv)
{
    filename = "./Fl_Html_View.html"; /// test_filename ();

    // char filename[1024];
    //  char mark[1024];
    // const char * directory = "c:/bbb";

    // const char * url = "#asd";
    // get_local_filename_from_uri(filename,  mark, directory, url);

    fl_register_images ();

    window = new Fl_Double_Window (400, 660);
    b = new Fl_Html_View (10, 10, 380, 480);
    // b->format_width(-1);
    // b->zoomed_font_measurement(true);
    // b->box(FL_FLAT_BOX);
    b->color (FL_WHITE);
    b->format_width (-1);

    Fl_Group * g = new Fl_Group (0, 500, 380, 280);

    /*Fl_Box * bb = new Fl_Box (10, 500, 380, 110);
    bb->box (FL_UP_BOX);
    const char * html_label =
        "<center><font color=red>This is a <b><font size=+2>HTML "
        "label!</font></b> </font> </center><br/>"
        //"<font size=-1>"
        "<p>A data-uri image " IMAGE ", an inlined "
        "<table style=display:inline-table;vertical-align:middle rules=all  "
        "border>"
        "  <tr><td colspan=2 bgcolor=white>table"
        "  <tr><td bgcolor=yellow><font size=-3> aa aaa</font><td "
        "bgcolor=green> <font size=-3>bbb bb</font>"
        "</table>,"
        "<font color=blue> blue color</font>, <b>bold</b>,  "
        "some<sub>subscript</sub>, some<sup>superscript</sup><br>"
        "<center><font size=-1>Label has <code>align (FL_ALIGN_CLIP | "
        "FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP)</code></font></center>"

        ;
    Fl_Html_Label_Cache label_cache (html_label);
    fl_html_label (bb, &label_cache);
    bb->align (FL_ALIGN_CLIP | FL_ALIGN_TOP | FL_ALIGN_INSIDE |
    FL_ALIGN_WRAP);*/
    // bb->label(html_label);
    // bb->labeltype(FL_HTML_LABEL_SRC);

    Fl_Check_Button * b1 =
        new Fl_Check_Button (20, 620, 130, 30, "format_width(-1)");
    b1->value (1);
    b1->box (FL_FLAT_BOX);
    b1->callback (&format_callback);
    Fl_Button * but = new Fl_Button (170, 620, 80, 30, "Reload");
    but->callback (&callback);

    Fl_Button * b2 = new Fl_Button (270, 620, 100, 30, "Open File...");
    b2->callback (&filechooser_callback);

    Fl_Box * box = new Fl_Box (370, 620, 5, 20);
    g->resizable (box);

    g->end ();

    callback (0, 0);

    window->resizable (b);

    window->end ();
    window->show (argc, argv);

    // fl_open_display();

    return Fl::run ();
}
