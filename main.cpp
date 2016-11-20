/*
main.c

dynrec experiment for performance improvment measure

*/


#include <iostream>
#include <stack>
#include <chrono>

#if __EMSCRIPTEN__
#include <emscripten.h>
#endif


#define LOOPS 100000

class VirtualMachine {
public:
  //enum OPCODES {
  //  ADD_I = 0,
  // }

  //int ic;
  std::stack<int> s;

  void push_i(int v) {
    s.push(v);
  };

  int pop_i()
  {
    int r = s.top();
    s.pop();
    return r;
  }

  void add_i()
  {
    int a = pop_i();
    int b = pop_i();
    s.push(a+b);
  }
};


int main(int argc, char* argv[])
{
  int a = 1;
  int b = 1;
  int r = 0;

  VirtualMachine vm;
  
  using namespace std::chrono;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  // Let's say we have a function that has a long series of number
  // manipulating ocodes. Every time we call this method we traverse these
  // opcodes and do work for each one.
  for(int i=0; i<LOOPS; ++i)
  {
    vm.push_i(a);
    vm.push_i(b);
    vm.add_i();
    a = vm.pop_i();
  }
  high_resolution_clock::time_point t2 = high_resolution_clock::now();

  std::cout<<"Virtual machine implementaition result: "<<a<<std::endl;
  duration<double> time_span = duration_cast<duration<double>>(t2-t1);
  std::cout<<"operation took : "<<time_span.count()<<" seconds."<<std::endl;

#if __EMSCRIPTEN__
  // assume instead that the first time we traverse these opcodes in the given
  // method, we form a list of corresponding native expressions for each.
  // this code is meant to emulate what such a 'native' representation of the previous
  // method would look like.
  a = 1;
  b = 1;
  r = 0;
  int loops =  LOOPS;
  high_resolution_clock::time_point t3 = high_resolution_clock::now();
  
  r = EM_ASM_INT({
    
    var a = $0;
    var b = $1;
    var loops = $2;
    var r = 0;

    for(var i=0;i < loops ;i++)
    {
      var x = a; // pop a
      var y = b; // pop b
      var t = x + y; // add
      a = t; // push result
      r = t; // pop result
    }
    return r;
  }
  ,a
  ,b
  ,loops);

  high_resolution_clock::time_point t4 = high_resolution_clock::now();

  std::cout<<"Virtual machine implementaition result: "<<r<<std::endl;
  duration<double> time_span_2 = duration_cast<duration<double>>(t4-t3);
  std::cout<<"operation took : "<<time_span_2.count()<<" seconds."<<std::endl;


#endif // __EMSCRIPTEN__


  return 0;

}

