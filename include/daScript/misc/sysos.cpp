#include "daScript/misc/platform.h"

#include "daScript/misc/sysos.h"

#if defined(_WIN32)
    #include <windows.h>
    namespace das {
        size_t getExecutablePathName(char* pathName, size_t pathNameCapacity) {
            return GetModuleFileNameA(NULL, pathName, (DWORD)pathNameCapacity);
        }
    }
#elif defined(__linux__) /* elif of: #if defined(_WIN32) */
    #include <unistd.h>
    namespace das {
        size_t getExecutablePathName(char* pathName, size_t pathNameCapacity) {
            size_t pathNameSize = readlink("/proc/self/exe", pathName, pathNameCapacity - 1);
            pathName[pathNameSize] = '\0';
            return pathNameSize;
        }
    }
#elif defined(__APPLE__) /* elif of: #elif defined(__linux__) */
    #include <mach-o/dyld.h>
    namespace das {
        size_t getExecutablePathName(char* pathName, size_t pathNameCapacity) {
            uint32_t pathNameSize = 0;
            _NSGetExecutablePath(NULL, &pathNameSize);
            if (pathNameSize > pathNameCapacity)
                pathNameSize = pathNameCapacity;
            if (!_NSGetExecutablePath(pathName, &pathNameSize)) {
                char real[PATH_MAX];
                if (realpath(pathName, real) != NULL) {
                    pathNameSize = strlen(real);
                    strncpy(pathName, real, pathNameSize);
                }
                return pathNameSize;
            }
            return 0;
        }
    }
#else /* else of: #elif defined(__APPLE__) */
    #error provide your own implementation
#endif /* end of: #if defined(_WIN32) */

namespace das {
    string getExecutableFileName ( void ) {
        char buffer[_MAX_PATH];
        return getExecutablePathName(buffer,_MAX_PATH) ? buffer : "";
    }

    string get_prefix ( const string & req ) {
        auto np = req.find_last_of("\\/");
        if ( np != string::npos ) {
            return req.substr(0,np);
        } else {
            return req;
        }
    }

    string getDasLibPath ( void ) {
        string efp = getExecutableFileName();   // ?/bin/debug/binary.exe
        auto np = efp.find_last_of("\\/");
        if ( np != string::npos ) {
            auto ep = get_prefix(efp);  // remove file name
            ep = get_prefix(ep);        // remove debug
            ep = get_prefix(ep);        // remove bin
            return ep + "/daslib";
        } else {
            return "../../daslib";
        }
    }
}
