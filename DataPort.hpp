#ifndef DATAPORT_HPP
#define DATAPORT_HPP

#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>

class Node; // Forward declaration

typedef struct {
    int* data;
    int shape[4];
    int size;
} NumPyArray;

typedef struct {
    int8_t* data;
    int shape[4];
    int size;
} QNumPyArray;

NumPyArray* createNumPyArray(int dim0, int dim1, int dim2, int dim3, void *data=nullptr)
{
    NumPyArray* array = (NumPyArray*)malloc(sizeof(NumPyArray));
    
    array->shape[0] = dim0;
    array->shape[1] = dim1;
    array->shape[2] = dim2;
    array->shape[3] = dim3;
    array->size = dim0 * dim1 * dim2 * dim3;
    array->data = (int *)data;
    if (data == nullptr) {
        if (array->data != nullptr)
            free(array->data);
        array->data = (int*)malloc(array->size * sizeof(int));
    }

    return array;
}

QNumPyArray* createQNumPyArray(int dim0, int dim1, int dim2, int dim3, void *data=nullptr)
{
    QNumPyArray* array = (QNumPyArray*)malloc(sizeof(QNumPyArray));
    
    array->shape[0] = dim0;
    array->shape[1] = dim1;
    array->shape[2] = dim2;
    array->shape[3] = dim3;
    array->size = dim0 * dim1 * dim2 * dim3;
    array->data = (int8_t *)data;
    if (data == nullptr) {
        if (array->data != nullptr)
            free(array->data);
        array->data = (int8_t*)malloc(array->size * sizeof(int8_t));
    }

    return array;
}

void freeNumPyArray(NumPyArray* array)
{
    free(array->data);
    free(array);
}

// Function to reshape a 2D array to 4D
void reshape(int* srcArray, int srcRows, int srcCols, NumPyArray* destArray)
{
    int index = 0;
    
    for (int i = 0; i < destArray->shape[0]; i++)
    {
        for (int j = 0; j < destArray->shape[1]; j++)
        {
            for (int k = 0; k < destArray->shape[2]; k++)
            {
                for (int l = 0; l < destArray->shape[3]; l++)
                {
                    destArray->data[index++] = srcArray[k * srcCols + l];
                }
            }
        }
    }
}

// Print the array
void printNumPyArray(NumPyArray *A)
{
    int index = 0;

    for (int i = 0; i < A->shape[0]; i++)
    {
        printf("[\n");
        for (int j = 0; j < A->shape[1]; j++)
        {
            printf("  [\n");
            for (int k = 0; k < A->shape[2]; k++)
            {
                printf("    [");
                for (int l = 0; l < A->shape[3]; l++)
                {
                    printf("%3d ", A->data[index++]);
                }
                printf("]\n");
            }
            printf("  ]\n");
        }
        printf("]\n");
    }
}

void printQNumPyArray(QNumPyArray *A)
{
    int index = 0;

    for (int i = 0; i < A->shape[0]; i++)
    {
        printf("[\n");
        for (int j = 0; j < A->shape[1]; j++)
        {
            printf("  [\n");
            for (int k = 0; k < A->shape[2]; k++)
            {
                printf("    [");
                for (int l = 0; l < A->shape[3]; l++)
                {
                    printf("%3d ", A->data[index++]);
                }
                printf("]\n");
            }
            printf("  ]\n");
        }
        printf("]\n");
    }
}

// template <class T>
class DataPort {
public:
    std::vector<int> shape;
    NumPyArray *data;
    std::vector<int> triggers;
    bool triggered;
    Node* node;

    DataPort(std::vector<int> shape = {}, NumPyArray *data = nullptr, bool triggered = false) : shape(shape), triggered(triggered), node(nullptr) {
        if (data == nullptr)
            data = createNumPyArray(1, 1, 1, 1);
        this->data = data;
    }

    std::string toString() const
    {
#if 1
        if (this == nullptr) return "<None>";
    
        std::stringstream ss;
        ss << "shape: [";
        for (const auto& s : shape) ss << s << " ";
        ss << "]\ndata:" << data << "\n";
        printNumPyArray(data);
    //    ss << "]\ndata: [0][0]" << data[0][0] << "\n";
    //    ss << "data: [1][0]" << data[1][0] << "\n";
    //    for (const auto& d : data) ss << d << " ";
    //    ss << "]\ntriggered: " << (triggered ? "True" : "False") << "\nnode: " << (node ? "Exists" : "None");
    
        return ss.str();
#endif
    }

    void trigger();
};

#endif // DATAPORT_HPP
