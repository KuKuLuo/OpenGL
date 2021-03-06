#ifndef _logger_included_124900971247253743634896758972312399234333009756612344
#define _logger_included_124900971247253743634896758972312399234333009756612344

#include <cstdlib>

//=======================================================================================================================================================================================================================
// simple multithreaded lock-free file logger
//=======================================================================================================================================================================================================================
extern void impl_debug_msg(const char* format, ...);
extern void impl_put_msg(const char* msg);

static constexpr const char * const short_path(const char * const str, const char * const last_slash)
{
    return *str == '\0' ? last_slash :
         ((*str == '/') || (*str == '\\')) ? short_path(str + 1, str + 1) :
                                             short_path(str + 1, last_slash);
}

static constexpr const char * const short_path(const char * const str) 
{ 
    return short_path(str, str);
}

//=======================================================================================================================================================================================================================
// the interface :)
//=======================================================================================================================================================================================================================

#define put_msg(str) impl_put_msg(str)

#if defined(_MSC_VER)
    #define debug_msg(str,...)  impl_debug_msg("%s : %s : %d : " str "\n",       short_path(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define exit_msg(str,...)  {impl_debug_msg("%s : %s : %d ERROR : " str "\n", short_path(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__); exit(1);}
#else
    #define debug_msg(str,...)  impl_debug_msg("%s : %s : %d : " str "\n",       short_path(__FILE__), __func__, __LINE__, ##__VA_ARGS__)
    #define exit_msg(str,...)  {impl_debug_msg("%s : %s : %d ERROR : " str "\n", short_path(__FILE__), __func__, __LINE__, ##__VA_ARGS__); exit(1);}
#endif

#endif // _logger_included_124900971247253743634896758972312399234333009756612344

