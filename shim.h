#ifndef SHIM_H
#define SHIM_H

/* Ignora a macro de função fraca do Kernel do OpenBSD */
#define DEF_WEAK(x)

/* Ensina o GCC a clonar a função corretamente usando o atributo de alias nativo do Linux */
#define MAKE_CLONE(name, target) __typeof(target) name __attribute__((alias(#target)))

#endif