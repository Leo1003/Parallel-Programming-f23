#include <CL/cl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hostFE.h"
#include "helper.h"

inline static size_t padded_size(int filterWidth, int imageHeight, int imageWidth)
{
    int pad_margin = filterWidth / 2;
    int padded_width = imageWidth + (pad_margin * 2);
    int padded_height = imageHeight + (pad_margin * 2);

    return padded_width * padded_height;
}

static float *pad_image(int filterWidth, int imageHeight, int imageWidth, const float *image)
{
    int pad_margin = filterWidth / 2;
    int padded_width = imageWidth + (pad_margin * 2);
    int padded_height = imageHeight + (pad_margin * 2);

    float *padded_image = (float *)calloc(padded_width * padded_height, sizeof(float));
    if (padded_image == NULL) {
        return NULL;
    }

    for (size_t sy = 0; sy < imageHeight; sy++) {
        int dy = sy + pad_margin;
        memcpy(&padded_image[padded_width * dy + pad_margin], &image[imageWidth * sy], sizeof(float) * imageWidth);
    }

    return padded_image;
}

void hostFE(int filterWidth, float *filter, int imageHeight, int imageWidth,
            float *inputImage, float *outputImage, cl_device_id *device,
            cl_context *context, cl_program *program)
{
    cl_int errcode;
    int filterSize = filterWidth * filterWidth;

    size_t padded_image_size = padded_size(filterWidth, imageHeight, imageWidth);
    float *padded_input = pad_image(filterWidth, imageHeight, imageWidth, inputImage);

    cl_mem buffer_input = clCreateBuffer(
        *context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(float) * padded_image_size,
        padded_input,
        &errcode);
    CHECK(errcode, "clCreateBuffer");
    cl_mem buffer_filter = clCreateBuffer(
        *context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(float) * filterSize,
        filter,
        &errcode);
    CHECK(errcode, "clCreateBuffer");
    cl_mem buffer_output = clCreateBuffer(
        *context,
        CL_MEM_WRITE_ONLY,
        sizeof(float) * imageWidth * imageHeight,
        NULL,
        &errcode);
    CHECK(errcode, "clCreateBuffer");

    cl_command_queue commandQueue = clCreateCommandQueueWithProperties(*context, *device, NULL, &errcode);
    CHECK(errcode, "clCreateCommandQueueWithProperties");
    cl_kernel kernel = clCreateKernel(*program, "convolution", &errcode);
    CHECK(errcode, "clCreateKernel");
    errcode = clSetKernelArg(kernel, 0, sizeof(int), &filterWidth);
    CHECK(errcode, "clSetKernelArg");
    errcode = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_filter);
    CHECK(errcode, "clSetKernelArg");
    errcode = clSetKernelArg(kernel, 2, sizeof(int), &imageHeight);
    CHECK(errcode, "clSetKernelArg");
    errcode = clSetKernelArg(kernel, 3, sizeof(int), &imageWidth);
    CHECK(errcode, "clSetKernelArg");
    errcode = clSetKernelArg(kernel, 4, sizeof(cl_mem), &buffer_input);
    CHECK(errcode, "clSetKernelArg");
    errcode = clSetKernelArg(kernel, 5, sizeof(cl_mem), &buffer_output);
    CHECK(errcode, "clSetKernelArg");

    size_t work_size[] = {imageWidth, imageHeight};
    errcode = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, work_size, NULL, 0, NULL, NULL);
    CHECK(errcode, "clEnqueueNDRangeKernel");
    errcode = clFinish(commandQueue);
    CHECK(errcode, "clFinish");

    errcode = clEnqueueReadBuffer(
        commandQueue,
        buffer_output,
        CL_TRUE,
        0,
        sizeof(float) * imageWidth * imageHeight,
        (void *)outputImage,
        0,
        NULL,
        NULL);
    CHECK(errcode, "clEnqueueReadBuffer");

    free(padded_input);
    errcode = clReleaseCommandQueue(commandQueue);
    CHECK(errcode, "clReleaseCommandQueue");
    errcode = clReleaseMemObject(buffer_input);
    CHECK(errcode, "clReleaseMemObject");
    errcode = clReleaseMemObject(buffer_output);
    CHECK(errcode, "clReleaseMemObject");
    errcode = clReleaseMemObject(buffer_filter);
    CHECK(errcode, "clReleaseMemObject");
    errcode = clReleaseKernel(kernel);
    CHECK(errcode, "clReleaseKernel");
}
