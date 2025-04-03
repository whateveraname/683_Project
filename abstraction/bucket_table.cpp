#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

int get_bucket(float equity, std::vector<float>& boundary) {
    auto it = std::lower_bound(boundary.begin(), boundary.end(), equity);
    int index = it - boundary.begin();
    return index;
}

int main() {
    std::ifstream in("/mnt/nfs/work1/ameli/yanqichen/river_equities.bin", std::ios::binary);
    std::ifstream in1("/mnt/nfs/work1/ameli/yanqichen/river_buckets.bin", std::ios::binary);
    size_t river_size = 2428287420;
    std::vector<double> equities(river_size);
    in.read((char*)equities.data(), river_size * sizeof(double));
    std::vector<float> boundary(4999);
    in1.read((char*)boundary.data(), 4999 * sizeof(float));
    std::vector<int> buckets(river_size); 
#pragma omp parallel for
    for (size_t i = 0; i < river_size; i++) {
        buckets[i] = get_bucket(equities[i], boundary);
    }
    std::ofstream out("/mnt/nfs/work1/ameli/yanqichen/river_bucket_table.bin", std::ios::binary);
    out.write((char*)buckets.data(), river_size * sizeof(int));
}