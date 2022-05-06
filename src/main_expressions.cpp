#include <iostream>
#include "Xplex.hpp"

void example_5_expressions() { // Exemplo Aula Anand - Análise de Sensibilidade
    Xplex::Model m;
    auto x1 = m.newVariable("x1");
    auto x2 = m.newVariable("x2");

    auto c1 =  1*x1        <= 4;
    auto c2 =         2*x2 <= 12;
    auto c3 =  3*x1 + 2*x2 <= 18;

    m.add(c1).add(c2).add(c3);
    m.objective() = Xplex::Maximize(3*x1 + 5*x2);
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(DEFAULT_VERBOSITY >= 1);
    xplex.solve();
    std::cout << xplex.getObjValue() << ": " << xplex.getValue(x1) << " " << xplex.getValue(x2) << "\n";

    std::cout << "\nExpected: x1=2  x2=6  x3=2\n";
}
