
# Fast lut manipulation

## Goals

Most of the time, logic optimization is done with advanced algorithmic constructs, like SOPs, BDDs, and AIGs.

This library is a try at the brute-force approach: lookup tables are relatively small for few inputs - actually smaller than any of the clever approaches - and they can be manipulated using shifts and bitwise operations.
The library provide Lut manipulation and construction primitives. Its goal is to allow brute-force based logic optimization, with only a few cycles per operation, something that would be slower with another approach.

## Implementation

The Luts are represented as vectors of 64-bit integers. Luts of 6 inputs or less have only one, but the vector's size doubles with each new input.
Depending on the memory requirements, it may be a bad idea to represent (let alone process) Luts with more than 10 inputs - a 13-input Lut will need 1kB of memory.

