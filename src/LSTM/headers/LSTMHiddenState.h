#ifndef LSTM_HIDDEN_STATE_H
#define LSTM_HIDDEN_STATE_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

// LA libraries:
#include <eigen3/Eigen/Dense>
#include <cmath>

using namespace pdb;

class LSTMHiddenState
    : public JoinComp<FFMatrixBlock, FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  LSTMHiddenState(){};

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2) override {
    return makeLambdaFromMethod(in1, getBlockColIndex) ==
           makeLambdaFromMethod(in2, getBlockRowIndex);
  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            // get the sizes
            uint32_t I = in1->getRowNums();
            uint32_t J = in2->getColNums();
            uint32_t K = in1->getColNums();

            // make an output
            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(
                    in1->getBlockRowIndex(), in2->getBlockColIndex(), I, J,
                    in1->getTotalRowNums(), in2->getTotalColNums());

            // get the ptrs
            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();
            double *in1Data = in1->getValue().rawData->c_ptr();
            double *in2Data = in2->getValue().rawData->c_ptr();

            // tanh 2nd input
            // Does this modify the data if used elsewhere?
            for (int i = 0; i < in2->getColNums() * in2->getRowNums(); i++) {
                in2Data[i] = (exp(in2Data[i]) - exp(-1 * in2Data[i])) / (exp(in2Data[i]) + exp(-1 * in2Data[i]));
            }

            // do the multiply
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                currentMatrix1(in1->getValue().rawData->c_ptr(),
                               in1->getRowNums(), in1->getColNums());
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                currentMatrix2(in2->getValue().rawData->c_ptr(),
                               in2->getRowNums(), in2->getColNums());

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                productMatrix(resultFFMatrixBlock->getValue().rawData->c_ptr(),
                              I, J);

            productMatrix = currentMatrix1 * currentMatrix2;

            return resultFFMatrixBlock;
          } else {
            std::cerr << "Wrong librayCode!" << std::endl;
            exit(1);
          }
        });
  }
};

#endif