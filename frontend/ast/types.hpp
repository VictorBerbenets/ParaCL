#pragma once

namespace frontend {

namespace ast {

class object_type {
 public:
   using size_type = std::size_t;

  ~virtual object_type() {}
};

class integer_type: public object_type {
  integer_type(int val)
      : value_ {val} {}
 private:
  int value_;
};

class integer_variable: public object_type {

};

// if we know the size before running
class array: public object_type {
 public:
  array(size_object_type sz, int init_val = {0})
      : stor_ (sz, init_val) {}
 protected:
  std::vector<int> stor_;
};

class init_array: public array {

};

// if we know the size before running

class dynamic_array: public array {

};

//class multi_array: public 
  
} // <--- namespace ast

} // <--- namespace frontend

