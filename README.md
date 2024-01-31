# FastMatMul
## An application of parallel computing and MPI to matrix multiplication.

Summary: `matrix_mul.c` demonstrates a faster way to multiply two square matrices of size $N$ using $P$ cores (where $P=1,2,4,16$). 

### Description
We developed a C program using MPI for parallel implementation of matrix-matrix multiplication. Among the many different available methods, we chose to implement a method with row-partition of matrix $A$ and column-partition of matrix $B$. 

We will be starting with some simple definitions, as well as a description of matrix multiplication. For simplicity, we will assume that we have been given two square matrices with the same size, namely $A,B\in\mathbb{R}^{N\times N}$. Then, the $\left(i,j\right)$-th element of the product $C=AB$ is:

$$c_{i,j}=\sum_{k=0}^{N}a_{ik}b_{kj}$$

This implies that the overall calculation of all $N\times N$ elements of the product matrix has time complexity $O\left(N^3\right)$. This is not a problem for small matrices, but one the size of the matrix increases (e.g., $N>1000$) the calculation becomes very expensive. As a result, we want to parallelize the computation.

First, we split the data across different cores; A is row-partitioned, while B is column-partitioned. In the following example, we use three cores:
![](pics/doc1pic.png)

Then, we multiply the sub-matrices and add the products in a “dot-product” fashion (even though they are matrices not vectors). The results are the diagonal submatrices.
![](pics/doc2pic.png)

We now permute the submatrix-rows across cores like in the following example:
![](pics/doc3pic.png)

And repeat the same process to get the following elements:
![](pics/doc4pic.png)

We conduct this P times, where P is the number of cores used.

