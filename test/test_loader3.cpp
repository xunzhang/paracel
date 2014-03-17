#include <string>
#include <functional>
#include "paracel_types.hpp"
#include "utils.hpp"
#include "load/loader.hpp"
#include "load/parser.hpp"

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>;

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  int rk = comm.get_rank();

  {
    parser_type f_parser;
    paracel::loader<paracel::str_type> ld("./data/", comm, f_parser, "linesplit");
    //paracel::loader<paracel::str_type> ld("/mfs/user/wuhong/pre_release/debug/paracel/data/", comm, f_parser, "linesplit");
	auto linelst = ld.load();
  }
  return 0;
}
