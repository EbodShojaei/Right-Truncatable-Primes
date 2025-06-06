# Clone using the web URL
git clone https://github.com/kimwalisch/primesieve.git
cd primesieve
# You need to have installed a C++ compiler which supports C++11 (or later) and CMake ≥ 3.4
cmake .
cmake --build . --parallel
sudo cmake --install .
# Compile the test program
cd ..
g++ test.cpp -o test.out -L./primesieve -lprimesieve -Wl,-rpath,@loader_path/primesieve
# Run the test program
./test.out
rm test.out
# Compile the main program
g++ count_primes.cpp -o count_primes.out -L./primesieve -lprimesieve -Wl,-rpath,@loader_path/primesieve
