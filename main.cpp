/*
main.c

dynrec experiment for performance improvment measure

*/


#include <iostream>
#include <stack>
#include <chrono>

#if __EMSCRIPTEN__
#include <emscripten.h>

#define __DYNREC__ 0

#endif


#define LOOPS 3

class VirtualMachine {
public:


#if __EMSCRIPTEN__
  void dynrec_startMethod(const char* name, const int argc)
  {
    std::cout<<__func__<<std::endl;

    EM_ASM_INT({
        var name = Pointer_stringify($0|0);

        Module.dynrec = Module.dynrec || {};
        Module.dynrec.methods = Module.dynrec.methods || {};
        Module.dynrec.currentMethod = name;
        code += 'var locals = [];\n';
        code += 'var r0 = 0;\n';
        Module.dynrec.code = code; 

        return 0;
    }
    ,name);
  };
  void dynrec_endMethod()
  {
    std::cout<<__func__<<std::endl;
    EM_ASM({
      
      var name = Module.dynrec.currentMethod;
      
      var code = Module.dynrec.code;
      code += 'return r0;\n'
      
      console.log(code);
      console.error('DYNREC method: ', name,' code-->',code);

      //Module.dynrec.methods[name] = new Function("a1", "a2", Module.dynrec.code);
    });
  };
  int dynrec_callmethod_i(const char* name, const int a, const int b)
  {
    return EM_ASM_INT({
      var name = Pointer_stringify($0|0);
      var a = $1;
      var b = $2;
      return Module.dynrec.methods[name](a,b);
    }
    ,name
    ,a
    ,b);
  };

  void emit_push_i()
  {
    EM_ASM({
      Module.dynrec.code += 'locals.push(;\n';
    });
  }
  void emit_add_i()
  {
    EM_ASM({
      Module.dynrec.code += '{\n';
      Module.dynrec.code += 'var l0 = locals[l-1] + locals[l]\n;'
      Module.dynrec.code += 'locals.pop();\nlocals.pop();\n';
      Module.dynrec.code += 'locals.push(l0);\n';
      Module.dynrec.code += '}\n';
    });
  }
  void emit_pop_i()
  {
    EM_ASM({
      Module.dynrec.code += 'locals[l] = arguments.pop();\n'
    });
  }
  void emit_return_i()
  {
    EM_ASM({
      Module.dynrec.code += 'var r0 = locals[l]; l--;return r0;';
    });
  }
#endif

  std::stack<int> s;
  int locals[100];

  void setlocal_i(const int reg)
  {
    locals[reg] = s.top();
    s.pop();
  }

  void getlocal_i(const int reg)
  {
    s.push(locals[reg]);
  }

  int return_local_i(const int reg)
  {
    return locals[reg];
  }

  void push_i(int v) {
    s.push(v);
#if __EMSCRIPTEN__ && __DYNREC__
    emit_push_i();
#endif
  };

  int pop_i()
  {
    int r = s.top();
    s.pop();
#if __EMSCRIPTEN__ && __DYNREC__
    emit_pop_i();
#endif
    return r;
  }

  void add_i()
  {
    int a = pop_i();
    int b = pop_i();

#if __EMSCRIPTEN__ && __DYNREC__
    emit_add_i();
#endif

    s.push(a+b);
  }

};

#if __EMSCRIPTEN__

void dummy(){};

#endif


int main(int argc, char* argv[])
{
  int a = 1;
  int b = 1;
  int r = 0;

  VirtualMachine vm;
 
#if __EMSCRIPTEN__ && __DYNREC__
  vm.dynrec_startMethod("sum");
#endif
 
  using namespace std::chrono;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  // Let's say we have a function that has a long series of number
  // manipulating ocodes. Every time we call this method we traverse these
  // opcodes and do work for each one.

  vm.push_i(a);
  vm.push_i(b);

  for(int i=0; i<LOOPS; ++i)
  {

    // method had two arguments on stack: a0, a1
    // method returns one value r0;

    vm.setlocal_i(2);
    vm.setlocal_i(1);
    vm.getlocal_i(1);
    vm.getlocal_i(2);
    vm.add_i();
    vm.setlocal_i(1);
    vm.getlocal_i(1);
    vm.getlocal_i(2);
  }
  a = vm.return_local_i(1);
  high_resolution_clock::time_point t2 = high_resolution_clock::now();

#if __EMSCRIPTEN__ && __DYNREC__
  vm.dynrec_endMethod();
#endif

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

#if 0 //__DYNREC__

  r = vm.dynrec_callmethod_i("sum", a, b);

#else // __DYNREC__

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

#endif // __DYNREC__

  high_resolution_clock::time_point t4 = high_resolution_clock::now();

  std::cout<<"Virtual machine implementaition result: "<<r<<std::endl;
  duration<double> time_span_2 = duration_cast<duration<double>>(t4-t3);
  std::cout<<"operation took : "<<time_span_2.count()<<" seconds."<<std::endl;


#endif // __EMSCRIPTEN__


#if __EMSCRIPTEN__
  emscripten_set_main_loop(dummy, 0, 1);
#endif

  return 0;

}

