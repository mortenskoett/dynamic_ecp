from __future__ import absolute_import
import eCP_wrapper as e
import numpy as np
from ann_benchmarks.algorithms.base import BaseANN

class eCP(BaseANN):
    def __init__(self, metric, percentage, sc, span, cpol, npol):
      # base args
        self.percentage = percentage
        self.name = "eCP"

      # benchmark args
        self.sc = sc
        self.span = span
        self.cpol = cpol
        self.npol = npol
        
        if(metric == 'angular'):
            self.metric = 1
        else:
            self.metric = 0

    def fit(self, dataset):
        #dataset contains float32, we need to convert it to float64 for the eCP algorithm
        descriptors = dataset.astype(np.float64)

        self.index = e.eCP_Index(descriptors, self.metric, self.sc, self.span, self.cpol, self.npol, self.percentage)

    def query(self, q, k):
        #query point is float32, convert it to float64
        query = q.astype(np.float64)
        
        #returns a tuple of (index, distance)
        indices = e.query(self.index, query, k, self.b)[0]
        return indices
   
    def set_query_arguments(self, b):
        self.b = b

    def __str__(self):
        return self.name+'(sc=%s, b=%s, span=%s, cpol=%s, npol=%s, percentage=%s)' % (self.sc, self.b, self.span, self.cpol, self.npol, self.percentage)
