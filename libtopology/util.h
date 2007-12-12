#ifndef TOPOLOGY_UTIL_H
#define TOPOLOGY_UTIL_H
namespace topology {
/**
 * templated delete functor for use in for_each loop over vector
 */
struct delete_object
{
  template <typename T>
  void operator()(T *ptr){ delete ptr;}
};
} // namespace topology
#endif // TOPOLOGY_UTIL_H
