#include <metatron/test-header/test.hpp>
#include <metatron/test-src/test.hpp>

auto main() -> int
{
  test_header();
  test_src();
  auto p = malloc(4);
  free(p);
  return 0;
}
