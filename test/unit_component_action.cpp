#include <Sosage/Component/Action.h>
#include <Sosage/Utils/error.h>

using namespace Sosage;

int main (int, char**)
{
  Component::Action action ("action");
  check (action.size() == 0, "Action should be empty");

  std::vector<std::string> a = { "a", "b", "c" };
  action.add (a);
  check (action.size() == 1, "Action size should be 1");
  check (action.begin()->get(0) == "a", "Action.begin() should point to a");
  check (action[0].get(0) == "a", "Action[0][0] should be a");
  check (action[0].get(1) == "b", "Action[0][1] should be b");
  check (action[0].get(2) == "c", "Action[0][2] should be c");

  std::vector<std::string> b = { "0", "2.5" };
  action.add (b);
  check (action.size() == 2, "Action size should be 1");
  check (action.begin()->get(0) == "a", "Action.begin() should point to a");
  check (action[0].get(0) == "a", "Action[0][0] should be a");
  check (action[0].get(1) == "b", "Action[0][1] should be b");
  check (action[0].get(2) == "c", "Action[0][2] should be c");
  check (action[1].get(0) == "0", "Action[1][0] should be 0");
  check (action[1].get_int(0) == 0, "Action[1][0] int should be 0");
  check (action[1].get(1) == "2.5", "Action[1][1] should be 2.5");

  return EXIT_SUCCESS;
}
