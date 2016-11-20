/*
main.c

dynrec experiment for performance improvment measure

*/


#include <iostream>
#include <stack>

#if __EMSCRIPTEN__
#include <emscripten.h>
#endif


static const int LOOPS = 1000;

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

  std::cout<<"Virtual machine implementaition result: "<<a<<std::endl;

#if __EMSCRIPTEN__
  // assume instead that the first time we traverse these opcodes in the given
  // method, we form a list of corresponding native expressions for each.
  // this code is meant to emulate what such a 'native' representation of the previous
  // method would look like.
  a = 1;
  b = 1;
  r = 0;
  
  r = EM_ASM_INT({
    
    var a = $0;
    var b = $1;
    var r = 0;
    
    for(var i=0;i < 1000 ;i++)
    {
      r = a + b;
      a = r;
    }
    return r;
  }
  ,a
  ,b);

  std::cout<<"Native emscripten dynrec implementation result: "<<r<<std::endl;

#endif // __EMSCRIPTEN__


  return 0;

}

