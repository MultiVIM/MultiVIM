find $1 \
     \( -name '*.c' -o -name '*.m' -o -name '*.mm' -o -name '*.cxx' \
    -o -name '*.h' -o -name '*.hh' -o -name '*.hxx' -o -name '*.H' \) \
    -printf "\"%p\" \n" \
    | xargs clang-format -i
