#pragma once

#include <unistd.h>
#include <linux/limits.h>
#include <libgen.h>
#include <cstring>
#include <stdexcept>

// Changes the working directory to the folder containing the running executable,
// so that relative asset paths (shaders, textures) resolve correctly regardless
// of where the binary was launched from.
inline void chdirToExe()
{
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        throw std::runtime_error("chdirToExe: readlink(/proc/self/exe) failed");
    }
    path[len] = '\0';
    if (chdir(dirname(path)) != 0) {
        throw std::runtime_error("chdirToExe: chdir failed");
    }
}
