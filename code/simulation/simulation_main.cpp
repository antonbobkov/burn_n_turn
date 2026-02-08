#include "exception.h"
#include "simulation.h"


#include <iostream>

int main() {
  try {
    RunSimulation();
    return 0;
  } catch (Gui::MyException &e) {
    std::cerr << e.GetDescription(true) << std::endl;
    return 1;
  }
}
