
#include "common/task.h"
#include <iostream>
#include <memory>
int main(int argc, char *argv[]) {
  common::Task t1{"/home/steinberg1/icarus/hello.sh", "another/path",
                  "third/path", common::Status::Done};
  common::Task t2;
  /* auto buf = std::make_unique<char>(t1.Size()); */
  char *buf = new char[t1.Size()];
  t1.Marshall(buf, t1.Size());

  t2.Unmarshall(buf, t1.Size());
  delete[] buf;

  std::cout << "Assertions beginning..\n";

  std::cout << "t1 == t2: " << (t1 == t2) << "\n";
  std::cout << "t1.obj_path=" + t1.GetObjPath() +
                   "\tt2.obj_path=" + t2.GetObjPath() + "\n";
  std::cout << "t1.obj_path == t2.obj_path: "
            << (t1.GetObjPath() == t2.GetObjPath()) << "\n";

  std::cout << "len(t1.obj_path) == len(t2.obj_path): "
            << (t1.GetObjPath().size() == t2.GetObjPath().size()) << "\n";
  std::cout << "t1.input_path == t2.input_path: "
            << (t1.GetInputPath() == t2.GetInputPath()) << "\n";

  std::cout << "len(t1.input_path) == len(t2.input_path): "
            << (t1.GetInputPath().size() == t2.GetInputPath().size()) << "\n";
  std::cout << "t1.result_path == t2.result_path: "
            << (t1.GetOutPath() == t2.GetOutPath()) << "\n";
  std::cout << "len(t1.result_path) == len(t2.result_path): "
            << (t1.GetOutPath().size() == t2.GetOutPath().size()) << "\n";

  std::cout << "t1.status == t2.status: " << (t1.GetStatus() == t2.GetStatus())
            << "\n\tt1.status: " << (int)t1.GetStatus()
            << " t2.status: " << (int)t2.GetStatus() << "\n";

  return 0;
}
