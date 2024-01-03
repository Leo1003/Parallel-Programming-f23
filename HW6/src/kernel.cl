__kernel void convolution(
    int filterWidth,
    __constant float *filter,
    int imageHeight,
    int imageWidth,
    __global float *inputImage,
    __global float *outputImage)
{
    int pad_margin = filterWidth / 2;
    int padded_width = imageWidth + (pad_margin * 2);
    int padded_height = imageHeight + (pad_margin * 2);

    int x = get_global_id(0);
    int y = get_global_id(1);
    float sum = 0;

    // Apply the filter to the neighborhood
    for (int fy = 0; fy <= filterWidth; fy++) {
        for (int fx = 0; fx <= filterWidth; fx++) {
            int dx = x + fx;
            int dy = y + fy;

            sum += inputImage[dy * padded_width + dx] * filter[fy * filterWidth + fx];
        }
    }

    outputImage[y * imageWidth + x] = sum;
}
