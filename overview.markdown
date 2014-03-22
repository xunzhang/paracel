# Paracel

Paracel is a distributed computational framework designs for 
machine learning problems, graph algorithms and scientific computation in C++

## Motivation

 * Parameters can not be hold in memory of a single machine
 * Asynchrounous learning to speed up
 * General framework
  * Offer simple communication model(compared to MapReduce)
  * Offer fault tolerance solution(compared to MPI)

Our basic idea is original triggered by Jeff Dean's [talk](http://infolab.stanford.edu/infoseminar/dean.pdf) @Stanford University in 2013.
You can get more details in his paper: [Large Scale Distributed Deep Networks](http://static.googleusercontent.com/media/research.google.com/en//archive/large_deep_networks_nips2012.pdf).

## Goals

 * Split both massive dataset and massive parameter space
 * Solve "the last reducer problem" of iterative tasks
 * Easy to programming, painless from serial to parallel
 * Good performance and fault tolerant

## Getting it

You can download the source code from http://code.dapps.douban.com/paracel:

```bash
$ git clone http://code.dapps.douban.com/paracel.git
```

## Installation

 * I. Prerequisite

 You must firstly install some third-party libraries below: 
  * [Boost(>=1.54)](http://www.boost.org/)
  * Zeromq(>=3.2.4) and [a c++ binding](http://zeromq.org/bindings:cpp) of it
  * [Msgpack-c-0.5.8](https://github.com/xunzhang/msgpack-c): a increment version
  * [Eigen(>=3.0)](http://eigen.tuxfamily.org/)
  * [GFlags](https://code.google.com/p/gflags/)

 And make sure you have:
  * a impl version of MPI
  * `gcc-4.`/`g++-4.7` or higher(`clang`/`clang++` is also ok)
  * `autotools`

 * II. Build

  ```bash
  $ mkdir build; cd build;
  $ cmake -DCMAKE_BUILD_TYPE=Release ..
  $ make -j 4
  $ make install
  ```

 If you use gentoo, you can just use ebuild files we provide
 
 Stuff @[Douban.Inc](douban.com) can just skip phaseI

## Get started

  A 20-minutes' tutorials is [here]()

## Logo

Draft version from xunzhang:

``` bash
(0.5,1) -> (0, 0.5) -> (1,0.5) -> (0.5, 1) -> (0.5, 0.25) -> (0.25, 0.25)
```

Plus version from Yinzi:

## Whisper

If you are using paracel, let us know.
Any bugs and related problems, feel free to ping us: <xunzhangthu@gmail.com>
