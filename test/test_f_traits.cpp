#include <assert.h>
#include <functional>
#include "utils.hpp"

double foo(double a, int b) {
  return a + (double)b;
}

class A {
public:  
  typedef double (*func_type)(double, int);
  static double foo(double a, int b) { return a + (double)b; }
  double too(double a, int b) { return a + (double)b; }
  double goo(double a, int b) const { return a + (double)b; }
public:
  func_type f_obj2;
};

int main(int argc, char *argv[]) 
{
  { // test for normal function
    using traits = paracel::f_traits<int(long, double*, const char &&, std::nullptr_t)>;
    assert(traits::arity == 4);
    static_assert(std::is_same<int, traits::result_type>::value, "err");
    static_assert(std::is_same<long, traits::args<0>::type>::value, "err");
    static_assert(std::is_same<double*, traits::args<1>::type>::value, "err");
    static_assert(std::is_same<const char &&, traits::args<2>::type>::value, "err");
    static_assert(std::is_same<std::nullptr_t, traits::args<3>::type>::value, "err");
  }
  { // test for func pointer
    double f(double, int);
    using traits = paracel::f_traits<decltype(f)>;
    assert(traits::arity == 2);
    static_assert(std::is_same<double, traits::result_type>::value, "err");
    static_assert(std::is_same<double, traits::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits::args<1>::type>::value, "err");
  }
  { // test for func pointer
    using traits = paracel::f_traits<decltype(foo)>;
    assert(traits::arity == 2);
    static_assert(std::is_same<double, traits::result_type>::value, "err");
    static_assert(std::is_same<double, traits::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits::args<1>::type>::value, "err");
  }
  {  // test for func pointer
    using traits = paracel::f_traits<decltype(&foo)>;
    assert(traits::arity == 2);
    static_assert(std::is_same<double, traits::result_type>::value, "err");
    static_assert(std::is_same<double, traits::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits::args<1>::type>::value, "err");
  }
  { // test for reference
    using traits = paracel::f_traits<void(&)()>;
    assert(traits::arity == 0);
    static_assert(std::is_same<void, traits::result_type>::value, "err");
  }
  { // test for rvalue reference
    using traits = paracel::f_traits<double((&&)(double, int))>;
    assert(traits::arity == 2);
    static_assert(std::is_same<double, traits::result_type>::value, "err");
    static_assert(std::is_same<double, traits::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits::args<1>::type>::value, "err");
  }
  { // test for member func
    class AA {
    public:  
      typedef double (*func_type)(double, int);
      static double foo(double a, int b) { return a + (double)b; }
      double too(double a, int b) { return a + (double)b; }
      double goo(double a, int b) const { return a + (double)b; }
      bool operator==(const AA&) const;
      virtual void v() {}
    };
    /*
    using traits_foo = paracel::f_traits<&AA::foo>;
    assert(traits_foo::arity == 2);
    static_assert(std::is_same<double, traits_foo::result_type>::value, "err");
    static_assert(std::is_same<double, traits_foo::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits_foo::args<1>::type>::value, "err");
    
    using traits_too = paracel::f_traits<&AA::too>;
    assert(traits_too::arity == 2);
    static_assert(std::is_same<double, traits_too::result_type>::value, "err");
    static_assert(std::is_same<double, traits_too::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits_too::args<1>::type>::value, "err");
    
    using traits_op = paracel::f_traits<&AA::operator==>;
    assert(traits_op::arity == 1);
    static_assert(std::is_same<bool, traits_op::result_type>::value, "err");
    static_assert(std::is_same<const AA &, traits_op::args<0>::type>::value, "err");
    
    using traits_v = paracel::f_traits<&AA::v>;
    assert(traits_v::arity == 0);
    static_assert(std::is_same<void, traits_v::result_type>::value, "err");
    */
  }
  {
    // test for member func 
    A obj_A;
    static_assert(std::is_same<double, paracel::f_traits<decltype(A::foo)>::result_type>::value, "err");
    static_assert(std::is_same<double, paracel::f_traits<decltype(A::foo)>::args<0>::type>::value, "err");
    static_assert(std::is_same<int, paracel::f_traits<decltype(A::foo)>::args<1>::type>::value, "err");

/*
    static_assert(std::is_same<double, paracel::f_traits<decltype(A::too)>::result_type>::value, "err");
    static_assert(std::is_same<double, paracel::f_traits<decltype(A::too)>::args<0>::type>::value, "err");
    static_assert(std::is_same<int, paracel::f_traits<decltype(A::too)>::args<1>::type>::value, "err");
*/

    using local_func_type = double (A::*)(double, int) const;
    static_assert(std::is_same<double, paracel::f_traits<decltype(static_cast<local_func_type>(&A::goo))>::result_type>::value, "err");
    //static_assert(std::is_same<const A*, paracel::f_traits<decltype(static_cast<local_func_type>(&A::goo))>::args<0>::type>::value, "err");
    //static_assert(std::is_same<double, paracel::f_traits<decltype(static_cast<local_func_type>(&A::goo))>::args<1>::type>::value, "err");
  
    typedef double(*func_type)(double, int);
    func_type A::*f_pt = &A::f_obj2;
    std::function<double(double, int)> f_obj1 = foo;
  }
  { // test for functor
    struct B {
      double operator()(double a, int b) {
        return a + (double)b;
      }
    };

    B functor;
    
    using traits = paracel::f_traits<decltype(functor)>;
    assert(traits::arity == 2);
    static_assert(std::is_same<double, traits::result_type>::value, "err");
    static_assert(std::is_same<double, traits::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits::args<1>::type>::value, "err");
  }
  {  // test for functor another
    using traits = paracel::f_traits<std::function<double(double, int)> >;
    assert(traits::arity == 2);
    static_assert(std::is_same<double, traits::result_type>::value, "err");
    static_assert(std::is_same<double, traits::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits::args<1>::type>::value, "err");
  }
  { // test for mem obj pt
    using traits = paracel::f_traits<decltype(A::f_obj2)>;
    assert(traits::arity == 2);
    static_assert(std::is_same<double, traits::result_type>::value, "err");
    static_assert(std::is_same<double, traits::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits::args<1>::type>::value, "err");
  }
  { // test for lambda
    auto lambda = [](double a, int b) { return a + (double)b; };
    using traits = paracel::f_traits<decltype(lambda)>;
    assert(traits::arity == 2);
    static_assert(std::is_same<double, traits::result_type>::value, "err");
    static_assert(std::is_same<double, traits::args<0>::type>::value, "err");
    static_assert(std::is_same<int, traits::args<1>::type>::value, "err");
  }
  return 0;
}
