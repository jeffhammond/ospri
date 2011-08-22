
/*************************************************
 * *           Likely and Unlikely Ifs              *
 * *************************************************/

#define likely_if(x) if(__builtin_expect(x,1))
#define unlikely_if(x) if(__builtin_expect(x,0))

