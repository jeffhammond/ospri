#include <iostream>
#include <typeinfo>

int main(void)
{
    int * i;
    int j[1];

    std::cout << "sizeof " << sizeof(i)        << " " << sizeof(j)           << "\n";
    std::cout << "sizeof " << sizeof(int*)     << " " << sizeof(int)         << "\n";
    std::cout << "sizeof " << typeid(i).name() << " " << typeid(j).name() << std::endl;

    return 0;
}
