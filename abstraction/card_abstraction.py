import numpy as np
import matplotlib.pyplot as plt
import faiss

ffe = open("/mnt/nfs/work1/ameli/yanqichen/flop_equities.bin", "rb")
flop_equity = np.fromfile(ffe, dtype=np.float64).astype(np.float32)
centroids = np.zeros(5000, dtype=np.float32)
faiss.kmeans_clustering(1, flop_equity.shape[0], 5000, faiss.swig_ptr(flop_equity), faiss.swig_ptr(centroids))
centroids = np.sort(centroids)
boudary = np.zeros((4999, 1), dtype=np.float32)
for i in range(1, 5000):
    boudary[i - 1] = (centroids[i - 1] + centroids[i]) / 2
boudary.tofile("/mnt/nfs/work1/ameli/yanqichen/flop_buckets.bin")

ffe = open("/mnt/nfs/work1/ameli/yanqichen/turn_equities.bin", "rb")
flop_equity = np.fromfile(ffe, dtype=np.float64).astype(np.float32)
centroids = np.zeros(5000, dtype=np.float32)
faiss.kmeans_clustering(1, flop_equity.shape[0], 5000, faiss.swig_ptr(flop_equity), faiss.swig_ptr(centroids))
centroids = np.sort(centroids)
boudary = np.zeros((4999, 1), dtype=np.float32)
for i in range(1, 5000):
    boudary[i - 1] = (centroids[i - 1] + centroids[i]) / 2
boudary.tofile("/mnt/nfs/work1/ameli/yanqichen/turn_buckets.bin")

ffe = open("/mnt/nfs/work1/ameli/yanqichen/river_equities.bin", "rb")
flop_equity = np.fromfile(ffe, dtype=np.float64).astype(np.float32)
flop_equity = np.random.permutation(flop_equity)
centroids = np.zeros(5000, dtype=np.float32)
faiss.Clustering.max_points_per_centroid = 1024
sample_num = 1024 * 5000
faiss.kmeans_clustering(1, sample_num, 5000, faiss.swig_ptr(flop_equity[:sample_num]), faiss.swig_ptr(centroids))
centroids = np.sort(centroids)
boudary = np.zeros((4999, 1), dtype=np.float32)
for i in range(1, 5000):
    boudary[i - 1] = (centroids[i - 1] + centroids[i]) / 2
boudary.tofile("/mnt/nfs/work1/ameli/yanqichen/river_buckets.bin")