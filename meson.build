project('gl_pointerlock', 'c')

glfw_dep = dependency('glfw3') # Window library
gl_dep = dependency('GL') # OpenGL - Hardware-accelerated 3D graphics
# png_dep = dependency('libpng') # For loading PNGs
# Forget it. stb_image to the rescue!

glad_targ = custom_target('glad',
input: 'dlglad.sh',
output: ['glad.c','glad.h', 'khrplatform.h'],
command: ['zsh', '@INPUT@'],
install: false)

glad_dep = declare_dependency(sources: [glad_targ], link_args: ['-ldl'])

cglm_inc = include_directories('cglm/include')

deplist = [gl_dep, glfw_dep, glad_dep]

executable('main', 'main.c', 'pngload.c', 'cube.c', 'shader.c', dependencies: deplist, include_directories: [cglm_inc], link_args: ['-lm'])