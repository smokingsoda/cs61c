from distutils.core import (setup, Extension)
import os
import sysconfig

def main():
    os.environ["CC"] = "gcc-14"  # 设置环境变量 CC 为 gcc-14
    CFLAGS = ['-g', '-Wall', '-std=c99', '-fopenmp' , '-pthread', '-O3']
    LDFLAGS = ['-fopenmp']
    # Use the setup function we imported and set up the modules.
    # You may find this reference helpful: https://docs.python.org/3.6/extending/building.html
    # TODO: YOUR CODE HERE
    setup(name="numc",
          version="0.0.1",
          description="numc matrix operations",
          ext_modules=[
            Extension("numc",
                      sources=["numc.c", "matrix.c"],
                      extra_compile_args=CFLAGS,
                      extra_link_args=LDFLAGS,
                      language='c')
          ])

if __name__ == "__main__":
    main()
