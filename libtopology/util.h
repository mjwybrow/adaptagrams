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
/** 
 * Sum over the results of calling operation for each member in the
 * iterator.  Handier than std::accumulate because we can use with
 * mem_fun to pass in a getter method.
 */
template <class InputIterator, class T, class Operation >
T sum_over(InputIterator beg, InputIterator end, T init, Operation op)
{
    for ( ; beg != end; ++beg)
    init = init + op(*beg);
    return init;
}
} // namespace topology
#endif // TOPOLOGY_UTIL_H
