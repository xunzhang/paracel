#include <dlfcn.h>
#include <iostream>
#include <functional>
#include <string>

//using update_result = std::function<std::string(std::string, std::string)>;

int main(int argc, char *agrv[])
{
  //std::string fn = "/mfs/user/wuhong/paracel/alg/matrix_factorization/update.so";
  //std::string fcn = "mf_fac_updater";
  //update_result update_f;
  std::string fn = "/usr/lib/doubanm/cos_sim_io_graph.so";
  //for(int i = 0; i < 10; ++i) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
    //if(!handler) {
    //  std::cerr << "Cannot open library in dlopen: " << dlerror() << '\n';
    //}
    //auto local = dlsym(handler, fcn.c_str());
    //if(!local) {
    //  std::cerr << "Can not load symbol in dlopen func: " << dlerror() << '\n';
    //  dlclose(handler);
    //}
    //update_f = *(std::function<std::string(std::string, std::string)>*) local;
    char *error = dlerror();
    dlclose(handler);
  //}
  return 0;
}
