Windows: 
cmake -DCMAKE_BUILD_TYPE=Debug -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang ..
make && cp compile_commands.json ../compile_commands.json
