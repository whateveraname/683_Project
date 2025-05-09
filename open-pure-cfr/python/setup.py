from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "strategy_parser",
        sources=[
            "mod.cpp",
            "mcts_agent.cpp",
            "../omp/EquityCalculator.cpp",
            "../omp/HandEvaluator.cpp",
            "../omp/CombinedRange.cpp",
            "../omp/CardRange.cpp",
            "../hand-isomorphism/src/hand_index.c",
        ],
        include_dirs=[
            ".",  # Current directory for mod.cpp/mcts_agent.cpp
            "../omp",  # For omp:: classes and headers
            "../hand-isomorphism/src",  # For hand_index.h
        ],
        language="c++",
        extra_compile_args=["-O3", "-std=c++17", "-march=native", "-fopenmp"],
        extra_link_args=["-fopenmp"],
    ),
]

setup(
    name="strategy_parser",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
