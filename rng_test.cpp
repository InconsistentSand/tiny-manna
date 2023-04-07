#include <random>
#include <iostream>
#include <climits>

auto getRandomSeed() 
    -> std::seed_seq
{
    // This gets a source of actual, honest-to-god randomness
    std::random_device source;

    // Here, we fill an array of random data from the source
    unsigned int random_data[10];
    for(auto& elem : random_data) {
        elem = source(); 
    }

    // this creates the random seed sequence out of the random data
    return std::seed_seq(random_data + 0, random_data + 10); 
}

int randomnumber() {
    // Making rng static ensures that it stays the same
    // Between different invocations of the function
    static auto seed = getRandomSeed(); 
    static std::default_random_engine rng(seed);

    std::uniform_real_distribution<float> dist(0, INT_MAX); 
    return (int)dist(rng);
}


int main() {

    for(int i = 0; i < 10; i++) {
        std::cout << randomnumber() << '\n'; 
    }
    std::cout << "\n";
}