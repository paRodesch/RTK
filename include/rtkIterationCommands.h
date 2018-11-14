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

#ifndef rtkIterationCommands_h
#define rtkIterationCommands_h

namespace rtk
{

/** \class IterationCommand
 * \brief Abstract superclass to all iteration callbacks.
 * Derived classes must implement the Run() method.
 *
 * \author Aurélien Coussat
 *
 * \ingroup RTK
 *
 */
template<typename TCaller>
class IterationCommand: public itk::Command
{
  protected:

    /** How many times this command has been executed. */
    unsigned int m_iterationCount = 0;

    /** Callback function to be redefined by derived classes. */
    virtual void Run(const TCaller * caller) = 0;

  public:
  
    /** Standard class typedefs. */
    typedef IterationCommand Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    
    void Execute(itk::Object * caller, const itk::EventObject & event) override
    {
      Execute( (const itk::Object *) caller, event );
    }
    
    void Execute(const itk::Object * caller, const itk::EventObject & event) override
    {
      if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
        return;
      }
      ++m_iterationCount;
      const auto * cCaller = dynamic_cast< const TCaller * >( caller );
      if ( cCaller )
      {
        Run( cCaller );
      } // TODO fail when cast fails
    }
    
};

/** \class VerboseIterationCommand
 * \brief Outputs to standard output when an iteration completes.
 *
 * \author Aurélien Coussat
 *
 * \ingroup RTK
 *
 */
template<typename TCaller>
class VerboseIterationCommand: public IterationCommand<TCaller>
{
  public:
    /** Standard class typedefs. */
    typedef VerboseIterationCommand Self;
    typedef IterationCommand<TCaller> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    itkNewMacro(Self);
  protected:
    void Run(const TCaller * caller) override
    {
      std::cout << "Iteration " << this->m_iterationCount << " completed." << std::endl; // TODO allow string personnalization
    }
};

/** \class OutputIterationCommand
 * \brief Output intermediate iterations in a file.
 * This class is usefull to check convergence of an iterative method
 * and to avoid starting over similar computations when testing
 * hyperparameters of an iterative algorithm.
 *
 * \author Aurélien Coussat
 *
 * \ingroup RTK
 *
 */
template<typename TCaller, typename TOutputImage>
class OutputIterationCommand: public IterationCommand<TCaller>
{
  public:
    /** Standard class typedefs. */
    typedef OutputIterationCommand Self;
    typedef IterationCommand<TCaller> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    itkNewMacro(Self);
  protected:
    void Run(const TCaller * caller) override
    {
      typedef itk::ImageFileWriter< TOutputImage > WriterType;
      typename WriterType::Pointer writer = WriterType::New();
      writer->SetFileName( "iter" + std::to_string( this->m_iterationCount ) + ".mha" ); // TODO allow to set the name
      writer->SetInput( caller->GetIntermediateReconstruction() );
      TRY_AND_EXIT_ON_ITK_EXCEPTION( writer->Update() )
    }
};

} // end namespace rtk

#endif
