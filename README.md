Paracel Overview
================

Paracel is a distributed computational framework designs for machine learning problems: Logistic Regression, SVD, Matrix Factorization(BFGS, sgd, als, cg), LDA, Lasso...

Firstly, paracel split both massive dataset and massive parameter space. Unlink Mapreduce-Like Systems, paracel offer a simple communication model, which allows you to work with a global, distributed key-value storage called parameter server.

In using paracel, you can build algorithms following the rule: 'pull parameters before learning | push local updates after learning'. It is rather a simple model(compared to MPI) and is almost painless transforming from serial to parallel.

Secondly, paracel try to solve 'the last-reducer problem' of iterative tasks. We use bounded staleness and find a sweet spot between 'improve-iter' curve and 'iter-sec' curve. A global scheduler take charge of asynchrounous working. This method is already proved to be a generalization of BSP/Pregel by CMU.

The other attractive aspect of paracel is fault tolerant while MPI have no idea with that.

Paracel can also be used for building graph algorithms and scientific computing. You can load your input in distributed file system and construct a graph, sparse/dense matrix in paracel.

Paracel is originally motivated by Jeff Dean's [talk](http://infolab.stanford.edu/infoseminar/dean.pdf) @Stanford in 2013. You can get more details in his paper: "[Large Scale Distributed Deep Networks](http://static.googleusercontent.com/media/research.google.com/en//archive/large_deep_networks_nips2012.pdf)".

A 20-minutes-tutorial is [here](http://xunzhang.github.io/paracel)

Be the first to get it, let's go!

Logo
----
Logo for paracel is really cool, you can make it with only one stroke:

``` bash
  (0.5,1) -> (0, 0.5) -> (1,0.5) -> (0.5, 1) -> (0.5, 0.25) -> (0.25, 0.25)
```

Whisper
-------
If you are using paracel, let me know.

Any bugs and related problems, just feel free to ping me: <xunzhangthu@gmail.com>.
