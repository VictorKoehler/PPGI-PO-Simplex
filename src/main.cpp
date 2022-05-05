#include <iostream>
#include "Xplex.hpp"

void example_1() {
    Xplex::Model m;
    auto x1 = m.newVariable("x1");
    auto x2 = m.newVariable("x2");
    auto x3 = m.newVariable("x3");
    auto c1 = Xplex::Constraint("c1", 14);
    auto c2 = Xplex::Constraint("c2", 28);
    auto c3 = Xplex::Constraint("c3", 30);
    std::cout << x1.getIndex() << " " << x2.getIndex() << " " << x3.getIndex() << " "
              << c1.getIndex() << " " << c2.getIndex() << " " << c3.getIndex() << "\n";
    c1.setVariableCoefficient(x1, 2);
    c1.setVariableCoefficient(x2, 1);
    c1.setVariableCoefficient(x3, 1);
    c2.setVariableCoefficient(x1, 4);
    c2.setVariableCoefficient(x2, 2);
    c2.setVariableCoefficient(x3, 3);
    c3.setVariableCoefficient(x1, 2);
    c3.setVariableCoefficient(x2, 5);
    c3.setVariableCoefficient(x3, 5);
    m.add(c1);
    m.add(c2);
    m.add(c3);
    m.objectiveFunction().setVariableCoefficient(x1, 1);
    m.objectiveFunction().setVariableCoefficient(x2, 2);
    m.objectiveFunction().setVariableCoefficient(x3, -1);
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(true);
    xplex.solve();
}

int main() {
    // const Xplex::Model m;
    // Xplex::Xplex xplex(&m);
    // xplex.setVerbose(true);
    // xplex.solve();
    example_1();
} 
