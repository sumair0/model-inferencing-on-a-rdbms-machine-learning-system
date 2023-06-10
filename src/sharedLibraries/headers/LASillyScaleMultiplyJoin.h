#ifndef SILLY_LA_SCALE_MULTIPLY_JOIN_H
#define SILLY_LA_SCALE_MULTIPLY_JOIN_H

// by Binhang, June 2017

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "JoinComp.h"
#include "MatrixBlock.h"

// LA libraries:
#include <eigen3/Eigen/Dense>

using namespace pdb;

class LASillyScaleMultiplyJoin : public JoinComp<MatrixBlock, MatrixBlock, MatrixBlock> {

public:
    ENABLE_DEEP_COPY

    LASillyScaleMultiplyJoin() {}

    Lambda<bool> getSelection(Handle<MatrixBlock> in1, Handle<MatrixBlock> in2) override {
        /*
        return makeLambda (in1, in2, [] (Handle<MatrixBlock> & in1, Handle<MatrixBlock> & in2) {
            return in1->getBlockRowIndex() == in2->getBlockRowIndex() && in1->getBlockColIndex() ==
        in2->getBlockColIndex();
        });
        */
        return makeLambdaFromMethod(in1, getKey) ==
            makeLambdaFromMethod(in2, getKey);  // This can be recognized by the pdb optimizer.
    }

    Lambda<Handle<MatrixBlock>> getProjection(Handle<MatrixBlock> in1,
                                              Handle<MatrixBlock> in2) override {
        return makeLambda(in1, in2, [](Handle<MatrixBlock>& in1, Handle<MatrixBlock>& in2) {
            if (MatrixBlock::librayCode == EIGEN_CODE) {
                // std::cout <<"Test Eigen" << std::endl;
                // std::cout <<"Current Matrix1 :"<< std::endl;
                // in1->print();
                // std::cout <<"Current Matrix2 :"<< std::endl;
                // in2->print();
                if (in1->getColNums() != in2->getColNums() ||
                    in1->getRowNums() != in2->getRowNums()) {
                    std::cerr << "Block dimemsions mismatch!" << std::endl;
                    exit(1);
                }
                int rowNums = in1->getRowNums();
                int colNums = in1->getColNums();
                int blockRowIndex = in1->getBlockRowIndex();
                int blockColIndex = in1->getBlockColIndex();
                int totalRows = in1->getTotalRowNums();
                int totalCols = in2->getTotalColNums();
                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                    currentMatrix1(
                        in1->getRawDataHandle()->c_ptr(), in1->getRowNums(), in1->getColNums());
                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                    currentMatrix2(
                        in2->getRawDataHandle()->c_ptr(), in2->getRowNums(), in2->getColNums());

                pdb::Handle<MatrixBlock> resultMatrixBlock = pdb::makeObject<MatrixBlock>(
                    blockRowIndex, blockColIndex, rowNums, colNums, totalRows, totalCols);
                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                    scaleProductMatrix(
                        resultMatrixBlock->getRawDataHandle()->c_ptr(), rowNums, colNums);

                scaleProductMatrix = currentMatrix1.array() * currentMatrix2.array();

                // std::cout <<"Result Matrix :"<< std::endl;
                // resultMatrixBlock->print();

                return resultMatrixBlock;
            } else {
                std::cerr << "Wrong librayCode!" << std::endl;
                exit(1);
            }
        });
    }
};


#endif