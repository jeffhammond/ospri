
/*************************************************
 * *           Likely and Unlikely Ifs              *
 * *************************************************/

#define likely_if(x) if(__builtin_expect(x,1))
#define unlikely_if(x) if(__builtin_expect(x,0))

/* Dave Goodell's version in ARMCI-MPI
#if defined(__GNUC__) && (__GNUC__ >= 3)
#  define unlikely(x_) __builtin_expect(!!(x_),0)
#  define likely(x_)   __builtin_expect(!!(x_),1)
#else
#  define unlikely(x_) (x_)
#  define likely(x_)   (x_)
#endif
*/
