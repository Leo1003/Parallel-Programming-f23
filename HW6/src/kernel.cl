__kernel void convolution(
    int filterWidth,
    __constant float *filter,
    int imageHeight,
    int imageWidth,
    __global float *inputImage,
    __global float *outputImage)
{
    int halffilterSize = filterWidth / 2;
    int x = get_global_id(0);
    int y = get_global_id(1);
    float sum = 0;

    // Apply the filter to the neighborhood
    // Copy from the serial version
    for (int k = -halffilterSize; k <= halffilterSize; k++) {
        for (int l = -halffilterSize; l <= halffilterSize; l++) {
            int dx = x + l;
            int dy = y + k;

            if (dy >= 0 &&
                dy < imageHeight &&
                dx >= 0 &&
                dx < imageWidth) {
                sum += inputImage[dy * imageWidth + dx] *
                       filter[(k + halffilterSize) * filterWidth + l + halffilterSize];
            }
        }
    }

    outputImage[y * imageWidth + x] = sum;
}
