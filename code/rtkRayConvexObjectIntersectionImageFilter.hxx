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

#ifndef rtkRayConvexObjectIntersectionImageFilter_hxx
#define rtkRayConvexObjectIntersectionImageFilter_hxx

#include "rtkRayConvexObjectIntersectionImageFilter.h"
#include "rtkProjectionsRegionConstIteratorRayBased.h"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIteratorWithIndex.h>

namespace rtk
{

template <class TInputImage, class TOutputImage>
RayConvexObjectIntersectionImageFilter<TInputImage,TOutputImage>
::RayConvexObjectIntersectionImageFilter():
  m_Geometry(ITK_NULLPTR)
{
}

template <class TInputImage, class TOutputImage>
void
RayConvexObjectIntersectionImageFilter<TInputImage,TOutputImage>
::BeforeThreadedGenerateData()
{
  if( this->m_ConvexObject.IsNull() )
    itkExceptionMacro(<<"ConvexObject has not been set.")
}

template <class TInputImage, class TOutputImage>
void
RayConvexObjectIntersectionImageFilter<TInputImage,TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       ThreadIdType threadId)
{
  // Local convex object since convex objects are not thread safe
  ConvexObjectPointer co = dynamic_cast<ConvexObject *>(m_ConvexObject->Clone().GetPointer());

  // Iterators on input and output
  typedef ProjectionsRegionConstIteratorRayBased<TInputImage> InputRegionIterator;
  InputRegionIterator *itIn;
  itIn = InputRegionIterator::New(this->GetInput(),
                                  outputRegionForThread,
                                  m_Geometry);
  typedef itk::ImageRegionIteratorWithIndex<TOutputImage> OutputRegionIterator;
  OutputRegionIterator itOut(this->GetOutput(), outputRegionForThread);

  // Go over each projection
  for(unsigned int pix=0; pix<outputRegionForThread.GetNumberOfPixels(); pix++, itIn->Next(), ++itOut)
    {
    // Compute ray intersection length
    ConvexObject::ScalarType near, far;
    if( co->IsIntersectedByRay(itIn->GetSourcePosition(), itIn->GetDirection(), near, far) )
      itOut.Set( itIn->Get() + co->GetDensity() * ( far - near ) );
    else
      itOut.Set( itIn->Get() );
    }

  delete itIn;
}

} // end namespace rtk

#endif
