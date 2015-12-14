#include <fstream>

#include "Globs/Simulation.hpp"

void print_help_and_exit(int argc, char **argv);
void print_version_and_exit(int argc, char **argv);

int main(int argc, char **argv)
{
  if(argc != 2)
    print_help_and_exit(argc, argv);

  string param(argv[1]);
  if(!param.compare("-v") || !param.compare("--version"))
    print_version_and_exit(argc, argv);

  Simulation *s = Simulation::getInstance();
  
  s->run(argv[1]);

  s->output();

  s->kill();

  return 0;
}

void print_help_and_exit(int argc, char **argv)
{
  cerr << "Usage : " << argv[0] << " <param_file>" << endl;
  exit(EXIT_FAILURE);
}

void print_version_and_exit(int argc, char **argv)
{
  cout << OF_MAJOR_REVISION << "." << OF_COMPILE_DATE << endl;

  exit(EXIT_SUCCESS);
}
