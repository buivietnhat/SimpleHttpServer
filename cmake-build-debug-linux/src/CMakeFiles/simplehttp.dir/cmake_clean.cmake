file(REMOVE_RECURSE
  "libsimplehttp.a"
  "libsimplehttp.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/simplehttp.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
