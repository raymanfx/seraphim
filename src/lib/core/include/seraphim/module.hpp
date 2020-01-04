#ifndef SPH_MODULE_HPP
#define SPH_MODULE_HPP

extern "C" {

typedef void *(*sph_factory_create_t)(int, ...);
typedef void (*sph_factory_destroy_t)(int, void *);

struct seraphim_module_factory {
    sph_factory_create_t create;
    sph_factory_destroy_t destroy;
};

struct seraphim_module_version {
    int major;
    int minor;
    int patch;
};

struct seraphim_module {
    const char *id;
    struct seraphim_module_version version;
    struct seraphim_module_factory factory;
};

typedef seraphim_module (*sph_module_t)(void);

} // extern "C"

#endif // SPH_MODULE_HPP
