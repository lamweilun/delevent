## delevent
### What is delevent?
- It is a header-only basic event dispatcher.
  - Compatible with C++11 and higher.
  - Short for "delegating events"
- Main inspiration came from C# delegate.

### What can it handle?
- Global functions
- Static member functions (Essentially "named" global functions)
- Member functions
- Lambdas (as long as it has no captures)

### What does not work?
- Unable to handle Function Objects (functors).
  - They work as "closures", meaning it has state, whereas function pointers have no state.
- Retrieving return results. For now, it is only meant to work with "void".return types.

### Future Plans
- Look into allowing users to retrieve return results.
- Could look into speeding up this tool, nothing has been benchmarked yet.
- MIGHT do some research into supporting function objects (functors).

### Basic usage of a del::event
```C++
#include "delevent.h" // del::event
#include <iostream>   // std::cout

void PrintNum(int num)
{
  std::cout << num << std::endl;
}

int main()
{
  // A delegate that is able to attach functions that takes in an integer and returns void
  del::event<void(int)> delEvent;

  // Attach a function simply by calling "attach"
  delEvent.attach(PrintNum);

  // Calling this delegate would invoke every function that is attached to it
  // This would then print "123"
  delEvent(123);

  // To detach a function, simply call "detach"
  delEvent.detach(PrintNum);

  // This would now effectively do nothing
  delEvent(456);
}
```

### Attaching lambdas
- Working with lambdas is the same as working with any general function
- NOTE: Captures are not supported, as having any would mean that it is a "closure" (having instance variables).
```C++
#include "delevent.h" // del::event
#include <iostream>   // std::cout

int main()
{
  // Create a basic lambda
  auto lambda = [](){std::cout << "This is a lambda" << std::endl;};

  // Attach said lambda
  del::event<void()> delEvent;

  delEvent.attach(lambda);
  delEvent();         // Prints "This is a lambda"
}
```

### Attaching member functions
```C++
#include "delevent.h" // del::event
#include <iostream>   // std::cout

// Basic singleton that holds del::event
struct PlayerEvents
{
  static PlayerEvents& Get()
  {
    static PlayerEvents pe;
    return pe;
  }

  del::event<void(int)> PlayerAttackEvent;
};

// Enemy that subscribes to PlayerEvents
class Enemy
{
  int m_HP;

public:

  // PlayerEvent attaches to this object's "TakeDamage" function
  Enemy(int hp) : m_HP{ hp }
  {
    PlayerEvents::Get().PlayerAttackEvent.attach(*this, &Enemy::TakeDamage);
  }

  // Destructor of this Enemy would then make PlayerEvents detach itself from it
  ~Enemy()
  {
    PlayerEvents::Get().PlayerAttackEvent.detach(*this, &Enemy::TakeDamage);
  }

private:
  void TakeDamage(int damage)
  {
    std::cout << "Taking " << damage << " damage..." << std::endl;
    m_HP -= damage;
    std::cout << "Remaining HP: " << m_HP << std::endl;
  }
};

int main()
{
  // Creating an enemy with 10 HP
  Enemy enemy(10);

  /* This would then print
  Taking 1 damage...
  Remaining HP: 9
  */
  PlayerEvents::Get().PlayerAttackEvent(1);
}
```

### Method Chaining
- Most methods of del::event return a reference to itself so it supports method chaining.
```C++
#include "delevent.h" // del::event
#include <iostream>   // std::cout

void Sum(float a, float b)
{
  std::cout << "Sum of " << a << " and " << b << " is " << a + b << std::endl;
}

void Difference(float a, float b)
{
  std::cout << "Difference of " << a << " and " << b << " is " << a - b << std::endl;
}

void Product(float a, float b)
{
  std::cout << "Product of " << a << " and " << b << " is " << a * b << std::endl;
}

int main()
{
  // A delegate that is able to attach functions that takes in 2 floats and returns void
  del::event<void(float, float)> mathEvents;

  // Attaches all of the above functions via method-chaining
  mathEvents.attach(Sum).attach(Difference).attach(Product);

  /* Calling this would now print:
  Sum of 6 and 4 is 10
  Difference of 6 and 4 is 2
  Product of 6 and 4 is 24
  */
  mathEvents(6, 4);

  // Similarly you can method-chain "detach"
  mathEvents.detach(Sum).detach(Difference).detach(Product);

  // Mix and match all you want
  mathEvents.attach(Sum).attach(Product).detach(Product);

  /* Calling this would now print:
  Sum of 6 and 4 is 10
  */
  mathEvents(6, 4);
}
```
