# Introduction

This project implements different parallel linked list algorithms.

- VectorBasedOptimisticDoublyLinkedList.cpp includes the implementation of a vector based optimistic doubly linked list.
  - The data of the doubly linked list is stored in several vectors (in the structure of array (SoA) manner), one for key, one for the next index and one for the previous index.
  - An integer named Head is used to idicate the head index.
  - There is no sentinel node in the algorithm. Therefore, special cares are needed when dealing with the first and the last element.