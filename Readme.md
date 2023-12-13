# KodJIT

JIT/AOT compiler

Build:
```bash
mkdir build && cd build
cmake -G Ninja ..
ninja
```
Testing:
```
cd build
ctest
```

Folder `utils` contains script `pic.sh` used to convert .dot dumps produced by tests to .png pics.


<img src=resources/factorial.svg title="Dump example">