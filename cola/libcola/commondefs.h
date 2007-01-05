#ifndef _COMMONDEFS_H
#define _COMMONDEFS_H

namespace cola {
/**
 * resolve overlaps:
 *   None: not at all
 *   Horizontal: only horizontally
 *   Both: resolve in either Horizontal or Vertical direction
 *         depending on which leads to less displacement
 */
enum NonOverlapConstraints { None, Horizontal, Both };
enum Dim { HORIZONTAL, VERTICAL };
}

#endif
