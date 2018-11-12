/*=========================================================================
 *
 *  Copyright RTK Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef rtkConjugateGradientConeBeamReconstructionFilter_h
#define rtkConjugateGradientConeBeamReconstructionFilter_h

#include <itkMultiplyImageFilter.h>
#include <itkDivideOrZeroOutImageFilter.h>
#include <itkProcessObject.h>
#include <itkObject.h>
#include <itkCommand.h>

#include "rtkConjugateGradientImageFilter.h"
#include "rtkReconstructionConjugateGradientOperator.h"
#include "rtkIterativeConeBeamReconstructionFilter.h"
#include "rtkThreeDCircularProjectionGeometry.h"
#include "rtkDisplacedDetectorImageFilter.h"
#include "rtkConstantImageSource.h"
#include "rtkLaplacianImageFilter.h"
#include "rtkBlockDiagonalMatrixVectorMultiplyImageFilter.h"

#ifdef RTK_USE_CUDA
  #include "rtkCudaConjugateGradientImageFilter.h"
  #include "rtkCudaDisplacedDetectorImageFilter.h"
  #include "rtkCudaConstantVolumeSource.h"
#endif

namespace rtk
{
  /** \class ConjugateGradientConeBeamReconstructionFilter
   * \brief Implements ConjugateGradient
   *
   * This filter implements the ConjugateGradient method.
   * ConjugateGradient attempts to find the f that minimizes
   * (1/2).|| sqrt(D) (Rf -p) ||_2^2 + (1/2).gamma.|| grad f ||_2^2
   * with R the forward projection operator,
   * p the measured projections, and D the displaced detector weighting operator.
   *
   * With gamma=0, this it is similar to the ART and SART methods. The difference lies
   * in the algorithm employed to minimize this cost function. ART uses the
   * Kaczmarz method (projects and back projects one ray at a time),
   * SART the block-Kaczmarz method (projects and back projects one projection
   * at a time), and ConjugateGradient a conjugate gradient method
   * (projects and back projects all projections together).
   *
   * With gamma > 0, a regularization is applied.
   *
   * \dot
   * digraph ConjugateGradientConeBeamReconstructionFilter {
   *
   * Input0 [ label="Input 0 (Volume)"];
   * Input0 [shape=Mdiamond];
   * Input1 [label="Input 1 (Projections)"];
   * Input1 [shape=Mdiamond];
   * Input2 [label="Input 2 (Weights)"];
   * Input2 [shape=Mdiamond];
   * Input3 [label="Input Support mask"];
   * Input3 [shape=Mdiamond];
   * Output [label="Output (Volume)"];
   * Output [shape=Mdiamond];
   *
   * node [shape=box];
   * MultiplyProjections [label="itk::MultiplyImageFilter" URL="\ref itk::MultiplyImageFilter"];
   * MultiplyVolumes [label="itk::MultiplyImageFilter" URL="\ref itk::MultiplyImageFilter"];
   * MultiplyOutput [label="itk::MultiplyImageFilter" URL="\ref itk::MultiplyImageFilter"];
   * BackProjection [ label="rtk::BackProjectionImageFilter" URL="\ref rtk::BackProjectionImageFilter"];
   * Displaced [ label="rtk::DisplacedDetectorImageFilter" URL="\ref rtk::DisplacedDetectorImageFilter"];
   * ConjugateGradient[ label="rtk::ConjugateGradientImageFilter" URL="\ref rtk::ConjugateGradientImageFilter"];
   * VolumeSource [ label="rtk::ConstantImageSource (Volume)" URL="\ref rtk::ConstantImageSource"];

   * Input0 -> ConjugateGradient;
   * Input1 -> MultiplyProjections;
   * Input2 -> Displaced;
   * Displaced -> MultiplyProjections;
   * MultiplyProjections -> BackProjection;
   * VolumeSource -> BackProjection;
   * Input3 -> MultiplyVolumes;
   * Input3 -> MultiplyOutput;
   * BackProjection -> MultiplyVolumes;
   * MultiplyVolumes -> ConjugateGradient;
   * ConjugateGradient -> MultiplyOutput;
   * MultiplyOutput -> Output;
   * }
   * \enddot
   *
   * \test rtkconjugategradientreconstructiontest.cxx
   *
   * \author Cyril Mory
   *
   * \ingroup RTK ReconstructionAlgorithm
   */

template< typename TOutputImage,
          typename TSingleComponentImage = TOutputImage,
          typename TWeightsImage = TOutputImage>
class ConjugateGradientConeBeamReconstructionFilter : public rtk::IterativeConeBeamReconstructionFilter<TOutputImage>
{
public:
    /** Standard class typedefs. */
    typedef ConjugateGradientConeBeamReconstructionFilter                      Self;
    typedef IterativeConeBeamReconstructionFilter<TOutputImage, TOutputImage>  Superclass;
    typedef itk::SmartPointer< Self >                                          Pointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Run-time type information (and related methods). */
    itkTypeMacro(ConjugateGradientConeBeamReconstructionFilter, itk::ImageToImageFilter)

    /** Setters for the inputs */
    void SetInputVolume(const TOutputImage* vol);
    void SetInputProjectionStack(const TOutputImage* projs);
    void SetInputWeights(const TWeightsImage* weights);

    typedef rtk::ForwardProjectionImageFilter< TOutputImage, TOutputImage >                 ForwardProjectionFilterType;
    typedef typename ForwardProjectionFilterType::Pointer                                   ForwardProjectionFilterPointer;
    typedef rtk::BackProjectionImageFilter< TOutputImage, TOutputImage >                    BackProjectionFilterType;
    typedef rtk::ConjugateGradientImageFilter<TOutputImage>                                 ConjugateGradientFilterType;
    typedef itk::MultiplyImageFilter<TOutputImage, TSingleComponentImage, TOutputImage>     MultiplyFilterType;
    typedef rtk::ReconstructionConjugateGradientOperator<TOutputImage,
                                                         TSingleComponentImage,
                                                         TWeightsImage>                     CGOperatorFilterType;
    typedef rtk::DisplacedDetectorImageFilter<TOutputImage>                                 DisplacedDetectorFilterType;
    typedef rtk::ConstantImageSource<TOutputImage>                                          ConstantImageSourceType;
    typedef itk::DivideOrZeroOutImageFilter<TOutputImage>                                   DivideFilterType;
    typedef itk::StatisticsImageFilter<TOutputImage>                                        StatisticsImageFilterType;
    typedef typename TOutputImage::Pointer                                                  OutputImagePointer;
    typedef itk::StatisticsImageFilter<TSingleComponentImage>                               StatisticsFilterType;

    typedef typename Superclass::ForwardProjectionType ForwardProjectionType;
    typedef typename Superclass::BackProjectionType BackProjectionType;

    // If TOutputImage is an itk::Image of floats or double, so are the weights, and a simple Multiply filter is required
    // If TOutputImage is an itk::Image of itk::Vector<float (or double)>, a BlockDiagonalMatrixVectorMultiply filter
    // is needed. Thus the meta-programming construct
    typedef rtk::BlockDiagonalMatrixVectorMultiplyImageFilter<TOutputImage, TWeightsImage>  MatrixVectorMultiplyFilterType;
    typedef itk::MultiplyImageFilter<TOutputImage, TOutputImage, TOutputImage>              PlainMultiplyFilterType;
    typedef typename std::conditional<std::is_same< TSingleComponentImage, TOutputImage>::value,
                                                    PlainMultiplyFilterType,
                                                    MatrixVectorMultiplyFilterType>::type MultiplyWithWeightsFilterType;

    /** Pass the ForwardProjection filter to the conjugate gradient operator */
    void SetForwardProjectionFilter (ForwardProjectionType _arg) ITK_OVERRIDE;

    /** Pass the backprojection filter to the conjugate gradient operator and to the back projection filter generating the B of AX=B */
    void SetBackProjectionFilter (BackProjectionType _arg) ITK_OVERRIDE;

    /** Set the support mask, if any, for support constraint in reconstruction */
    void SetSupportMask(const TSingleComponentImage *SupportMask);
    typename TSingleComponentImage::ConstPointer GetSupportMask();

    /** Pass the geometry to all filters needing it */
    itkSetConstObjectMacro(Geometry, ThreeDCircularProjectionGeometry)

    itkSetMacro(NumberOfIterations, int)
    itkGetMacro(NumberOfIterations, int)

    itkSetMacro(IterationCosts, bool)
    itkGetMacro(IterationCosts, bool)

    /** Set / Get whether the displaced detector filter should be disabled */
    itkSetMacro(DisableDisplacedDetectorFilter, bool)
    itkGetMacro(DisableDisplacedDetectorFilter, bool)

    /** If Regularized, perform laplacian-based regularization during
     *  reconstruction (gamma is the strength of the regularization) */
    itkSetMacro(Tikhonov, float)
    itkGetMacro(Tikhonov, float)
    itkSetMacro(Gamma, float)
    itkGetMacro(Gamma, float)

    /** Get / Set whether conjugate gradient should be performed on GPU */
    itkGetMacro(CudaConjugateGradient, bool)
    itkSetMacro(CudaConjugateGradient, bool)

    /** Getter for ResidualCosts storing array **/
    const std::vector<double> &GetResidualCosts();

protected:
    ConjugateGradientConeBeamReconstructionFilter();
    virtual ~ConjugateGradientConeBeamReconstructionFilter() ITK_OVERRIDE {}

    /** Does the real work. */
    void GenerateData() ITK_OVERRIDE;

    /** Member pointers to the filters used internally (for convenience)*/
    typename MultiplyFilterType::Pointer                                        m_MultiplyProjectionsFilter;
    typename MultiplyFilterType::Pointer                                        m_MultiplyVolumeFilter;
    typename MultiplyFilterType::Pointer                                        m_MultiplyOutputFilter;
    typename ConjugateGradientFilterType::Pointer                               m_ConjugateGradientFilter;
    typename CGOperatorFilterType::Pointer                                      m_CGOperator;
    typename ForwardProjectionImageFilter<TOutputImage, TOutputImage>::Pointer  m_ForwardProjectionFilter;
    typename BackProjectionImageFilter<TOutputImage, TOutputImage>::Pointer     m_BackProjectionFilter;
    typename BackProjectionImageFilter<TOutputImage, TOutputImage>::Pointer     m_BackProjectionFilterForB;
    typename DisplacedDetectorFilterType::Pointer                               m_DisplacedDetectorFilter;
    typename ConstantImageSourceType::Pointer                                   m_ConstantVolumeSource;
    typename MultiplyWithWeightsFilterType::Pointer                            m_MultiplyWithWeightsFilter;

    /** The inputs of this filter have the same type (float, 3) but not the same meaning
    * It is normal that they do not occupy the same physical space. Therefore this check
    * must be removed */
#if ITK_VERSION_MAJOR<5
    void VerifyInputInformation() ITK_OVERRIDE {}
#else
    void VerifyInputInformation() const ITK_OVERRIDE {}
#endif

    /** The volume and the projections must have different requested regions
    */
    void GenerateInputRequestedRegion() ITK_OVERRIDE;
    void GenerateOutputInformation() ITK_OVERRIDE;

    /** Getters for the inputs */
    typename TOutputImage::ConstPointer   GetInputVolume();
    typename TOutputImage::ConstPointer   GetInputProjectionStack();
    typename TWeightsImage::ConstPointer  GetInputWeights();

    /** Iteration reporter */
    void ReportProgress(itk::Object *, const itk::EventObject &);

private:
    ConjugateGradientConeBeamReconstructionFilter(const Self &); //purposely not implemented
    void operator=(const Self &);  //purposely not implemented

    ThreeDCircularProjectionGeometry::ConstPointer m_Geometry;

    int                          m_NumberOfIterations;
    float                        m_Gamma;
    float                        m_Tikhonov;
    bool                         m_IterationCosts;
    bool                         m_CudaConjugateGradient;
    bool                         m_DisableDisplacedDetectorFilter;

    itk::IterationReporter iterationReporter;
};
} //namespace RTK


#ifndef ITK_MANUAL_INSTANTIATION
#include "rtkConjugateGradientConeBeamReconstructionFilter.hxx"
#endif

#endif
