#pragma once

#include <algorithm>  // std::find_if
#include <functional> // std::function
#include <memory>     // std::unique_ptr
#include <vector>     // std::vector

namespace del
{
  // Primary Template Declaration
  template <typename T>
  class event;

  // Template Specialization using a variadic function signature
  template <typename R, typename... Args>
  class event<R(Args...)>
  {
    // A pure abstract struct that is meant to handle type-erased callable types
    struct base_callable
    {
      virtual ~base_callable() = default;
      virtual bool compare(base_callable *bc) = 0;
      virtual R invoke(Args &&...args) = 0;
    };

    // Handles general callable types EXCEPT for Function Objects (Functors)
    template <typename T>
    struct generic_callable_handle : base_callable
    {
      template <typename C>
      generic_callable_handle(C &&c) : m_Func{std::forward<C>(c)}
      {
      }

      // Two callables are the same if they have the same addr
      bool compare(base_callable *bc) override
      {
        auto cc = dynamic_cast<generic_callable_handle<T> *>(bc);
        return cc && cc->m_Func == m_Func;
      }

      R invoke(Args &&...args) override
      {
        return m_Func(std::forward<Args>(args)...);
      }

    private:
      T m_Func; ///< Holds the function pointer
    };

    // A callable handle that is meant primarily for member functions
    template <typename T>
    struct object_callable_handle : base_callable
    {
      object_callable_handle(T *obj, R (T::*memFunc)(Args...)) :
        m_Obj{obj},
        m_MemFunc{memFunc}
      {
      }

      // Two callables are the same if they have the same mem func and obj addr
      bool compare(base_callable *bc) override
      {
        auto oc = dynamic_cast<object_callable_handle<T> *>(bc);
        return oc && (oc->m_Obj == m_Obj) && (oc->m_MemFunc == m_MemFunc);
      }

      R invoke(Args &&...args) override
      {
        return (m_Obj->*m_MemFunc)(std::forward<Args>(args)...);
      }

    private:
      T *m_Obj; ///< Address to the object of type T
      R (T::*m_MemFunc)
      (Args...); ///< Address of the object's mem func
    };

  public:
    // Attaches a callback, which will be invoked when this delegate is called.
    template <typename T>
    event &attach(T &&callback)
    {
      m_Callables.emplace_back(new generic_callable_handle<T>(std::forward<T>(callback)));
      return *this;
    }

    // Attaches a member function, which will be invoked when this delegate is called.
    template <typename T>
    event &attach(T &obj, R (T::*memFn)(Args...))
    {
      m_Callables.emplace_back(new object_callable_handle<T>(&obj, memFn));
      return *this;
    }

    // Detaches the first instance of a certain callback.
    template <typename T>
    event &detach(T &&callback)
    {
      generic_callable_handle<T> temp(std::forward<T>(callback));
      detach_impl(temp);
      return *this;
    }

    // Detaches the first instance of a certain member function.
    template <typename T>
    event &detach(T &obj, R (T::*memFn)(Args...))
    {
      object_callable_handle<T> temp(&obj, memFn);
      detach_impl(temp);
      return *this;
    }

    // Clears all instance of a certain callback.
    template <typename T>
    event &detach_all(T &&callback)
    {
      generic_callable_handle<T> temp(std::forward<T>(callback));
      detach_all_impl(temp);
      return *this;
    }

    // Clears all instance of a certain member function.
    template <typename T>
    event &detach_all(T &obj, R (T::*memFn)(Args...))
    {
      object_callable_handle<T> temp(&obj, memFn);
      detach_all_impl(temp);
      return *this;
    }

    // Removes all callbacks attached to this delegate.
    event &clear()
    {
      m_Callables.clear();
      return *this;
    }

    // Calls this delegate, effectively invoking every callback that is attached to it.
    void operator()(Args &&...args)
    {
      for (auto &callable : m_Callables)
        callable->invoke(std::forward<Args>(args)...);
    }

  private:
    void detach_impl(base_callable &temp)
    {
      // Predicate to determine if it is the same callable type
      auto compPred = [&](std::unique_ptr<base_callable> &callable)
      { return temp.compare(callable.get()); };

      // Finds the first instance of that callable type, and removes it
      m_Callables.erase(std::find_if(std::begin(m_Callables), std::end(m_Callables), compPred));
    }

    void detach_all_impl(base_callable &temp)
    {
      // Predicate to determine if it is the same callable type
      auto compPred = [&](std::unique_ptr<base_callable> &callable)
      { return temp.compare(callable.get()); };

      // Removes all instance of that callable type
      m_Callables.erase(std::remove_if(std::begin(m_Callables), std::end(m_Callables), compPred), std::end(m_Callables));
    }

    std::vector<std::unique_ptr<base_callable>> m_Callables; ///< Generic container to hold all callables
  };
}
