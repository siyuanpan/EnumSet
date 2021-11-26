# EnumSet

`EnumSet` provides strongly-typed set-like functionality for strongly typed enums in C++17. This implementation was heavily influenced by Daniil Goncharov's amazing [magic_enum](https://github.com/Neargye/magic_enum) project.

## Usage

`EnumSet` is a header-only library, sinply copy `include/EnumSet.h` to your project to use it.

Usage example:

```c++
#include "EnumSet.h"

enum class Fruit {
    Apple,
    Banana,
    Orange
};

int main() {
    EnumSet<Fruit> fruits(Fruit::Apple);

    if (fruits & Fruit::Apple)
        cout << "Apple";
}
```